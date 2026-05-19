/**
 * test_run.ts — runs build_resources.exe parser pipeline
 *
 * Stages ~5 headers from the wx interface tree, runs the documentation
 * parser, verifies outputs, and runs --zip-test to validate the archive.
 *
 * Equivalent of test_run.ps1 but for build_resources.exe.
 *
 * Usage: deno run --allow-read --allow-write --allow-run tests/scripts/test_run.ts
 */

import * as path from "@std/path";

// ---------------------------------------------------------------------------
// Paths
// ---------------------------------------------------------------------------

const scriptDir = path.dirname(path.fromFileUrl(import.meta.url));
const repoRoot = path.resolve(scriptDir, "..", "..");

const exe = path.join(
    repoRoot,
    "tools",
    "build_resources",
    "build",
    "Debug",
    "build_resources.exe",
);
const interfaceWx = path.join(
    repoRoot,
    "tools",
    "build_resources",
    "build",
    "_deps",
    "interface-src",
    "wx",
);
const testsDir = path.join(repoRoot, "tests");
const stagingDir = path.join(testsDir, "_zip_input");
const zipOutputDir = path.join(testsDir, "_zip_output");
const zipFile = path.join(zipOutputDir, "test_output.zip");

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

const sampleHeaders = [
    "event.h",
    "clrpicker.h",
    "bitmap.h",
    "app.h",
    "colour.h",
];

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function printBanner(...lines: string[]): void
{
    console.log("-".repeat(72));
    for (const line of lines)
    {
        console.log(line);
    }
    console.log("-".repeat(72));
}

function fail(message: string): never
{
    console.error(`[FAIL] ${message}`);
    Deno.exit(1);
}

async function fileSize(filePath: string): Promise<number>
{
    const info = await Deno.stat(filePath);
    return info.size;
}

// ---------------------------------------------------------------------------
// Stage 0 – Sanity checks
// ---------------------------------------------------------------------------

{
    const exeExists = await Deno.stat(exe).then(() => true).catch(() => false);
    if (!exeExists)
    {
        fail(
            `Executable not found: ${exe}\n`
                + "Build the Debug configuration first: cmake --build build --config Debug",
        );
    }
}

{
    const wxExists = await Deno.stat(interfaceWx).then(() => true).catch(
        () => false,
    );
    if (!wxExists)
    {
        fail(
            `Interface headers not found: ${interfaceWx}\n`
                + "Run CMake configure so FetchContent populates build/_deps/interface-src/.",
        );
    }
}

for (const h of sampleHeaders)
{
    const hPath = path.join(interfaceWx, h);
    const exists = await Deno.stat(hPath).then(() => true).catch(() => false);
    if (!exists)
    {
        fail(`Sample header missing from interface tree: ${h}`);
    }
}

// ---------------------------------------------------------------------------
// Stage 1 – Stage input headers
// ---------------------------------------------------------------------------

// Remove and recreate staging directory
await Deno.remove(stagingDir, { recursive: true }).catch(() =>
{});
await Deno.mkdir(stagingDir, { recursive: true });

for (const h of sampleHeaders)
{
    await Deno.copyFile(path.join(interfaceWx, h), path.join(stagingDir, h));
}

// ---------------------------------------------------------------------------
// Stage 2 – Clean previous outputs
// ---------------------------------------------------------------------------

const expectedMdFiles = sampleHeaders.map((h) => path.join(stagingDir, h.replace(/\.h$/, ".md")));
const expectedKfts = path.join(stagingDir, "data", "search_index.kfts");
const expectedMap = path.join(stagingDir, "data", "doc_map.json");
const expectedIdx = path.join(stagingDir, "index.md");

const cleanup = [zipFile, expectedKfts, expectedMap, expectedIdx, ...expectedMdFiles];

for (const stale of cleanup)
{
    await Deno.remove(stale).catch(() =>
    {});
}

// ---------------------------------------------------------------------------
// Stage 3 – Run parser
// ---------------------------------------------------------------------------

const runArgs = [
    "--parse",
    "--srcdir",
    stagingDir,
    "--outdir",
    stagingDir,
    "--zip-path",
    zipFile,
    "--verbose",
];

console.log(`Running: ${exe} ${runArgs.join(" ")}`);
printBanner();

const runCmd = new Deno.Command(exe, {
    args: runArgs,
    stdout: "inherit",
    stderr: "inherit",
});
const runResult = await runCmd.output();

printBanner();

// ---------------------------------------------------------------------------
// Stage 4 – Verify expected outputs
// ---------------------------------------------------------------------------

let ok = true;

for (const md of expectedMdFiles)
{
    try
    {
        const size = await fileSize(md);
        console.log(`[PASS] ${md}  (${size} bytes)`);
    }
    catch
    {
        console.log(`[FAIL] ${md}  -- NOT FOUND`);
        ok = false;
    }
}

for (
    const [, file] of Object.entries({
        "search_index.kfts": expectedKfts,
        "doc_map.json": expectedMap,
        "index.md": expectedIdx,
    })
)
{
    try
    {
        const size = await fileSize(file);
        console.log(`[PASS] ${file}  (${size} bytes)`);
    }
    catch
    {
        console.log(`[FAIL] ${file}  -- NOT FOUND`);
        ok = false;
    }
}

// ---------------------------------------------------------------------------
// Stage 5 – Verify ZIP archive
// ---------------------------------------------------------------------------

try
{
    const zipSize = await fileSize(zipFile);
    console.log(`[PASS] ${zipFile}  (${zipSize} bytes)`);

    // List ZIP contents via PowerShell
    const psCmd = new Deno.Command("powershell", {
        args: [
            "-NoProfile",
            "-Command",
            `
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $a = [System.IO.Compression.ZipFile]::OpenRead('${zipFile.replace(/'/g, "''")}')
        Write-Host ""
        Write-Host "ZIP contents:"
        foreach ($e in $a.Entries) {
          $r = if ($e.Length -gt 0) { [math]::Round((1.0 - $e.CompressedLength / $e.Length) * 100, 1) } else { 0 }
          Write-Host ("  {0,-40} {1,8} bytes  ({2}% compression)" -f $e.FullName, $e.Length, $r)
        }
        $a.Dispose()
      `,
        ],
        stdout: "inherit",
        stderr: "inherit",
    });
    await psCmd.output();
}
catch
{
    console.log("  (could not list ZIP contents)");
}

// ---------------------------------------------------------------------------
// Stage 6 – Run zip-test
// ---------------------------------------------------------------------------

if (ok)
{
    console.log("");
    const testArgs = ["--zip-test", zipFile];
    console.log(`Running: ${exe} ${testArgs.join(" ")}`);
    printBanner();

    const testCmd = new Deno.Command(exe, {
        args: testArgs,
        stdout: "inherit",
        stderr: "inherit",
    });
    const testResult = await testCmd.output();

    printBanner();

    if (!testResult.success)
    {
        console.error(`build_resources --zip-test exited with code ${testResult.code}`);
        ok = false;
    }
}

// ---------------------------------------------------------------------------
// Exit
// ---------------------------------------------------------------------------

if (!runResult.success)
{
    fail(`build_resources --parse exited with code ${runResult.code}`);
}

if (!ok)
{
    fail("One or more expected output files are missing.");
}

console.log("");
console.log("All checks passed.");
Deno.exit(0);
