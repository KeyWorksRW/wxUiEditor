// Global syntax extension registry
// Purpose: Maintains list of available extensions (tables, strikethrough, etc.)
// Key functions: cmark_list_syntax_extensions()
// Status: Legacy code - uses manual malloc and static global list (Phase 1.5 modernization
// candidate) Dependencies: cmark-gfm.hxx, syntax_extension.hxx, registry.hxx

#include <cstring>

#include "cmark-gfm.hxx"
#include "syntax_extension.hxx"

#include "registry.hxx"

static cmark_llist* syntax_extensions = nullptr;

void cmark_release_plugins(void)
{
    if (syntax_extensions)
    {
        cmark_llist_free_full(syntax_extensions, (cmark_free_func) cmark_syntax_extension_free);
        syntax_extensions = nullptr;
    }
}

cmark_llist* cmark_list_syntax_extensions()
{
    cmark_llist* iter = nullptr;
    cmark_llist* res = nullptr;

    for (iter = syntax_extensions; iter; iter = iter->next)
    {
        res = cmark_llist_append(res, iter->data);
    }
    return res;
}

cmark_syntax_extension* cmark_find_syntax_extension(const char* name)
{
    cmark_llist* tmp = nullptr;

    for (tmp = syntax_extensions; tmp; tmp = tmp->next)
    {
        cmark_syntax_extension* ext = static_cast<cmark_syntax_extension*>(tmp->data);
        if (!strcmp(ext->name, name))
        {
            return ext;
        }
    }
    return nullptr;
}
