# miniz

Vendored copy of **miniz**, a single-file, zlib-compatible DEFLATE/INFLATE
implementation that also provides a ZIP archive reading and writing API.

- Upstream project: <https://github.com/richgel999/miniz>
- [@richgel999](https://github.com/richgel999))
- Version: 3.1.2 (`MZ_VERSION` in [`miniz.h`](miniz.h) is `"11.3.2"`)
- [LICENSE](LICENSE)

## What it is used for

wxUiEditor uses miniz to read and write the ZIP archives that hold the bundled wxWidgets
API reference (`wxWidgetsDocs.zip`). The `docparser` static library wraps the `mz_zip_*`
API:

- `zip_writer.cpp` — creates the documentation archive
- `zip_reader.cpp` — extracts markdown entries from an existing archive
- `zip_test.cpp` — validates archive contents

libdeflate is linked alongside miniz and supplies the actual DEFLATE compression
backend; miniz owns the ZIP container (local file headers, central directory, and entry
metadata).

## Contents

| File(s)                | Description                                             |
|------------------------|---------------------------------------------------------|
| `miniz.c` / `miniz.h`  | Core zlib-style API (deflate/inflate, CRC-32, Adler-32) |
| `miniz_tdef.c` / `.h`  | Low-level DEFLATE compressor (`tdefl`)                  |
| `miniz_tinfl.c` / `.h` | Low-level INFLATE decompressor (`tinfl`)                |
| `miniz_zip.c` / `.h`   | ZIP archive reader/writer API (`mz_zip_*`)              |
| `miniz_common.h`       | Shared internal definitions                             |

## Build

The `CMakeLists.txt` in this directory compiles the sources above into the `miniz`
**static library**. It is added via `add_subdirectory(3rdparty/miniz)` from the
top-level `CMakeLists.txt` and consumed by the `docparser` library.

`miniz.h` includes a generated `miniz_export.h`, so the target runs CMake’s
`generate_export_header()` and exposes the generated header directory to consumers.

## Updating

Download a newer source release from <https://github.com/richgel999/miniz>, replace the
files in this directory (do not modify them), and update the version noted above.
