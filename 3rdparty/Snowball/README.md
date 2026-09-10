# Snowball (libstemmer_c)

Vendored copy of the **Snowball** C stemming library, used for word normalization in
wxUiEditor’s full-text search.

- Upstream project: <https://snowballstem.org>
- Source release: <https://snowballstem.org/dist/libstemmer_c-2.2.0.tar.gz> (version
  2.2.0)
- [LICENSE](LICENSE) (and the upstream [COPYING](COPYING))

## Contents

| Directory      | Description                                                     |
|----------------|-----------------------------------------------------------------|
| `include/`     | Public header, `libstemmer.h`                                   |
| `runtime/`     | Snowball runtime (`api.c`, `utilities.c`, and shared headers)   |
| `libstemmer/`  | `libstemmer.c` plus the generated `modules.h`/`modules_utf8.h`  |
| `src_c/`       | Generated stemming algorithms (`stem_*.c` / `stem_*.h`)         |

## Build

The `CMakeLists.txt` in this directory compiles the sources above into the
`snowball_stemmer` **static library**. It is added via
`add_subdirectory(3rdparty/Snowball)` from the top-level `CMakeLists.txt`, and is
consumed by the `ftsrch`/`parser` libraries and the `wxUiEditor` executable.

## Updating

Download a new `libstemmer_c-x.y.z.tar.gz` from
<https://snowballstem.org/download.html>, replace the files in the directories listed
above (do not modify them), and update the version noted here.
