/**
 * full_run.ts — runs build_resources.exe parser pipeline against the full
 * wxWidgets interface tree, producing markdown docs + a ZIP archive.
 *
 * Equivalent of full_run.ps1 but for build_resources.exe.
 *
 * Usage: deno run --allow-read --allow-write --allow-run tests/scripts/full_run.ts
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
const outputDir = path.join(testsDir, "full_build");
const zipFile = path.join(testsDir, "wxWidgetsDocs.zip");

// ---------------------------------------------------------------------------
// Stage 0 – Sanity checks
// ---------------------------------------------------------------------------

{
    const exists = await Deno.stat(exe).then(() => true).catch(() => false);
    if (!exists)
    {
        console.error(
            `Executable not found: ${exe}\n`
                + "Build the Debug configuration first: cmake --build build --config Debug",
        );
        Deno.exit(1);
    }
}

{
    const exists = await Deno.stat(interfaceWx).then(() => true).catch(
        () => false,
    );
    if (!exists)
    {
        console.error(
            `Interface headers not found: ${interfaceWx}\n`
                + "Run CMake configure so FetchContent populates build/_deps/interface-src/.",
        );
        Deno.exit(1);
    }
}

// ---------------------------------------------------------------------------
// Stage 1 – Clean previous outputs
// ---------------------------------------------------------------------------

await Deno.remove(outputDir, { recursive: true }).catch(() =>
{});
await Deno.remove(zipFile).catch(() =>
{});
await Deno.mkdir(outputDir, { recursive: true });

// ---------------------------------------------------------------------------
// Stage 2 – Run parser
// ---------------------------------------------------------------------------

const runArgs = [
    "--parse",
    "--srcdir",
    interfaceWx,
    "--outdir",
    outputDir,
    "--zip-path",
    zipFile,
];

console.log(`Running: ${exe} ${runArgs.join(" ")}`);

const startMs = performance.now();

const runCmd = new Deno.Command(exe, {
    args: runArgs,
    stdout: "null",
    stderr: "inherit",
});
const runResult = await runCmd.output();

const elapsedMs = performance.now() - startMs;

// ---------------------------------------------------------------------------
// Stage 3 – Verify expected outputs
// ---------------------------------------------------------------------------

if (!runResult.success)
{
    console.error(
        `build_resources --parse exited with code ${runResult.code}`,
    );
    Deno.exit(runResult.code);
}

const expectedFiles = [
    path.join(outputDir, "index.md"),
    path.join(outputDir, "data", "search_index.kfts"),
    path.join(outputDir, "data", "doc_map.json"),
    zipFile,
];

for (const filePath of expectedFiles)
{
    try
    {
        await Deno.stat(filePath);
    } catch
    {
        console.error(`Expected output not found: ${filePath}`);
        Deno.exit(1);
    }
}

// ---------------------------------------------------------------------------
// Stage 4 – Summary
// ---------------------------------------------------------------------------

// Count markdown files recursively
let markdownCount = 0;
for await (const entry of Deno.readDir(outputDir))
{
    if (entry.isDirectory)
    {
        // Simple recursive count for subdirectories
        markdownCount += await countMdFiles(path.join(outputDir, entry.name));
    } else if (entry.name.endsWith(".md"))
    {
        markdownCount++;
    }
}

const zipSize = (await Deno.stat(zipFile)).size;

// Format elapsed time as hh:mm:ss.fff
const totalSeconds = elapsedMs / 1000;
const hours = Math.floor(totalSeconds / 3600);
const minutes = Math.floor((totalSeconds % 3600) / 60);
const seconds = totalSeconds % 60;
const elapsedStr = `${String(hours).padStart(2, "0")}:${String(minutes).padStart(2, "0")}:${
    seconds.toFixed(3).padStart(7, "0")
}`;

console.log(`Elapsed:        ${elapsedStr}`);
console.log(`Markdown files: ${markdownCount}`);
console.log(`ZIP size:       ${zipSize} bytes`);
console.log(`Output dir:     ${outputDir}`);
console.log(`ZIP file:       ${zipFile}`);

Deno.exit(0);

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------

async function countMdFiles(dir: string): Promise<number>
{
    let count = 0;
    try
    {
        for await (const entry of Deno.readDir(dir))
        {
            if (entry.isDirectory)
            {
                count += await countMdFiles(path.join(dir, entry.name));
            } else if (entry.name.endsWith(".md"))
            {
                count++;
            }
        }
    } catch
    {
        // Permission denied or missing dir — skip
    }
    return count;
}
