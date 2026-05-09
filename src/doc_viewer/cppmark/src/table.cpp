// GFM table syntax extension
// Purpose: Implements GitHub Flavored Markdown table parsing and HTML rendering
// Key types: NodeTable, NodeTableRow, HtmlTableState
// Key functions: create_table_extension(), try_opening_table_block(), html_render()
// Dependencies: cmark-gfm-extension_api.hxx, node.hxx, parser.hxx, render.hxx

#include "table.hxx"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include "buffer.hxx"
#include "cmark-gfm-extension_api.hxx"
#include "cmark-gfm.hxx"
#include "html.hxx"
#include "node.hxx"
#include "parser.hxx"
#include "render.hxx"

// ---------------------------------------------------------------------------
// Global node type variable definitions
// ---------------------------------------------------------------------------

cmark_node_type CMARK_NODE_TABLE = static_cast<cmark_node_type>(0);
cmark_node_type CMARK_NODE_TABLE_ROW = static_cast<cmark_node_type>(0);
cmark_node_type CMARK_NODE_TABLE_CELL = static_cast<cmark_node_type>(0);

constexpr int MAX_AUTOCOMPLETED_CELLS = 0x80000;

static CMarkNodeInternalFlags CMARK_NODE_TABLE_VISITED = CMarkNodeInternalFlags { 0 };

// ---------------------------------------------------------------------------
// Per-node data structures
// ---------------------------------------------------------------------------

struct NodeTable
{
    uint16_t n_columns = 0;
    std::vector<uint8_t> alignments;
    int n_rows = 0;
    int n_nonempty_cells = 0;
};

struct NodeTableRow
{
    bool is_header = false;
};

struct HtmlTableState
{
    bool need_closing_tbody = false;
    bool in_thead = false;
};

// ---------------------------------------------------------------------------
// Row-parsing structures
// ---------------------------------------------------------------------------

struct CellData
{
    std::string content;
    int start_offset = 0;
    int end_offset = 0;
    int internal_offset = 0;
};

struct TableRow
{
    std::vector<CellData> cells;
    int paragraph_offset = 0;
};

// ---------------------------------------------------------------------------
// Flag helpers
// ---------------------------------------------------------------------------

static bool flag_has(const cmark_node* node, CMarkNodeInternalFlags flag)
{
    return (static_cast<uint8_t>(node->flags) & static_cast<uint8_t>(flag)) != 0;
}

static void flag_set(cmark_node* node, CMarkNodeInternalFlags flag)
{
    node->flags = static_cast<CMarkNodeInternalFlags>(static_cast<uint8_t>(node->flags) |
                                                      static_cast<uint8_t>(flag));
}

// ---------------------------------------------------------------------------
// Scanner functions
// ---------------------------------------------------------------------------

static size_t scan_table_start(unsigned char* ptr, int len, size_t offset)
{
    const unsigned char saved = ptr[len];
    ptr[len] = '\0';
    const unsigned char* ptr_p = ptr + offset;
    const unsigned char* end = ptr + len;

    // Skip optional leading pipe
    if (*ptr_p == '|')
    {
        ++ptr_p;
    }

    int cell_count = 0;

    while (ptr_p < end && *ptr_p != '\n' && *ptr_p != '\0')
    {
        // Skip leading spaces
        while (*ptr_p == ' ' || *ptr_p == '\t')
        {
            ++ptr_p;
        }

        // Trailing pipe followed by end-of-line -> done
        if (*ptr_p == '\n' || *ptr_p == '\0')
        {
            break;
        }

        // Optional leading colon
        if (*ptr_p == ':')
        {
            ++ptr_p;
        }

        // Must have at least one dash
        if (*ptr_p != '-')
        {
            ptr[len] = saved;
            return 0;
        }
        while (*ptr_p == '-')
        {
            ++ptr_p;
        }

        // Optional trailing colon
        if (*ptr_p == ':')
        {
            ++ptr_p;
        }

        // Skip trailing spaces
        while (*ptr_p == ' ' || *ptr_p == '\t')
        {
            ++ptr_p;
        }

        ++cell_count;

        if (*ptr_p == '|')
        {
            ++ptr_p;
        }
        else if (*ptr_p != '\n' && *ptr_p != '\0')
        {
            ptr[len] = saved;
            return 0;
        }
    }

    if (cell_count == 0)
    {
        ptr[len] = saved;
        return 0;
    }

    // Skip optional trailing spaces, then newline
    while (*ptr_p == ' ' || *ptr_p == '\t')
    {
        ++ptr_p;
    }
    if (*ptr_p == '\n')
    {
        ++ptr_p;
    }

    const size_t result = static_cast<size_t>(ptr_p - (ptr + offset));
    ptr[len] = saved;
    return result;
}

