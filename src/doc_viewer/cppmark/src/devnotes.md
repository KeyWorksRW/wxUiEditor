# cppmark — Agent Developer Notes

## What it is

cppmark is a C++23 adaptation of cmark-gfm (CommonMark + GitHub Flavored Markdown).
It is a **static library** (`add_library(cppmark STATIC)`) with no external
dependencies beyond the C++ standard library. It is **not** related to or used by
the parser pipeline (`parser/`); it is used exclusively by the viewer GUI to
convert user-visible markdown into HTML for display in `wxHtmlWindow`.

## Single public API entry point

The only public header is:

```
cppmark/include/cppmark_html.h
```

It exposes exactly one function and option constants:

```cpp
std::string cmark_markdown_to_html(std::string_view text, int options);
```

The viewer calls this in **two places** in `viewer/mainframe.cpp`:

1. `OpenArchive()` (line 85) — when initially loading the doc archive
2. `DisplayArchivePage()` (line 130) — when navigating to a page

Both pass `CMARK_OPT_GITHUB_PRE_LANG` as the options flag:

```cpp
const std::string html_text = cmark_markdown_to_html(markdown, CMARK_OPT_GITHUB_PRE_LANG);
m_html_win->SetPage(wxString::FromUTF8(html_text));
```

## Option flags (from `cppmark_html.h`)

| Flag | Value | Purpose |
|------|-------|---------|
| `CMARK_OPT_DEFAULT` | 0 | No options |
| `CMARK_OPT_SOURCEPOS` | 1<<1 | Add data-sourcepos attributes |
| `CMARK_OPT_HARDBREAKS` | 1<<2 | Treat newlines as `<br>` |
| `CMARK_OPT_SAFE` | 1<<3 | Filter raw HTML (deprecated) |
| `CMARK_OPT_NOBREAKS` | 1<<4 | Collapse all softbreaks |
| `CMARK_OPT_NORMALIZE` | 1<<8 | Normalize tree |
| `CMARK_OPT_VALIDATE_UTF8` | 1<<9 | Validate UTF-8 input |
| `CMARK_OPT_SMART` | 1<<10 | Smart quotes/typographic |
| `CMARK_OPT_GITHUB_PRE_LANG` | 1<<11 | `<pre lang="...">` for code blocks |
| `CMARK_OPT_LIBERAL_HTML_TAG` | 1<<12 | Lenient HTML tag parsing |
| `CMARK_OPT_FOOTNOTES` | 1<<13 | Enable footnote syntax |
| `CMARK_OPT_STRIKETHROUGH_DOUBLE_TILDE` | 1<<14 | `~~strike~~` |
| `CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES` | 1<<15 | Table style attrs |
| `CMARK_OPT_FULL_INFO_STRING` | 1<<16 | Full info string on code |
| `CMARK_OPT_UNSAFE` | 1<<17 | Allow raw HTML |

**Important:** The viewer only uses `CMARK_OPT_GITHUB_PRE_LANG`. This flag
causes fenced code blocks to render as `<pre lang="LANG">` (instead of the
default `<pre><code class="language-LANG">`). This is because wxHtmlWindow
handles `<pre lang="...">` better than `<code class="...">`.

## Internal architecture

### Implementation entry point (`src/cmark.cpp`)

`cmark_markdown_to_html()` does the following:

1. Creates a parser via `cmark_parser_new(options)`
2. Creates and attaches the **GFM table extension** via `create_table_extension()`
3. Feeds the markdown text to the parser
4. Finishes parsing to get the AST root (`cmark_node* document`)
5. Renders the AST to HTML via `cmark_render_html(document, options, extensions)`
6. Frees the parser, extension, and node tree
7. Returns the HTML string

The table extension is the **only syntax extension** registered. There is no
global extension registry usage — the extension is created fresh per call.

### Two-phase parsing

cppmark follows the CommonMark spec parsing strategy:

