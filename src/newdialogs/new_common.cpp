/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains code common between all new_ dialogs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "new_common.h"

#include "mainapp.h"          // App -- Main application class
#include "project_handler.h"  // ProjectHandler class

void UpdateFormClass(Node* form_node)
{
    bool is_base_class = false;
    ttString baseName = form_node->prop_as_wxString(prop_class_name);
    if (baseName.Right(4) == "Base")
    {
        baseName.Replace("Base", wxEmptyString);
        is_base_class = true;
    }
    baseName.MakeLower();
    baseName << "_base";
    if (Project.HasValue(prop_base_directory))
        baseName.insert(0, Project.as_ttString(prop_base_directory) << '/');

    form_node->prop_set_value(prop_base_file, baseName);
    if (is_base_class)
    {
        form_node->prop_set_value(prop_base_file, baseName);

        wxString class_name = form_node->prop_as_wxString(prop_class_name);
        if (class_name.Right(4) == "Base")
        {
            class_name.Replace("Base", wxEmptyString);
        }
        else
        {
            class_name << "Derived";
        }
        form_node->prop_set_value(prop_derived_class_name, class_name);

        ttString drvName = form_node->prop_as_wxString(prop_derived_class_name);
        if (drvName.Right(7) == "Derived")
            drvName.Replace("Derived", "_derived");
        else if (!is_base_class)
        {
            drvName << "_derived";
        }

        drvName.MakeLower();
        if (Project.HasValue(prop_derived_class_name))
        {
            drvName.insert(0, Project.as_ttString(prop_base_directory) << '/');
        }

        form_node->prop_set_value(prop_derived_file, drvName);
    }
}

bool IsClassNameUnique(wxString classname)
{
    auto new_classname = classname.utf8_string();

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& iter: forms)
    {
        if (iter->prop_as_string(prop_class_name).is_sameas(new_classname))
        {
            return false;
        }
    }

    return true;
}