[[maybe_unused]] static size_t scan_table_cell(unsigned char* ptr, int len, size_t offset)
{
    const unsigned char saved = ptr[len];
    ptr[len] = '\0';
    const unsigned char* ptr_p = ptr + offset;
    const unsigned char* end = ptr + len;

    if (ptr_p >= end || *ptr_p == '|' || *ptr_p == '\n' || *ptr_p == '\0')
    {
        ptr[len] = saved;
        return 0;
    }

    while (ptr_p < end && *ptr_p != '|' && *ptr_p != '\n' && *ptr_p != '\0')
    {
        if (*ptr_p == '\\' && *(ptr_p + 1) != '\0' && *(ptr_p + 1) != '\n')
        {
            ptr_p += 2;
        }
        else
        {
            ++ptr_p;
        }
    }

    const size_t result = static_cast<size_t>(ptr_p - (ptr + offset));
    ptr[len] = saved;
    return result;
}

[[maybe_unused]] static size_t scan_table_cell_end(unsigned char* ptr, int len, size_t offset)
{
    const unsigned char saved = ptr[len];
    ptr[len] = '\0';
    const unsigned char* ptr_p = ptr + offset;
    const unsigned char* end = ptr + len;

    if (ptr_p >= end || *ptr_p != '|')
    {
        ptr[len] = saved;
        return 0;
    }
    ++ptr_p;
    while (ptr_p < end && (*ptr_p == ' ' || *ptr_p == '\t'))
    {
        ++ptr_p;
    }

    const size_t result = static_cast<size_t>(ptr_p - (ptr + offset));
    ptr[len] = saved;
    return result;
}

[[maybe_unused]] static size_t scan_table_row_end(unsigned char* ptr, int len, size_t offset)
{
    const unsigned char saved = ptr[len];
    ptr[len] = '\0';
    const unsigned char* ptr_p = ptr + offset;
    const unsigned char* end = ptr + len;

    while (ptr_p < end && (*ptr_p == ' ' || *ptr_p == '\t'))
    {
        ++ptr_p;
    }

    if (ptr_p >= end || (*ptr_p != '\n' && *ptr_p != '\0'))
    {
        ptr[len] = saved;
        return 0;
    }
    if (*ptr_p == '\n')
    {
        ++ptr_p;
    }

    const size_t result = static_cast<size_t>(ptr_p - (ptr + offset));
    ptr[len] = saved;
    return result;
}

// ---------------------------------------------------------------------------
// String helpers
// ---------------------------------------------------------------------------

static std::string trim_string(const std::string& str)
{
    const size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return {};
    }
    const size_t end_pos = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end_pos - start + 1);
}

static uint8_t get_alignment(const std::string& cell_content)
{
    const bool starts_colon = !cell_content.empty() && cell_content.front() == ':';
    const bool ends_colon = !cell_content.empty() && cell_content.back() == ':';
    if (starts_colon && ends_colon)
    {
        return static_cast<uint8_t>('c');
    }
    if (ends_colon)
    {
        return static_cast<uint8_t>('r');
    }
    if (starts_colon)
    {
        return static_cast<uint8_t>('l');
    }
    return 0U;
}

