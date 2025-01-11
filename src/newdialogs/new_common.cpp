/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains code common between all new_ dialogs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "new_common.h"

#include "mainapp.h"          // App -- Main application class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

void UpdateFormClass(Node* form_node)
{
    bool is_base_class = form_node->as_string(prop_class_name).ends_with("Base");
    auto filename = CreateBaseFilename(form_node, form_node->as_string(prop_class_name));
    form_node->set_value(prop_base_file, filename);

    if (Project.getCodePreference() == GEN_LANG_PERL)
    {
        form_node->set_value(prop_perl_file, filename);
    }
    else if (Project.getCodePreference() == GEN_LANG_PYTHON)
    {
        form_node->set_value(prop_python_file, filename);
    }
    else if (Project.getCodePreference() == GEN_LANG_RUBY)
    {
        form_node->set_value(prop_ruby_file, filename);
    }
    else if (Project.getCodePreference() == GEN_LANG_RUST)
    {
        form_node->set_value(prop_rust_file, filename);
    }
    else if (Project.getCodePreference() == GEN_LANG_XRC)
    {
        form_node->set_value(prop_xrc_file, filename);
    }

#if GENERATE_NEW_LANG_CODE
    else if (Project.getCodePreference() == GEN_LANG_FORTRAN)
    {
        form_node->set_value(prop_fortran_file, filename);
    }
    else if (Project.getCodePreference() == GEN_LANG_HASKELL)
    {
        form_node->set_value(prop_haskell_file, filename);
    }
    else if (Project.getCodePreference() == GEN_LANG_LUA)
    {
        form_node->set_value(prop_lua_file, filename);
    }
#endif  // GENERATE_NEW_LANG_CODE

    if (is_base_class)
    {
        auto class_name = form_node->as_string(prop_class_name);
        if (class_name.ends_with("Base"))
        {
            class_name.erase(class_name.size() - (sizeof("Base") - 1));
        }
        else
        {
            class_name += "Derived";
        }
        form_node->set_value(prop_derived_class_name, class_name);

        filename = CreateDerivedFilename(form_node, class_name);
        form_node->set_value(prop_derived_file, filename);
    }
}

bool IsClassNameUnique(wxString classname)
{
    auto new_classname = classname.utf8_string();

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& iter: forms)
    {
        if (iter->as_string(prop_class_name).is_sameas(new_classname))
        {
            return false;
        }
    }

    return true;
}
