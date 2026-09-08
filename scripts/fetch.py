#!/usr/bin/env python3
"""
fetch.py

Downloads the latest kwxFetch-source.tar.xz and syncs it over the existing
wxWidgets source tree. No cmake reconfigure needed — just run ninja to rebuild.
CMake only auto-reconfigures if wxWidgets' own CMakeLists.txt files changed.

After syncing, applies the optional .private/wxWidgets overlay.

Also updates build/_deps/cmrc-src (CMakeRC2) to origin/main.

The tarballs are only downloaded when they have actually changed since the
last run. This is determined by querying the GitHub API for the release
asset's digest (a ~4KB request) instead of downloading the multi-MB tarball.
The last-known digests are cached in build/.fetch_state.json. Use --force to
bypass the check and always download.

Usage:
    python scripts/fetch.py [--force]
"""

import json
import os
import shutil
import subprocess
import sys
import tempfile
import urllib.error
import urllib.request

WX_URL = "https://github.com/KeyWorksRW/kwxFetch/releases/download/wx-dev-latest/kwxFetch-source.tar.xz"
WX_API_URL = "https://api.github.com/repos/KeyWorksRW/kwxFetch/releases/tags/wx-dev-latest"
INTERFACE_URL = "https://github.com/KeyWorksRW/kwxFetch/releases/download/latest-interface/kwxFetch-source.tar.xz"
INTERFACE_API_URL = "https://api.github.com/repos/KeyWorksRW/kwxFetch/releases/tags/latest-interface"
CMRC_REPO_URL = "https://github.com/KeyWorksRW/CMakeRC2.git"
STATE_FILE = ".fetch_state.json"
TARBALL_NAME = "kwxFetch-source.tar.xz"


def run_command(cmd: list[str], cwd: str | None = None, check: bool = True) -> subprocess.CompletedProcess:
    """Run a command and return its result. Raises on non-zero exit unless check=False."""
    proc = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if proc.returncode != 0 and check:
        stderr = (proc.stderr or "").strip()[:500]
        raise RuntimeError(f'"{" ".join(cmd)}" failed (exit {proc.returncode}):\n{stderr}')
    return proc


def is_directory(path: str) -> bool:
    return os.path.isdir(path)


def download(url: str, dest_path: str, description: str) -> None:
    """Download a URL to a file, raising on HTTP errors."""
    req = urllib.request.Request(url, headers={"User-Agent": "kwxFetch-python"})
    try:
        with urllib.request.urlopen(req) as resp, open(dest_path, "wb") as f:
            shutil.copyfileobj(resp, f)
    except urllib.error.HTTPError as e:
        raise RuntimeError(f"{description} failed (HTTP {e.code}): {e.reason}") from e


def load_state(state_path: str) -> dict:
    """Load the cached digests from the previous run (empty dict if unavailable)."""
    if os.path.isfile(state_path):
        try:
            with open(state_path, encoding="utf-8") as f:
                return json.load(f)
        except (json.JSONDecodeError, OSError):
            pass
    return {}


def save_state(state_path: str, state: dict) -> None:
    """Persist the cached digests for the next run."""
    os.makedirs(os.path.dirname(state_path), exist_ok=True)
    with open(state_path, "w", encoding="utf-8") as f:
        json.dump(state, f, indent=2)


def get_asset_digest(api_tag_url: str) -> str | None:
    """Return the digest of the kwxFetch-source.tar.xz asset for a release tag.

    The GitHub API exposes a sha256 digest for release assets. If the digest
    field is missing (older API data), the asset id is used instead — it also
    changes whenever the asset is re-uploaded. Returns None if the metadata
    cannot be retrieved, in which case the caller should just download.
    """
    req = urllib.request.Request(
        api_tag_url,
        headers={"User-Agent": "kwxFetch-python", "Accept": "application/vnd.github+json"},
    )
    try:
        with urllib.request.urlopen(req) as resp:
            data = json.load(resp)
    except (urllib.error.URLError, json.JSONDecodeError) as e:
        print(f"  Warning: could not fetch release metadata ({e}) — will download tarball.")
        return None
    for asset in data.get("assets", []):
        if asset.get("name") == TARBALL_NAME:
            digest = asset.get("digest") or ""
            return digest.removeprefix("sha256:") or str(asset.get("id"))
    return None


def git_ls_remote_main(cmrc_src_dir: str) -> str | None:
    """Return the current SHA of origin/main without a full fetch, or None on failure."""
    try:
        proc = run_command(["git", "ls-remote", CMRC_REPO_URL, "refs/heads/main"], check=False)
    except OSError:
        return None
    output = (proc.stdout or "").strip().split()
    return output[0] if output else None


def git_local_head(cmrc_src_dir: str) -> str | None:
    """Return the local HEAD SHA of a git repo, or None if it can't be determined."""
    proc = run_command(["git", "-C", cmrc_src_dir, "rev-parse", "HEAD"], check=False)
    output = (proc.stdout or "").strip()
    return output if output else None


def extract_tarball(tarball: str, extract_dir: str) -> str:
    """Extract a tarball and return the source root (handles a single top-level directory)."""
    os.makedirs(extract_dir, exist_ok=True)
    run_command(["tar", "-xf", tarball, "-C", extract_dir])

    src_root = extract_dir
    entries = os.listdir(extract_dir)
    if len(entries) == 1 and os.path.isdir(os.path.join(extract_dir, entries[0])):
        src_root = os.path.join(extract_dir, entries[0])
    return src_root