static std::optional<TableRow> row_from_string(cmark_syntax_extension* /*self*/,
                                               cmark_parser* /*parser*/, unsigned char* string,
                                               int len)
{
    if (len <= 0 || string == nullptr)
    {
        return std::nullopt;
    }

    const unsigned char saved = string[len];
    string[len] = '\0';

    const unsigned char* ptr_p = string;
    const unsigned char* end = string + len;
    TableRow result;

    // Skip optional leading pipe
    if (*ptr_p == '|')
    {
        ++ptr_p;
    }

    while (ptr_p < end && *ptr_p != '\n' && *ptr_p != '\0')
    {
        // Skip spaces at start of cell
        while (ptr_p < end && (*ptr_p == ' ' || *ptr_p == '\t'))
        {
            ++ptr_p;
        }

        // End of line after trailing pipe
        if (ptr_p >= end || *ptr_p == '\n' || *ptr_p == '\0')
        {
            break;
        }

        CellData cell;
        cell.start_offset = static_cast<int>(ptr_p - string);

        // Collect cell content with escape handling
        std::string content;
        while (ptr_p < end && *ptr_p != '|' && *ptr_p != '\n' && *ptr_p != '\0')
        {
            if (*ptr_p == '\\' && (ptr_p + 1) < end && *(ptr_p + 1) != '\0' && *(ptr_p + 1) != '\n')
            {
                // Unescape: \| -> |, \\ -> \, etc.
                content += static_cast<char>(*(ptr_p + 1));
                ptr_p += 2;
            }
            else
            {
                content += static_cast<char>(*ptr_p);
                ++ptr_p;
            }
        }

        cell.end_offset = static_cast<int>(ptr_p - string);
        cell.content = trim_string(content);
        result.cells.push_back(std::move(cell));

        // Consume pipe separator
        if (ptr_p < end && *ptr_p == '|')
        {
            ++ptr_p;
        }
    }

    string[len] = saved;

    if (result.cells.empty())
    {
        return std::nullopt;
    }
    return result;
}

// ---------------------------------------------------------------------------
// Row-count helper
// ---------------------------------------------------------------------------

static void incr_table_row_count(cmark_node* table_node)
{
    NodeTable* node_table = static_cast<NodeTable*>(table_node->as.opaque);
    ++node_table->n_rows;
}

// ---------------------------------------------------------------------------
// try_opening_table_header
// ---------------------------------------------------------------------------

