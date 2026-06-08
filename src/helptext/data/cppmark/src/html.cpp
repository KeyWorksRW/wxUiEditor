// Purpose: Converts markdown AST to HTML output
// Key functions: cmark_render_html(), cmark_render_html_with_mem() (entry points)
// Render flow: Walk AST with iterator -> call S_render_node() for each ENTER/EXIT event -> build
// HTML buffer Status: Legacy code - uses manual malloc (Phase 1.5 modernization candidate)
// Dependencies: All rendering-related headers, houdini for escaping

#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <tuple>

#include "cmark-gfm.hxx"
#include "cmark_ctype.hxx"
#include "houdini.hxx"
#include "html.hxx"
#include "node.hxx"
#include "render.hxx"
#include "scanners.hxx"
#include "syntax_extension.hxx"

// Functions to convert cmark_nodes to HTML strings.

void cmark_html_render_cr(CMarkStringBuffer* html)
{
    if (html->get_size() && html->get_ptr()[html->get_size() - 1] != '\n')
    {
        html->Putc('\n');
    }
}

constexpr int BUFFER_SIZE = 100;

static void cmark_html_render_sourcepos(cmark_node* node, CMarkStringBuffer* html, int options)
{
    if ((options & CMARK_OPT_SOURCEPOS) != 0)
    {
        std::array<char, BUFFER_SIZE> buffer {};
        std::snprintf(buffer.data(), buffer.size(), " data-sourcepos=\"%d:%d-%d:%d\"",
                      cmark_node_get_start_line(node), cmark_node_get_start_column(node),
                      cmark_node_get_end_line(node), cmark_node_get_end_column(node));
        html->Puts(buffer.data());
    }
}

static void escape_html(CMarkStringBuffer* dest, const unsigned char* source, size_t length)
{
    houdini_escape_html0(dest, source, length, 0);
}

static void filter_html_block(cmark_html_renderer* renderer, uint8_t* data, size_t len)
{
    CMarkStringBuffer* html = renderer->html;
    const cmark_llist* extension_item = nullptr;
    cmark_syntax_extension* extension = nullptr;
    bool filtered = false;
    uint8_t* match = nullptr;

    while (len)
    {
        match = static_cast<uint8_t*>(memchr(data, '<', len));
        if (!match)
        {
            break;
        }

        if (match != data)
        {
            html->Put(data, static_cast<size_t>(match - data));
            len -= static_cast<size_t>(match - data);
            data = match;
        }

        filtered = false;
        for (extension_item = renderer->filter_extensions; extension_item;
             extension_item = extension_item->next)
        {
            extension = static_cast<cmark_syntax_extension*>(extension_item->data);
            if (!extension->html_filter_func(extension, data, len))
            {
                filtered = true;
                break;
            }
        }

        if (!filtered)
        {
            html->Putc('<');
        }
        else
        {
            html->Puts("&lt;");
        }

        ++data;
        --len;
    }

    if (len)
    {
        html->Put(data, (size_t) len);
    }
}

static bool S_put_footnote_backref(cmark_html_renderer* renderer, CMarkStringBuffer* html,
                                   cmark_node* node)
{
    if (renderer->written_footnote_ix >= renderer->footnote_ix)
    {
        return false;
    }
    renderer->written_footnote_ix = renderer->footnote_ix;
    char backref_index_text[32] {};
    snprintf(backref_index_text, sizeof(backref_index_text), "%d", renderer->written_footnote_ix);

    html->Puts("<a href=\"#fnref-");
    houdini_escape_href(html, node->as.literal.data, node->as.literal.len);

    html->Puts("\" class=\"footnote-backref\" data-footnote-backref data-footnote-backref-idx=\"");
    html->Puts(backref_index_text);
    html->Puts("\" aria-label=\"Back to reference ");
    html->Puts(backref_index_text);
    html->Puts("\">↩</a>");

    if (node->footnote.def_count > 1)
    {
        for (int i = 2; i <= node->footnote.def_count; i++)
        {
            char definition_index_text[32] {};
            snprintf(definition_index_text, sizeof(definition_index_text), "%d", i);

            html->Puts(" <a href=\"#fnref-");
            houdini_escape_href(html, node->as.literal.data, node->as.literal.len);
            html->Puts("-");
            html->Puts(definition_index_text);

            html->Puts(
                "\" class=\"footnote-backref\" data-footnote-backref data-footnote-backref-idx=\"");
            html->Puts(backref_index_text);
            html->Puts("-");
            html->Puts(definition_index_text);
            html->Puts("\" aria-label=\"Back to reference ");
            html->Puts(backref_index_text);
            html->Puts("-");
            html->Puts(definition_index_text);
            html->Puts("\">↩<sup class=\"footnote-ref\">");
            html->Puts(definition_index_text);
            html->Puts("</sup></a>");
        }
    }

    return true;
}

