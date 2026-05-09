// Syntax extension creation and manipulation
// Purpose: Creates cmark_syntax_extension objects, sets hook function pointers
// Key functions: cmark_syntax_extension_new(), various cmark_syntax_extension_set_*() setters
// Status: Phase 1.5 modernization - converted to std::unique_ptr
// Dependencies: cmark-gfm.hxx, syntax_extension.hxx

#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>

#include "cmark-gfm.hxx"

#include "syntax_extension.hxx"

void cmark_syntax_extension_free(cmark_syntax_extension* extension)
{
    if (extension->free_function && extension->priv)
    {
        extension->free_function(extension->priv);
    }

    cmark_llist_free(extension->special_inline_chars);
    delete[] extension->name;
    delete extension;
}

cmark_syntax_extension* cmark_syntax_extension_new(const char* name)
{
    std::unique_ptr<cmark_syntax_extension> extension = std::make_unique<cmark_syntax_extension>();

    const std::size_t name_length = std::strlen(name) + 1;
    std::unique_ptr<char[]> name_storage = std::make_unique<char[]>(name_length);
    std::strcpy(name_storage.get(), name);
    extension->name = name_storage.release();

    return extension.release();
}

cmark_node_type cmark_syntax_extension_add_node(int is_inline)
{
    cmark_node_type* ref = !is_inline ? &CMARK_NODE_LAST_BLOCK : &CMARK_NODE_LAST_INLINE;

    if ((*ref & CMARK_NODE_VALUE_MASK) == CMARK_NODE_VALUE_MASK)
    {
        assert(false);
        return static_cast<cmark_node_type>(0);
    }

    return *ref = static_cast<cmark_node_type>(static_cast<int>(*ref) + 1);
}

void cmark_syntax_extension_set_emphasis(cmark_syntax_extension* extension, int emphasis)
{
    extension->emphasis = emphasis == 1;
}

void cmark_syntax_extension_set_open_block_func(cmark_syntax_extension* extension,
                                                cmark_open_block_func func)
{
    extension->try_opening_block = func;
}

void cmark_syntax_extension_set_match_block_func(cmark_syntax_extension* extension,
                                                 cmark_match_block_func func)
{
    extension->last_block_matches = func;
}

void cmark_syntax_extension_set_match_inline_func(cmark_syntax_extension* extension,
                                                  cmark_match_inline_func func)
{
    extension->match_inline = func;
}

void cmark_syntax_extension_set_inline_from_delim_func(cmark_syntax_extension* extension,
                                                       cmark_inline_from_delim_func func)
{
    extension->insert_inline_from_delim = func;
}

void cmark_syntax_extension_set_special_inline_chars(cmark_syntax_extension* extension,
                                                     cmark_llist* special_chars)
{
    extension->special_inline_chars = special_chars;
}

void cmark_syntax_extension_set_get_type_string_func(cmark_syntax_extension* extension,
                                                     cmark_get_type_string_func func)
{
    extension->get_type_string_func = func;
}

void cmark_syntax_extension_set_can_contain_func(cmark_syntax_extension* extension,
                                                 cmark_can_contain_func func)
{
    extension->can_contain_func = func;
}

void cmark_syntax_extension_set_contains_inlines_func(cmark_syntax_extension* extension,
                                                      cmark_contains_inlines_func func)
{
    extension->contains_inlines_func = func;
}

void cmark_syntax_extension_set_commonmark_render_func(cmark_syntax_extension* extension,
                                                       cmark_common_render_func func)
{
    extension->commonmark_render_func = func;
}

void cmark_syntax_extension_set_plaintext_render_func(cmark_syntax_extension* extension,
                                                      cmark_common_render_func func)
{
    extension->plaintext_render_func = func;
}

void cmark_syntax_extension_set_latex_render_func(cmark_syntax_extension* extension,
                                                  cmark_common_render_func func)
{
    extension->latex_render_func = func;
}

void cmark_syntax_extension_set_xml_attr_func(cmark_syntax_extension* extension,
                                              cmark_xml_attr_func func)
{
    extension->xml_attr_func = func;
}

void cmark_syntax_extension_set_man_render_func(cmark_syntax_extension* extension,
                                                cmark_common_render_func func)
{
    extension->man_render_func = func;
}

void cmark_syntax_extension_set_html_render_func(cmark_syntax_extension* extension,
                                                 cmark_html_render_func func)
{
    extension->html_render_func = func;
}

void cmark_syntax_extension_set_html_filter_func(cmark_syntax_extension* extension,
                                                 cmark_html_filter_func func)
{
    extension->html_filter_func = func;
}

void cmark_syntax_extension_set_postprocess_func(cmark_syntax_extension* extension,
                                                 cmark_postprocess_func func)
{
    extension->postprocess_func = func;
}

void cmark_syntax_extension_set_private(cmark_syntax_extension* extension, void* priv,
                                        cmark_free_func free_func)
{
    extension->priv = priv;
    extension->free_function = free_func;
}

void* cmark_syntax_extension_get_private(cmark_syntax_extension* extension)
{
    return extension->priv;
}

void cmark_syntax_extension_set_opaque_alloc_func(cmark_syntax_extension* extension,
                                                  cmark_opaque_alloc_func func)
{
    extension->opaque_alloc_func = func;
}

void cmark_syntax_extension_set_opaque_free_func(cmark_syntax_extension* extension,
                                                 cmark_opaque_free_func func)
{
    extension->opaque_free_func = func;
}

void cmark_syntax_extension_set_commonmark_escape_func(cmark_syntax_extension* extension,
                                                       cmark_commonmark_escape_func func)
{
    extension->commonmark_escape_func = func;
}
