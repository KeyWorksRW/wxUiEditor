// Purpose: Defines global node type bounds and markdown-to-HTML conversion with table support
// Status: Table extension wired up
// Dependencies: cmark-gfm.hxx, table.hxx

#include <string>
#include <string_view>

#include "cmark-gfm.hxx"
#include "table.hxx"

cmark_node_type CMARK_NODE_LAST_BLOCK = CMARK_NODE_FOOTNOTE_DEFINITION;
cmark_node_type CMARK_NODE_LAST_INLINE = CMARK_NODE_FOOTNOTE_REFERENCE;

std::string cmark_markdown_to_html(std::string_view text, int options)
{
    cmark_parser* parser = cmark_parser_new(options);

    cmark_syntax_extension* table_ext = create_table_extension();
    if (table_ext != nullptr)
    {
        cmark_parser_attach_syntax_extension(parser, table_ext);
    }

    cmark_parser_feed(parser, text.data(), text.size());
    cmark_node* document = cmark_parser_finish(parser);
    cmark_parser_free(parser);

    cmark_llist* extensions = nullptr;
    if (table_ext != nullptr)
    {
        extensions = cmark_llist_append(nullptr, table_ext);
    }
    const std::string result = cmark_render_html(document, options, extensions, text.size());
    cmark_llist_free(extensions);

    cmark_node_free(document);
    if (table_ext != nullptr)
    {
        cmark_syntax_extension_free(table_ext);
    }
    return result;
}