static cmark_node* try_opening_table_header(cmark_syntax_extension* self, cmark_parser* parser,
                                            cmark_node* parent, unsigned char* input, int len)
{
    // Step 1: Already visited -> skip re-check
    if (flag_has(parent, CMARK_NODE_TABLE_VISITED))
    {
        return parent;
    }

    // Step 2: scan_table_start at first_nonspace -- must match
    if (scan_table_start(input, len, parser->first_nonspace) == 0)
    {
        flag_set(parent, CMARK_NODE_TABLE_VISITED);
        return parent;
    }

    // Step 3: Parse delimiter row to extract column count and alignments
    unsigned char* delim_input = input + parser->first_nonspace;
    const int delim_len = len - parser->first_nonspace;
    std::optional<TableRow> delim_row_opt = row_from_string(self, parser, delim_input, delim_len);
    if (!delim_row_opt || delim_row_opt->cells.empty())
    {
        flag_set(parent, CMARK_NODE_TABLE_VISITED);
        return parent;
    }

    // Step 4: Get header row from paragraph's string_content
    const char* raw_content = cmark_node_get_string_content(parent);
    if (raw_content == nullptr || *raw_content == '\0')
    {
        flag_set(parent, CMARK_NODE_TABLE_VISITED);
        return parent;
    }

    // Make a mutable copy for scanning
    std::string header_str(raw_content);
    std::vector<unsigned char> header_buf(header_str.begin(), header_str.end());
    header_buf.push_back('\0');  // Extra null for safety
    std::optional<TableRow> header_row_opt =
        row_from_string(self, parser, header_buf.data(), static_cast<int>(header_str.size()));
    if (!header_row_opt || header_row_opt->cells.empty())
    {
        flag_set(parent, CMARK_NODE_TABLE_VISITED);
        return parent;
    }

    // Step 5: Column counts must match
    const int n_cols = static_cast<int>(delim_row_opt->cells.size());
    if (n_cols != static_cast<int>(header_row_opt->cells.size()))
    {
        flag_set(parent, CMARK_NODE_TABLE_VISITED);
        return parent;
    }

    // Step 6: Transform parent from PARAGRAPH to TABLE
    if (cmark_node_set_type(parent, CMARK_NODE_TABLE) == 0)
    {
        flag_set(parent, CMARK_NODE_TABLE_VISITED);
        return parent;
    }

    // Step 7: If paragraph_offset > 0, insert a PARAGRAPH before the table
    // (Not needed in common cases; paragraph_offset is 0 in our implementation)

    // Step 8: Set syntax extension on table node
    cmark_node_set_syntax_extension(parent, self);

    // Step 9: Allocate NodeTable with column info and alignments
    NodeTable* node_table = new NodeTable {};
    node_table->n_columns = static_cast<uint16_t>(n_cols);
    node_table->alignments.resize(static_cast<size_t>(n_cols), 0);
    for (int col = 0; col < n_cols; ++col)
    {
        node_table->alignments[static_cast<size_t>(col)] =
            get_alignment(delim_row_opt->cells[static_cast<size_t>(col)].content);
    }
    parent->as.opaque = node_table;

    // Step 10: Create TABLE_ROW child (header row)
    cmark_node* header_row = cmark_node_new_with_ext(CMARK_NODE_TABLE_ROW, self);
    if (header_row == nullptr)
    {
        delete node_table;
        parent->as.opaque = nullptr;
        return nullptr;
    }
    NodeTableRow* row_data = static_cast<NodeTableRow*>(header_row->as.opaque);
    if (row_data == nullptr)
    {
        cmark_node_free(header_row);
        delete node_table;
        parent->as.opaque = nullptr;
        return nullptr;
    }
    row_data->is_header = true;

    // Step 11: Create TABLE_CELL children for each header cell
    for (int col = 0; col < n_cols; ++col)
    {
        cmark_node* cell = cmark_node_new_with_ext(CMARK_NODE_TABLE_CELL, self);
        if (cell == nullptr)
        {
            continue;
        }
        cell->as.cell_index = col;
        const std::string& cell_content = header_row_opt->cells[static_cast<size_t>(col)].content;
        cmark_node_set_string_content(cell, cell_content.c_str());
        cmark_node_set_syntax_extension(cell, self);
        cmark_node_append_child(header_row, cell);
        ++node_table->n_nonempty_cells;
    }

    cmark_node_append_child(parent, header_row);

    // Step 12: Increment row count
    incr_table_row_count(parent);

    // Step 13: Advance parser offset to end of line
    cmark_parser_advance_offset(parser, static_cast<const char*>(static_cast<void*>(input)),
                                len - 1 - parser->offset, 0);

    return parent;
}

// ---------------------------------------------------------------------------
// try_opening_table_row
// ---------------------------------------------------------------------------