static int S_render_node(cmark_html_renderer* renderer, cmark_node* node, cmark_event_type ev_type,
                         int options)
{
    cmark_node* parent = nullptr;
    const cmark_node* grandparent = nullptr;
    CMarkStringBuffer* html = renderer->html;
    const cmark_llist* it = nullptr;
    cmark_syntax_extension* syntax_extension = nullptr;
    char start_heading[] = "<h0";
    char end_heading[] = "</h0";
    bool tight = false;
    bool filtered = false;
    char buffer[BUFFER_SIZE] {};

    const bool entering = (ev_type == CMARK_EVENT_ENTER);

    if (renderer->plain == node)
    {  // back at original node
        renderer->plain = nullptr;
    }

    if (renderer->plain != nullptr)
    {
        switch (node->type)
        {
            case CMARK_NODE_TEXT:
            case CMARK_NODE_CODE:
            case CMARK_NODE_HTML_INLINE:
                escape_html(html, node->as.literal.data, node->as.literal.len);
                break;

            case CMARK_NODE_LINEBREAK:
            case CMARK_NODE_SOFTBREAK:
                html->Putc(' ');
                break;

            default:
                break;
        }
        return 1;
    }

    if (node->extension && node->extension->html_render_func)
    {
        node->extension->html_render_func(node->extension, renderer, node, ev_type, options);
        return 1;
    }

    switch (node->type)
    {
        case CMARK_NODE_DOCUMENT:
            break;

        case CMARK_NODE_BLOCK_QUOTE:
            if (entering)
            {
                cmark_html_render_cr(html);
                html->Puts("<blockquote");
                cmark_html_render_sourcepos(node, html, options);
                html->Puts(">\n");
            }
            else
            {
                cmark_html_render_cr(html);
                html->Puts("</blockquote>\n");
            }
            break;

        case CMARK_NODE_LIST:
            {
                const cmark_list_type list_type = node->as.list.list_type;
                const int start = node->as.list.start;

                if (entering)
                {
                    cmark_html_render_cr(html);
                    if (list_type == CMARK_BULLET_LIST)
                    {
                        html->Puts("<ul");
                        cmark_html_render_sourcepos(node, html, options);
                        html->Puts(">\n");
                    }
                    else if (start == 1)
                    {
                        html->Puts("<ol");
                        cmark_html_render_sourcepos(node, html, options);
                        html->Puts(">\n");
                    }
                    else
                    {
                        snprintf(buffer, BUFFER_SIZE, "<ol start=\"%d\"", start);
                        html->Puts(buffer);
                        cmark_html_render_sourcepos(node, html, options);
                        html->Puts(">\n");
                    }
                }
                else
                {
                    html->Puts(list_type == CMARK_BULLET_LIST ? "</ul>\n" : "</ol>\n");
                }
                break;
            }

        case CMARK_NODE_ITEM:
            if (entering)
            {
                cmark_html_render_cr(html);
                html->Puts("<li");
                cmark_html_render_sourcepos(node, html, options);
                html->Putc('>');
            }
            else
            {
                html->Puts("</li>\n");
            }
            break;

        case CMARK_NODE_HEADING:
            if (entering)
            {
                cmark_html_render_cr(html);
                start_heading[2] = static_cast<char>('0' + node->as.heading.level);
                html->Puts(start_heading);
                cmark_html_render_sourcepos(node, html, options);
                html->Putc('>');
            }
            else
            {
                end_heading[3] = static_cast<char>('0' + node->as.heading.level);
                html->Puts(end_heading);
                html->Puts(">\n");
            }
            break;

        case CMARK_NODE_CODE_BLOCK:
            cmark_html_render_cr(html);

            if (node->as.code.info.len == 0)
            {
                html->Puts("<pre");
                cmark_html_render_sourcepos(node, html, options);
                html->Puts("><code>");
            }
            else
            {
                size_t first_tag = 0;
                while (first_tag < node->as.code.info.len &&
                       !cmark_isspace(node->as.code.info.data[first_tag]))
                {
                    ++first_tag;
                }

                if (options & CMARK_OPT_GITHUB_PRE_LANG)
                {
                    html->Puts("<pre");
                    cmark_html_render_sourcepos(node, html, options);
                    html->Puts(" lang=\"");
                    escape_html(html, node->as.code.info.data, first_tag);
                    if (first_tag < node->as.code.info.len &&
                        (options & CMARK_OPT_FULL_INFO_STRING))
                    {
                        html->Puts("\" data-meta=\"");
                        escape_html(html, node->as.code.info.data + first_tag + 1,
                                    node->as.code.info.len - first_tag - 1);
                    }
                    html->Puts("\"><code>");
                }
                else
                {
                    html->Puts("<pre");
                    cmark_html_render_sourcepos(node, html, options);
                    html->Puts("><code class=\"language-");
                    escape_html(html, node->as.code.info.data, first_tag);
                    if (first_tag < node->as.code.info.len &&
                        (options & CMARK_OPT_FULL_INFO_STRING))
                    {
                        html->Puts("\" data-meta=\"");
                        escape_html(html, node->as.code.info.data + first_tag + 1,
                                    node->as.code.info.len - first_tag - 1);
                    }
                    html->Puts("\">");
                }
            }

            escape_html(html, node->as.code.literal.data, node->as.code.literal.len);
            html->Puts("</code></pre>\n");
            break;

        case CMARK_NODE_HTML_BLOCK:
            cmark_html_render_cr(html);
            if (!(options & CMARK_OPT_UNSAFE))
            {
                html->Puts("<!-- raw HTML omitted -->");
            }
            else if (renderer->filter_extensions)
            {
                filter_html_block(renderer, node->as.literal.data, node->as.literal.len);
            }
            else
            {
                html->Put(node->as.literal.data, node->as.literal.len);
            }
            cmark_html_render_cr(html);
            break;

        case CMARK_NODE_CUSTOM_BLOCK:
            cmark_html_render_cr(html);
            if (entering)
            {
                html->Put(node->as.custom.on_enter.data, node->as.custom.on_enter.len);
            }
            else
            {
                html->Put(node->as.custom.on_exit.data, node->as.custom.on_exit.len);
            }
            cmark_html_render_cr(html);
            break;

        case CMARK_NODE_THEMATIC_BREAK:
            cmark_html_render_cr(html);
            html->Puts("<hr");
            cmark_html_render_sourcepos(node, html, options);
            html->Puts(" />\n");
            break;

        case CMARK_NODE_PARAGRAPH:
            parent = cmark_node_parent(node);
            grandparent = cmark_node_parent(parent);
            if (grandparent != nullptr && grandparent->type == CMARK_NODE_LIST)
            {
                tight = grandparent->as.list.tight;
            }
            else
            {
                tight = false;
            }
            if (!tight)
            {
                if (entering)
                {
                    cmark_html_render_cr(html);
                    html->Puts("<p");
                    cmark_html_render_sourcepos(node, html, options);
                    html->Putc('>');
                }
                else
                {
                    if (parent->type == CMARK_NODE_FOOTNOTE_DEFINITION && node->next == nullptr)
                    {
                        html->Putc(' ');
                        std::ignore = S_put_footnote_backref(renderer, html, parent);
                    }
                    html->Puts("</p>\n");
                }
            }
            break;

        case CMARK_NODE_TEXT:
            escape_html(html, node->as.literal.data, node->as.literal.len);
            break;

        case CMARK_NODE_LINEBREAK:
            html->Puts("<br />\n");
            break;

        case CMARK_NODE_SOFTBREAK:
            if (options & CMARK_OPT_HARDBREAKS)
            {
                html->Puts("<br />\n");
            }
            else if (options & CMARK_OPT_NOBREAKS)
            {
                html->Putc(' ');
            }
            else
            {
                html->Putc('\n');
            }
            break;

        case CMARK_NODE_CODE:
            html->Puts("<code>");
            escape_html(html, node->as.literal.data, node->as.literal.len);
            html->Puts("</code>");
            break;

        case CMARK_NODE_HTML_INLINE:
            if (!(options & CMARK_OPT_UNSAFE))
            {
                html->Puts("<!-- raw HTML omitted -->");
            }
            else
            {
                filtered = false;
                for (it = renderer->filter_extensions; it; it = it->next)
                {
                    syntax_extension = static_cast<cmark_syntax_extension*>(it->data);
                    if (!syntax_extension->html_filter_func(syntax_extension, node->as.literal.data,
                                                            node->as.literal.len))
                    {
                        filtered = true;
                        break;
                    }
                }
                if (!filtered)
                {
                    html->Put(node->as.literal.data, node->as.literal.len);
                }
                else
                {
                    html->Puts("&lt;");
                    html->Put(node->as.literal.data + 1, node->as.literal.len - 1);
                }
            }
            break;

        case CMARK_NODE_CUSTOM_INLINE:
            if (entering)
            {
                html->Put(node->as.custom.on_enter.data, node->as.custom.on_enter.len);
            }
            else
            {
                html->Put(node->as.custom.on_exit.data, node->as.custom.on_exit.len);
            }
            break;

        case CMARK_NODE_STRONG:
            if (node->parent == nullptr || node->parent->type != CMARK_NODE_STRONG)
            {
                if (entering)
                {
                    html->Puts("<strong>");
                }
                else
                {
                    html->Puts("</strong>");
                }
            }
            break;

        case CMARK_NODE_EMPH:
            if (entering)
            {
                html->Puts("<em>");
            }
            else
            {
                html->Puts("</em>");
            }
            break;

        case CMARK_NODE_LINK:
            if (entering)
            {
                html->Puts("<a href=\"");
                if ((options & CMARK_OPT_UNSAFE) || !(scan_dangerous_url(&node->as.link.url, 0)))
                {
                    houdini_escape_href(html, node->as.link.url.data, node->as.link.url.len);
                }
                if (node->as.link.title.len)
                {
                    html->Puts("\" title=\"");
                    escape_html(html, node->as.link.title.data, node->as.link.title.len);
                }
                html->Puts("\">");
            }
            else
            {
                html->Puts("</a>");
            }
            break;

        case CMARK_NODE_IMAGE:
            if (entering)
            {
                html->Puts("<img src=\"");
                if ((options & CMARK_OPT_UNSAFE) || !(scan_dangerous_url(&node->as.link.url, 0)))
                {
                    houdini_escape_href(html, node->as.link.url.data, node->as.link.url.len);
                }
                html->Puts("\" alt=\"");
                renderer->plain = node;
            }
            else
            {
                if (node->as.link.title.len)
                {
                    html->Puts("\" title=\"");
                    escape_html(html, node->as.link.title.data, node->as.link.title.len);
                }

                html->Puts("\" />");
            }
            break;

        case CMARK_NODE_FOOTNOTE_DEFINITION:
            if (entering)
            {
                if (renderer->footnote_ix == 0)
                {
                    html->Puts("<section class=\"footnotes\" data-footnotes>\n<ol>\n");
                }
                ++renderer->footnote_ix;

                html->Puts("<li id=\"fn-");
                houdini_escape_href(html, node->as.literal.data, node->as.literal.len);
                html->Puts("\">\n");
            }
            else
            {
                if (S_put_footnote_backref(renderer, html, node))
                {
                    html->Putc('\n');
                }
                html->Puts("</li>\n");
            }
            break;

        case CMARK_NODE_FOOTNOTE_REFERENCE:
            if (entering)
            {
                html->Puts("<sup class=\"footnote-ref\"><a href=\"#fn-");
                houdini_escape_href(html, node->parent_footnote_def->as.literal.data,
                                    node->parent_footnote_def->as.literal.len);
                html->Puts("\" id=\"fnref-");
                houdini_escape_href(html, node->parent_footnote_def->as.literal.data,
                                    node->parent_footnote_def->as.literal.len);

                if (node->footnote.ref_ix > 1)
                {
                    char reference_index_text[32] {};
                    snprintf(reference_index_text, sizeof(reference_index_text), "%d",
                             node->footnote.ref_ix);
                    html->Puts("-");
                    html->Puts(reference_index_text);
                }

                html->Puts("\" data-footnote-ref>");
                houdini_escape_href(html, node->as.literal.data, node->as.literal.len);
                html->Puts("</a></sup>");
            }
            break;

        default:
            assert(false);
            break;
    }

    return 1;
}