**Phase 1 — Block structure** (`src/blocks.cpp`, 1929 lines):
- `cmark_parse_document()` is the entry point.
- Reads input line-by-line, matches block patterns (paragraphs, headings,
  lists, code fences, blockquotes, thematic breaks, HTML blocks, footnote
  definitions).
- Builds an AST of block-level `cmark_node` objects.
- Uses a `cmark_parser` struct (`src/parser.hxx`) to track parsing state:
  current line, indentation, open blocks, reference map.
- Terminates at configurable `MAX_LIST_DEPTH` (100) to avoid quadratic perf
  issues on deeply nested lists.
- The GFM table extension hooks into this phase via `try_opening_block` to
  detect pipe-delimited tables after a paragraph is finalized.

**Phase 2 — Inline parsing** (`src/inlines.cpp`, 2239 lines):
- `cmark_parse_inlines()` is called on each block node that can contain inline
  content.
- Parses emphasis (`**bold**`, `*italic*`), links `[text](url)`, images,
  code spans (backticks), HTML inline, autolinks, and hard/soft line breaks.
- Uses a delimiter stack for nested emphasis resolution.
- Uses a bracket stack for link/image matching.
- Reference links `[ref]` look up entries in the `cmark_map` hash map
  populated during Phase 1.

### AST node representation (`src/node.hxx`)

The `cmark_node` struct is a tree node with:
- Doubly-linked siblings (`next`/`prev`)
- Parent + first/last child pointers
- A `type` field (block vs inline, see `cmark_node_type` enum)
- A union `as` for type-specific data: `cmark_chunk literal`, `cmark_list`,
  `cmark_code`, `cmark_heading`, `cmark_link`, `cmark_custom`, etc.
- Footnote tracking fields (`ref_ix`, `parent_footnote_def`)
- Extension pointer for custom node types (tables use this)
- User-data pointer with free-function callback

Node types are split into blocks (DOCUMENT, PARAGRAPH, HEADING, LIST, ITEM,
CODE_BLOCK, HTML_BLOCK, BLOCK_QUOTE, THEMATIC_BREAK, CUSTOM_BLOCK,
FOOTNOTE_DEFINITION) and inlines (TEXT, SOFTBREAK, LINEBREAK, CODE, HTML_INLINE,
CUSTOM_INLINE, EMPH, STRONG, LINK, IMAGE, FOOTNOTE_REFERENCE).

### Tree iteration (`src/iterator.cpp`, `src/iterator.hxx`)

The `cmark_iter` provides ENTER/EXIT event-based traversal:
- Create with `cmark_iter_new(root)`
- Loop calling `cmark_iter_next()` until `CMARK_EVENT_DONE`
- Get current node with `cmark_iter_get_node()`
- Leaf nodes never produce EXIT events.
- Used by the HTML renderer and other output format renderers.

### Rendering (`src/html.cpp`, `src/render.cpp`)

- `cmark_render_html()` in `html.cpp` is the main HTML renderer.
- Walks the AST with a `cmark_iter`, calling `S_render_node()` for each
  ENTER/EXIT event.
- `S_render_node()` is a 600-line switch over all node types, producing
  appropriate HTML tags (e.g., `<h1>`, `<p>`, `<ul>`, `<li>`, `<pre><code>`).
- Table rendering is handled by the extension's `html_render_func` callback
  (defined in `src/table.cpp`).
- `src/render.cpp` provides the base `cmark_render()` framework with line
  wrapping, escaping, and carriage-return tracking. All output format renderers
  (HTML, CommonMark, LaTeX, man, plaintext, XML) build on this.

### Syntax extension system (`src/syntax_extension.cpp/hxx`)

The `cmark_syntax_extension` struct is a set of function pointers (≈virtual
methods) that hook into parsing and rendering:

- `last_block_matches` — called during block phase for open blocks owned by
  the extension
- `try_opening_block` — called when no standard block matches a new line
- `match_inline` / `insert_inline_from_delim` — inline parsing hooks
- `html_render_func` — renders custom nodes to HTML
- `postprocess_func` — called after parsing completes