static cmark_node* try_opening_table_row(cmark_syntax_extension* self, cmark_parser* parser,
                                         cmark_node* parent, unsigned char* input, int len)
{
    // Step 1: Blank line -> table ends
    if (parser->blank)
    {
        return nullptr;
    }

    NodeTable* node_table = static_cast<NodeTable*>(parent->as.opaque);
    if (node_table == nullptr)
    {
        return nullptr;
    }

    // Step 2: Check autocompleted cell limit
    if (node_table->n_rows > 0 &&
        node_table->n_nonempty_cells / node_table->n_rows * node_table->n_columns >
            MAX_AUTOCOMPLETED_CELLS)
    {
        return nullptr;
    }

    // Step 3: Add TABLE_ROW as child of TABLE via parser
    cmark_node* row_node =
        cmark_parser_add_child(parser, parent, CMARK_NODE_TABLE_ROW, parser->first_nonspace + 1);
    if (row_node == nullptr)
    {
        return nullptr;
    }

    cmark_node_set_syntax_extension(row_node, self);
    NodeTableRow* row_data = new NodeTableRow {};
    row_data->is_header = false;
    row_node->as.opaque = row_data;

    // Step 4: Parse row content
    unsigned char* row_input = input + parser->first_nonspace;
    const int row_len = len - parser->first_nonspace;
    std::optional<TableRow> row_opt = row_from_string(self, parser, row_input, row_len);
    if (!row_opt || row_opt->cells.empty())
    {
        // Free dangling node
        cmark_node_unlink(row_node);
        cmark_node_free(row_node);
        return nullptr;
    }

    const int n_cols = static_cast<int>(node_table->n_columns);
    const int n_parsed = static_cast<int>(row_opt->cells.size());

    // Step 5: Create TABLE_CELL children for parsed cells (up to n_columns)
    for (int col = 0; col < std::min(n_cols, n_parsed); ++col)
    {
        cmark_node* cell = cmark_node_new_with_ext(CMARK_NODE_TABLE_CELL, self);
        if (cell == nullptr)
        {
            continue;
        }
        cell->as.cell_index = col;
        cmark_node_set_string_content(cell,
                                      row_opt->cells[static_cast<size_t>(col)].content.c_str());
        cmark_node_set_syntax_extension(cell, self);
        cmark_node_append_child(row_node, cell);
        ++node_table->n_nonempty_cells;
    }

    // Step 6: Pad with empty cells for remaining columns
    for (int col = n_parsed; col < n_cols; ++col)
    {
        cmark_node* cell = cmark_node_new_with_ext(CMARK_NODE_TABLE_CELL, self);
        if (cell == nullptr)
        {
            continue;
        }
        cell->as.cell_index = col;
        cmark_node_set_string_content(cell, "");
        cmark_node_set_syntax_extension(cell, self);
        cmark_node_append_child(row_node, cell);
    }

    // Step 7: Increment row count
    incr_table_row_count(parent);

    // Step 8: Advance parser to end of line
    cmark_parser_advance_offset(parser, static_cast<const char*>(static_cast<void*>(input)),
                                len - 1 - parser->offset, 0);

    return row_node;
}

// ---------------------------------------------------------------------------
// Hook functions
// ---------------------------------------------------------------------------

static cmark_node* try_opening_table_block(cmark_syntax_extension* self, int indented,
                                           cmark_parser* parser, cmark_node* parent,
                                           unsigned char* input, int len)
{
    if (indented != 0)
    {
        return nullptr;
    }

    const cmark_node_type parent_type = static_cast<cmark_node_type>(parent->type);

    if (parent_type == CMARK_NODE_PARAGRAPH)
    {
        return try_opening_table_header(self, parser, parent, input, len);
    }
    if (parent_type == CMARK_NODE_TABLE)
    {
        return try_opening_table_row(self, parser, parent, input, len);
    }
    return nullptr;
}

static int matches(cmark_syntax_extension* self, cmark_parser* parser, unsigned char* input,
                   int len, cmark_node* parent)
{
    if (static_cast<cmark_node_type>(parent->type) != CMARK_NODE_TABLE)
    {
        return 0;
    }

    unsigned char* row_input = input + parser->first_nonspace;
    const int row_len = len - parser->first_nonspace;
    std::optional<TableRow> row_opt = row_from_string(self, parser, row_input, row_len);
    if (!row_opt || row_opt->cells.empty())
    {
        return 0;
    }
    return 1;
}

static const char* get_type_string(cmark_syntax_extension* /*self*/, cmark_node* node)
{
    const cmark_node_type node_type = static_cast<cmark_node_type>(node->type);
    if (node_type == CMARK_NODE_TABLE)
    {
        return "table";
    }
    if (node_type == CMARK_NODE_TABLE_ROW)
    {
        if (node->as.opaque != nullptr)
        {
            const NodeTableRow* row_data = static_cast<const NodeTableRow*>(node->as.opaque);
            return row_data->is_header ? "table_header" : "table_row";
        }
        return "table_row";
    }
    if (node_type == CMARK_NODE_TABLE_CELL)
    {
        return "table_cell";
    }
    return "<unknown>";
}