std::string cmark_render_html(cmark_node* root, int options, cmark_llist* extensions,
                              size_t initial_buffer_size)
{
    CMarkStringBuffer html = CMarkStringBuffer();
    html.Init(initial_buffer_size);
    cmark_event_type ev_type = CMARK_EVENT_DONE;
    cmark_node* cur = nullptr;
    cmark_html_renderer renderer = { .html = &html,
                                     .plain = nullptr,
                                     .filter_extensions = nullptr,
                                     .footnote_ix = 0,
                                     .written_footnote_ix = 0,
                                     .opaque = nullptr };
    cmark_iter* iter = cmark_iter_new(root);

    for (; extensions; extensions = extensions->next)
    {
        if (static_cast<cmark_syntax_extension*>(extensions->data)->html_filter_func)
        {
            renderer.filter_extensions = cmark_llist_append(
                renderer.filter_extensions, static_cast<cmark_syntax_extension*>(extensions->data));
        }
    }

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
    {
        cur = cmark_iter_get_node(iter);
        S_render_node(&renderer, cur, ev_type, options);
    }

    if (renderer.footnote_ix)
    {
        html.Puts("</ol>\n</section>\n");
    }

    cmark_llist_free(renderer.filter_extensions);

    cmark_iter_free(iter);
    return std::string(html.CStr(), html.Len());
}