Only the **table extension** (`src/table.cpp`, 917 lines) is currently
implemented. It registers three custom node types: `CMARK_NODE_TABLE`,
`CMARK_NODE_TABLE_ROW`, `CMARK_NODE_TABLE_CELL`.

### Internal data structures

**`CMarkStringBuffer`** (`src/buffer.hxx`, modernized — `std::string` backend):
- Growable byte buffer used throughout the parser and renderer.
- Supports append, trim, search, and whitespace normalization.
- Also aliased as `cmark_strbuf` for backward compat during migration.

**`cmark_chunk`** (`src/chunk.hxx`, legacy — manual memory):
- Non-owning view into text (similar to `std::string_view` but allows owning).
- When `alloc == 0`: view into input buffer; when `alloc != 0`: owns `new[]` memory.
- Used extensively during parsing to avoid copies.
- Not yet modernized; still uses raw `unsigned char*` with `new[]`/`delete[]`.

**`cmark_map`** (`src/map.hxx`, `src/map.cpp`):
- Chained hash map for reference links and footnotes.
- Labels are normalized (case-folded via UTF-8 case folding, whitespace collapsed)
  for case-insensitive lookup.
- Used by `references.cpp` for `[ref]: url "title"` definitions.
- Used by `footnotes.cpp` for `[^fn]: definition` entries.

**`cmark_llist`** (in `cmark-gfm.hxx`, `linked_list.cpp`):
- Singly-linked list for extension lists and filter chains.
- Phase 1.5 modernization: uses `std::make_unique` internally but still exposes
  raw `cmark_llist*` pointers.

### Locale-independent character classification (`src/cmark_ctype.cpp/hxx`)

- `cmark_isspace()`, `cmark_ispunct()`, `cmark_isalnum()`, `cmark_isdigit()`,
  `cmark_isalpha()` — all ASCII-only, locale-independent.
- Used instead of `<cctype>` functions because markdown spec requires ASCII
  behavior regardless of system locale.

### UTF-8 handling (`src/utf8.cpp/hxx`)

- `cmark_utf8proc_iterate()` — decode UTF-8 multi-byte sequences
- `cmark_utf8proc_is_space()` / `cmark_utf8proc_is_punctuation()` — Unicode-aware
  whitespace/punctuation detection (not just ASCII)
- `cmark_utf8proc_case_fold()` — case folding for reference label normalization
- `cmark_utf8proc_encode_char()` — encode a Unicode code point as UTF-8
- Uses large generated tables from `case_fold_switch.inc` and `entities.inc`

### HTML/URL escaping (`src/houdini_*.cpp`, `src/houdini.hxx`)

Houdini library (MIT license) for:
- `houdini_escape_html()` / `houdini_unescape_html()` — HTML entity escaping
- `houdini_escape_href()` — URL percent-encoding for href attributes
- `houdini_unescape_ent()` — decode a single HTML entity

### Scanners (`src/scanners.cpp/hxx`)

- Auto-generated from a re2c `.re` file (do not edit `_scan_*` functions directly).
- Fast lookahead pattern matchers for: HTML tags, URIs/autolinks, code fences,
  ATX/setext headings, link titles, entities, and dangerous URLs.
- Called from block and inline parsers to detect syntax boundaries.

### Footnotes (`src/footnotes.cpp/hxx`)

- Creates `cmark_footnote` entries when the parser finds `[^label]: content`.
- Maps hold `cmark_footnote` structs linking label → AST node + index.
- `cmark_unlink_footnotes_map()` prevents use-after-free when two unused
  footnote definitions reference each other.

## Build integration

- cppmark is built **before** wxWidgets (in `CMakeLists.txt` line 226).
- This is intentional — cppmark has no wxWidgets dependency, so building it
  first avoids pulling wx headers into cppmark translation units.
- Linked to `wxViewer` via `target_link_libraries(wxViewer PRIVATE cppmark)`.
- C++23 required (`CXX_STANDARD 23`).
- Precompiled headers: `<array>`, `<cassert>`, `<climits>`, `<cstdint>`,
  `<cstdio>`, `<cstdlib>`, `<cstring>`, `<memory>`, `<string>`, `<string_view>`.

