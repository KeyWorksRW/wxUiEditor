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

    if (Project.as_string(prop_code_preference) == "Python")
    {
        form_node->set_value(prop_python_file, filename);
    }

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