def sync_robocopy(src_root: str, dest_dir: str, label: str) -> None:
    """Sync src_root over dest_dir with robocopy (/E, no purge — preserves generated files)."""
    print(f"Syncing to {dest_dir} ...")
    # /E = recurse including empty dirs. No /PURGE — preserves generated files (setup.h etc.)
    # /NFL = no file list, /NDL = no dir list, /NJH = no job header, /NP = no progress
    result = run_command(
        ["robocopy", src_root, dest_dir, "/E", "/NFL", "/NDL", "/NJH", "/NP"],
        check=False,
    )
    if result.returncode > 7:
        raise RuntimeError(f"robocopy failed (exit code {result.returncode})")
    print(f"{label} updated.\n")


def main() -> None:
    force = "--force" in sys.argv[1:]

    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.realpath(os.path.join(script_dir, ".."))
    wx_src_dir = os.path.join(repo_root, "build", "_deps", "wxwidgets-src")
    interface_src_dir = os.path.join(repo_root, "build", "_deps", "interface-src")
    state_path = os.path.join(repo_root, "build", STATE_FILE)

    # Verify wxWidgets source exists
    if not is_directory(wx_src_dir):
        print(f"wxWidgets source not found at {wx_src_dir} — run cmake configure first.", file=sys.stderr)
        sys.exit(1)

    # Verify interface source exists
    if not is_directory(interface_src_dir):
        print(f"Interface source not found at {interface_src_dir} — run cmake configure first.", file=sys.stderr)
        sys.exit(1)

    tmp_dir = tempfile.mkdtemp(prefix="kwxFetch-update-")
    try:
        # ─── Phase 1: Download & Sync (skipped when tarball is unchanged) ────
        state = load_state(state_path)
        new_state: dict = {}

        print("Checking for kwxFetch updates ...")
        wx_digest = None if force else get_asset_digest(WX_API_URL)
        if wx_digest is not None and wx_digest == state.get("wx"):
            print("  wxWidgets tarball unchanged — skipping download.")
        else:
            print("Downloading kwxFetch-source.tar.xz ...")
            tarball = os.path.join(tmp_dir, TARBALL_NAME)
            download(WX_URL, tarball, "Download")
            print(f"  Downloaded to {tarball}")

            print("Extracting ...")
            src_root = extract_tarball(tarball, os.path.join(tmp_dir, "extract"))

            sync_robocopy(src_root, wx_src_dir, "wxWidgets source")

        if wx_digest is None:
            new_state.pop("wx", None)
        else:
            new_state["wx"] = wx_digest

        # ─── Interface headers sync ──────────────────────────────────────────
        if not is_directory(interface_src_dir):
            print(f"Interface source not found at {interface_src_dir} — run cmake configure first.", file=sys.stderr)
            sys.exit(1)

        iface_digest = None if force else get_asset_digest(INTERFACE_API_URL)
        if iface_digest is not None and iface_digest == state.get("interface"):
            print("  Interface tarball unchanged — skipping download.")
        else:
            print("Downloading interface headers tarball ...")
            iface_tarball = os.path.join(tmp_dir, "kwxFetch-interface.tar.xz")
            download(INTERFACE_URL, iface_tarball, "Interface download")
            print(f"  Downloaded to {iface_tarball}")

            print("Extracting interface headers ...")
            iface_src_root = extract_tarball(iface_tarball, os.path.join(tmp_dir, "iface-extract"))

            sync_robocopy(iface_src_root, interface_src_dir, "Interface headers")

        if iface_digest is None:
            new_state.pop("interface", None)
        else:
            new_state["interface"] = iface_digest

        save_state(state_path, new_state)

        # ─── CMakeRC update (skipped when already at origin/main) ────────────
        cmrc_src_dir = os.path.join(repo_root, "build", "_deps", "cmrc-src")
        if not is_directory(cmrc_src_dir):
            print(f"CMakeRC source not found at {cmrc_src_dir} — run cmake configure first.", file=sys.stderr)
            sys.exit(1)

        print("Checking CMakeRC (build/_deps/cmrc-src) against origin/main ...")
        remote_head = git_ls_remote_main(cmrc_src_dir)
        local_head = git_local_head(cmrc_src_dir)
        if remote_head and local_head and remote_head == local_head:
            print("  CMakeRC already at origin/main — skipping update.")
        else:
            print("Updating CMakeRC to origin/main ...")
            run_command(["git", "-C", cmrc_src_dir, "fetch", "origin"])
            run_command(["git", "-C", cmrc_src_dir, "reset", "--hard", "origin/main"])
        print("CMakeRC checked.\n")

        # ─── Phase 2: Apply Local Patches ────────────────────────────────────
        overwritten: list[str] = []

        # ─── 2c. .private/wxWidgets overlay ─────────────────────────────────
        private_wx_dir = os.path.join(repo_root, ".private", "wxWidgets")
        if is_directory(private_wx_dir):
            print("\nApplying .private/wxWidgets overlay ...")
            file_count = 0
            for root, _dirs, filenames in os.walk(private_wx_dir):
                for name in filenames:
                    full_path = os.path.join(root, name)
                    rel_path = os.path.relpath(full_path, private_wx_dir).replace("\\", "/")
                    dest_file = os.path.join(wx_src_dir, *rel_path.split("/"))

                    is_overwrite = os.path.isfile(dest_file)
                    os.makedirs(os.path.dirname(dest_file), exist_ok=True)
                    shutil.copyfile(full_path, dest_file)

                    if is_overwrite:
                        overwritten.append(rel_path)
                    file_count += 1
            print(f"  Copied {file_count} file(s) from .private/wxWidgets.")

        # ─── Summary of overwritten files ────────────────────────────────────
        if overwritten:
            print("\nFiles overwritten in build/_deps/wxwidgets-src:")
            for f in sorted(set(overwritten)):
                print(f"  {f}")
    finally:
        # Cleanup temp directory
        shutil.rmtree(tmp_dir, ignore_errors=True)


if __name__ == "__main__":
    main()