static int can_contain(cmark_syntax_extension* /*self*/, cmark_node* node,
                       cmark_node_type child_type)
{
    const cmark_node_type node_type = static_cast<cmark_node_type>(node->type);
    if (node_type == CMARK_NODE_TABLE)
    {
        return child_type == CMARK_NODE_TABLE_ROW ? 1 : 0;
    }
    if (node_type == CMARK_NODE_TABLE_ROW)
    {
        return child_type == CMARK_NODE_TABLE_CELL ? 1 : 0;
    }
    if (node_type == CMARK_NODE_TABLE_CELL)
    {
        return (child_type == CMARK_NODE_TEXT || child_type == CMARK_NODE_CODE ||
                child_type == CMARK_NODE_EMPH || child_type == CMARK_NODE_STRONG ||
                child_type == CMARK_NODE_LINK || child_type == CMARK_NODE_IMAGE ||
                child_type == CMARK_NODE_HTML_INLINE ||
                child_type == CMARK_NODE_FOOTNOTE_REFERENCE) ?
                   1 :
                   0;
    }
    return 0;
}

static int contains_inlines(cmark_syntax_extension* /*self*/, cmark_node* node)
{
    return static_cast<cmark_node_type>(node->type) == CMARK_NODE_TABLE_CELL ? 1 : 0;
}

static void opaque_alloc(cmark_syntax_extension* /*self*/, cmark_node* node)
{
    const cmark_node_type node_type = static_cast<cmark_node_type>(node->type);
    if (node_type == CMARK_NODE_TABLE)
    {
        node->as.opaque = new NodeTable {};
    }
    else if (node_type == CMARK_NODE_TABLE_ROW)
    {
        node->as.opaque = new NodeTableRow {};
    }
}

static void opaque_free(cmark_syntax_extension* /*self*/, cmark_node* node)
{
    const cmark_node_type node_type = static_cast<cmark_node_type>(node->type);
    if (node_type == CMARK_NODE_TABLE && node->as.opaque != nullptr)
    {
        delete static_cast<NodeTable*>(node->as.opaque);
        node->as.opaque = nullptr;
    }
    else if (node_type == CMARK_NODE_TABLE_ROW && node->as.opaque != nullptr)
    {
        delete static_cast<NodeTableRow*>(node->as.opaque);
        node->as.opaque = nullptr;
    }
}

// ---------------------------------------------------------------------------
// HTML renderer
// ---------------------------------------------------------------------------