## Modernization status

The source files contain comments with "Phase 1.5" and "Phase 2" labels:

- **Phase 2 complete** (modernized): `CMarkStringBuffer` (`buffer.hxx`) —
  fully uses `std::string` backend.
- **Phase 1.5 complete** (smart pointers): `node.cpp`, `iterator.cpp`,
  `linked_list.cpp`, `map.cpp`, `references.cpp`, `footnotes.cpp`,
  `syntax_extension.cpp` — uses `std::unique_ptr` internally but still exposes
  raw pointers in the public API.
- **Phase 1.5 candidate** (still legacy `malloc`/`calloc`): `blocks.cpp`,
  `inlines.cpp`, `html.cpp`, `render.cpp`, `chunk.cpp`, `registry.cpp`,
  `parser.hxx` internals, `scanners.cpp`, `syntax_extension.hxx`.
- **Not to be modernized**: `scanners.cpp` (auto-generated from re2c).

## Key files at a glance

| File | Purpose |
|------|---------|
| `include/cppmark_html.h` | **Only public header** — `cmark_markdown_to_html()` + option flags |
| `src/cmark.cpp` | Implements `cmark_markdown_to_html()` — wires parser + table extension + renderer |
| `src/cmark-gfm.hxx` | Main internal header — all node types, parser API, render API, accessors |
| `src/cmark-gfm-extension_api.hxx` | Extension API — hook function typedefs and registration |
| `src/blocks.cpp` | Phase 1 block parser (1929 lines) |
| `src/inlines.cpp` | Phase 2 inline parser (2239 lines) |
| `src/html.cpp` | HTML renderer (646 lines) |
| `src/render.cpp` | Base rendering framework (275 lines) |
| `src/table.cpp` | GFM table extension (917 lines) |
| `src/node.hxx` | AST node struct definition |
| `src/buffer.hxx` | `CMarkStringBuffer` (modernized `std::string`-backed buffer) |
| `src/chunk.hxx` | `cmark_chunk` (legacy non-owning/owning string slice) |
| `src/parser.hxx` | `cmark_parser` state struct |
| `src/map.hxx` | Hash map for reference links and footnotes |
| `src/scanners.hxx` | re2c-generated pattern matchers |
| `src/utf8.hxx` | UTF-8 decode/case-fold/classification |
| `src/houdini.hxx` | HTML/URL escaping |
| `src/footnotes.hxx` | Footnote definition storage |
| `src/references.hxx` | Reference link storage |
| `src/syntax_extension.hxx` | Extension struct with all hook function pointers |

## What an agent needs to know when editing

1. **Don't break the public API.** The only function the viewer calls is
   `cmark_markdown_to_html()`. If you change its signature, update
   `viewer/mainframe.cpp`.

2. **The table extension is the only extension.** It's created per-call in
   `cmark.cpp`. There's no global `cmark_register_plugin()` usage.

3. **Memory management is mixed.** Some subsystems use modern C++ (`std::string`,
   `std::unique_ptr`); others still use raw `new[]`/`delete[]` or
   `malloc`/`free`. Be careful when mixing.

4. **Options matter for the viewer.** The viewer uses `CMARK_OPT_GITHUB_PRE_LANG`.
   Changes to how this option works will affect HTML output in the viewer.

5. **`CMarkStringBuffer` is the active buffer class.** `buffer_new.hxx` contains
   an alternative `cmark_strbuf` class that is **not currently used** — prefer
   `buffer.hxx`'s `CMarkStringBuffer`.

6. **The short-name macros** (`#define NODE_DOCUMENT CMARK_NODE_DOCUMENT` etc.)
   at the bottom of `cmark-gfm.hxx` exist for backward compat. New code should
   use the `CMARK_NODE_*` names directly.

7. **scanners.cpp is auto-generated** from a re2c `.re` file. Do not edit the
   `_scan_*` functions manually; edit the `.re` source and re-run re2c instead.