static void html_render(cmark_syntax_extension* /*self*/, cmark_html_renderer* renderer,
                        cmark_node* node, cmark_event_type ev_type, int options)
{
    cmark_strbuf* html = renderer->html;
    const cmark_node_type node_type = static_cast<cmark_node_type>(node->type);
    const bool entering = (ev_type == CMARK_EVENT_ENTER);

    if (node_type == CMARK_NODE_TABLE)
    {
        if (entering)
        {
            // Allocate HTML state for this table
            renderer->opaque = new HtmlTableState {};
            cmark_html_render_cr(html);
            html->Puts("<table>\n");
        }
        else
        {
            const HtmlTableState* state = static_cast<HtmlTableState*>(renderer->opaque);
            if (state != nullptr && state->need_closing_tbody)
            {
                html->Puts("</tbody>\n");
            }
            html->Puts("</table>\n");
            delete state;
            renderer->opaque = nullptr;
        }
    }
    else if (node_type == CMARK_NODE_TABLE_ROW)
    {
        HtmlTableState* state = static_cast<HtmlTableState*>(renderer->opaque);
        const bool is_header =
            (node->as.opaque != nullptr) && static_cast<NodeTableRow*>(node->as.opaque)->is_header;

        if (entering)
        {
            if (is_header)
            {
                html->Puts("<thead>\n<tr>\n");
                if (state != nullptr)
                {
                    state->in_thead = true;
                }
            }
            else
            {
                if (state != nullptr && !state->need_closing_tbody)
                {
                    html->Puts("<tbody>\n");
                    state->need_closing_tbody = true;
                }
                html->Puts("<tr>\n");
                if (state != nullptr)
                {
                    state->in_thead = false;
                }
            }
        }
        else
        {
            html->Puts("</tr>\n");
            if (is_header)
            {
                html->Puts("</thead>\n");
            }
        }
    }
    else if (node_type == CMARK_NODE_TABLE_CELL)
    {
        const HtmlTableState* state = static_cast<HtmlTableState*>(renderer->opaque);
        const bool in_thead = (state != nullptr) && state->in_thead;
        const char* tag = in_thead ? "th" : "td";

        if (entering)
        {
            // Determine alignment
            const cmark_node* row = node->parent;
            const cmark_node* table = (row != nullptr) ? row->parent : nullptr;
            uint8_t alignment = 0;
            if (table != nullptr && table->as.opaque != nullptr)
            {
                const NodeTable* node_table = static_cast<const NodeTable*>(table->as.opaque);
                const int col_idx = node->as.cell_index;
                if (col_idx >= 0 && static_cast<size_t>(col_idx) < node_table->alignments.size())
                {
                    alignment = node_table->alignments[static_cast<size_t>(col_idx)];
                }
            }

            uint8_t effective_alignment = alignment;
            if (in_thead && effective_alignment == 0U)
            {
                effective_alignment = static_cast<uint8_t>('l');
            }

            if ((options & CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES) != 0)
            {
                html->Puts("<");
                html->Puts(tag);
                if (effective_alignment == static_cast<uint8_t>('l'))
                {
                    html->Puts(" style=\"text-align: left\"");
                }
                else if (effective_alignment == static_cast<uint8_t>('r'))
                {
                    html->Puts(" style=\"text-align: right\"");
                }
                else if (effective_alignment == static_cast<uint8_t>('c'))
                {
                    html->Puts(" style=\"text-align: center\"");
                }
                html->Puts(">");
            }
            else
            {
                html->Puts("<");
                html->Puts(tag);
                if (effective_alignment == static_cast<uint8_t>('l'))
                {
                    html->Puts(" align=\"left\"");
                }
                else if (effective_alignment == static_cast<uint8_t>('r'))
                {
                    html->Puts(" align=\"right\"");
                }
                else if (effective_alignment == static_cast<uint8_t>('c'))
                {
                    html->Puts(" align=\"center\"");
                }
                html->Puts(">");
            }
        }
        else
        {
            html->Puts("</");
            html->Puts(tag);
            html->Puts(">\n");
        }
    }
}

// ---------------------------------------------------------------------------
// create_table_extension
// ---------------------------------------------------------------------------

cmark_syntax_extension* create_table_extension()
{
    static bool is_initialized = false;

    cmark_syntax_extension* table_ext = cmark_syntax_extension_new("table");
    if (table_ext == nullptr)
    {
        return nullptr;
    }

    if (!is_initialized)
    {
        cmark_register_node_flag(&CMARK_NODE_TABLE_VISITED);

        CMARK_NODE_TABLE = cmark_syntax_extension_add_node(0);
        CMARK_NODE_TABLE_ROW = cmark_syntax_extension_add_node(0);
        CMARK_NODE_TABLE_CELL = cmark_syntax_extension_add_node(0);
        is_initialized = true;
    }

    cmark_syntax_extension_set_match_block_func(table_ext, matches);
    cmark_syntax_extension_set_open_block_func(table_ext, try_opening_table_block);
    cmark_syntax_extension_set_get_type_string_func(table_ext, get_type_string);
    cmark_syntax_extension_set_can_contain_func(table_ext, can_contain);
    cmark_syntax_extension_set_contains_inlines_func(table_ext, contains_inlines);
    cmark_syntax_extension_set_html_render_func(table_ext, html_render);
    cmark_syntax_extension_set_opaque_alloc_func(table_ext, opaque_alloc);
    cmark_syntax_extension_set_opaque_free_func(table_ext, opaque_free);

    return table_ext;
}
