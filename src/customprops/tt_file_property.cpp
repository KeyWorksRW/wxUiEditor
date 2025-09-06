/////////////////////////////////////////////////////////////////////////////
// Purpose:   Version of wxFileProperty specific to wxUiEditor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filedlg.h>
#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "node_prop.h"        // for wxNodeProperty
#include "project_handler.h"  // ProjectHandler class

#include "tt_file_property.h"

wxPG_IMPLEMENT_PROPERTY_CLASS(ttFileProperty, wxEditorDialogProperty, TextCtrlAndButton);

ttFileProperty::ttFileProperty(NodeProperty* prop) :
    wxEditorDialogProperty(prop->get_DeclName().make_wxString(), wxPG_LABEL)
{
    m_prop = prop;
    SetValue(prop->value().make_wxString());
}

ttFileProperty::ttFileProperty(const wxString& label, const wxString& name, const wxString& value) :
    wxEditorDialogProperty(label, name)
{
    ASSERT_MSG(false, "This constructor should not be called -- m_prop will not be set!");
    SetValue(value);
}

bool ttFileProperty::DisplayEditorDialog(wxPropertyGrid* pg, wxVariant& value)
{
    wxFileName root_path;
    wxString wildcard;
    wxString title;
    auto* form = m_prop->getNode()->get_Form();
    auto* folder =
        form ? form->get_Folder() : static_cast<Node*>(nullptr);  // this will usually be a nullptr

    switch (m_prop->get_name())
    {
        case prop_base_file:
            if (folder && folder->HasValue(prop_folder_base_directory))
                root_path.AssignDir(folder->as_string(prop_folder_base_directory));
            else if (Project.get_ProjectNode()->HasValue(prop_base_directory))
                root_path.AssignDir(Project.get_ProjectNode()->as_string(prop_base_directory));
            else
                root_path.AssignDir(Project.get_wxFileName()->GetPath());
            title = "Base class filename";
            wildcard = "C++ Files|*.cpp;*.cc;*.cxx";
            break;

        case prop_derived_file:
            if (folder && folder->HasValue(prop_folder_derived_directory))
                root_path.AssignDir(folder->as_string(prop_folder_derived_directory));
            else if (Project.get_ProjectNode()->HasValue(prop_derived_directory))
                root_path.AssignDir(Project.get_ProjectNode()->as_string(prop_derived_directory));
            else
                root_path.AssignDir(Project.get_wxFileName()->GetPath());
            title = "Derived class filename";
            wildcard = "C++ Files|*.cpp;*.cc;*.cxx";
            break;

        case prop_perl_file:
            if (folder && folder->HasValue(prop_folder_perl_output_folder))
                root_path.AssignDir(folder->as_string(prop_folder_perl_output_folder));
            else if (Project.get_ProjectNode()->HasValue(prop_perl_output_folder))
                root_path.AssignDir(Project.get_ProjectNode()->as_string(prop_perl_output_folder));
            else
                root_path.AssignDir(Project.get_wxFileName()->GetPath());
            title = "Perl filename";
            wildcard = "Perl Files|*.pl;*.pm";
            break;

        case prop_python_file:
        case prop_python_combined_file:
            if (folder && folder->HasValue(prop_folder_python_output_folder))
                root_path.AssignDir(folder->as_string(prop_folder_python_output_folder));
            else if (Project.get_ProjectNode()->HasValue(prop_python_output_folder))
                root_path.AssignDir(
                    Project.get_ProjectNode()->as_string(prop_python_output_folder));
            else
                root_path.AssignDir(Project.get_wxFileName()->GetPath());
            title = "Python filename";
            wildcard = "Python Files|*.py";
            break;

        case prop_ruby_file:
        case prop_ruby_combined_file:
            if (folder && folder->HasValue(prop_folder_ruby_output_folder))
                root_path.AssignDir(folder->as_string(prop_folder_ruby_output_folder));
            else if (Project.get_ProjectNode()->HasValue(prop_ruby_output_folder))
                root_path.AssignDir(Project.get_ProjectNode()->as_string(prop_ruby_output_folder));
            else
                root_path.AssignDir(Project.get_wxFileName()->GetPath());
            title = "Ruby filename";
            wildcard = "Ruby Files|*.rb;*.rbw";
            break;

        case prop_rust_file:
            if (folder && folder->HasValue(prop_folder_rust_output_folder))
                root_path.AssignDir(folder->as_string(prop_folder_rust_output_folder));
            else if (Project.get_ProjectNode()->HasValue(prop_rust_output_folder))
                root_path.AssignDir(Project.get_ProjectNode()->as_string(prop_rust_output_folder));
            else
                root_path.AssignDir(Project.get_wxFileName()->GetPath());
            title = "Rust filename";
            wildcard = "Rust Files|*.rust";
            break;

        case prop_xrc_file:
        case prop_combined_xrc_file:
        case prop_folder_combined_xrc_file:
            if (folder && folder->HasValue(prop_folder_xrc_directory))
                root_path.AssignDir(folder->as_string(prop_folder_xrc_directory));
            else if (Project.get_ProjectNode()->HasValue(prop_xrc_directory))
                root_path.AssignDir(Project.get_ProjectNode()->as_string(prop_xrc_directory));
            else
                root_path.AssignDir(Project.get_wxFileName()->GetPath());
            title = "XRC filename";
            wildcard = "XRC Files|*.xrc";
            break;

        case prop_cmake_file:
        case prop_folder_cmake_file:
            root_path = Project.get_ProjectPath();
            title = "CMake filename";
            wildcard = "CMake Files|*.cmake";
            break;

        // Currently this is for a wxFrame window
        case prop_subclass_header:
            root_path = Project.get_ProjectPath();
            title = "Subclass Header";
            wildcard = "Header Files|*.h;*.hh;*.hpp;*.hxx";
            break;

        case prop_output_file:
            root_path = Project.get_ProjectPath();
            title = "Data output filename";
            wildcard = "C++ Files|*.cpp;*.cc;*.cxx";
            break;

        case prop_data_file:
            if (m_prop->as_string().size())
            {
                root_path.Assign(m_prop->as_string());
            }
            else
            {
                auto result =
                    Project.GetOutputPath(m_prop->getNode()->get_Form(), GEN_LANG_CPLUSPLUS);
                if (!result.second)
                    root_path.AssignDir(result.first);
                else
                    root_path.Assign(result.first);
            }
            if (m_prop->getNode()->is_Gen(gen_data_xml))
            {
                title = "XML file";
                wildcard = "XML/XRC Files|*.xml;*.xrc";
            }
            else
            {
                title = "Data file";
                wildcard = "Files|*.*";
            }
            break;

        case prop_initial_filename:
            root_path = Project.get_ProjectPath();
            title = "Initial filename";
            wildcard = "All Files|*.*";
            break;

        default:
            FAIL_MSG(tt_string() << "Unknown property type: " << m_prop->get_DeclName().substr());
            break;

    }  // switch (m_prop->get_name())

    wxFileName full_path;
    full_path.AssignDir(root_path.GetPath());
    auto cur_path = value.GetString();
    if (cur_path.starts_with("./"))
    {
        full_path.Assign(cur_path);
    }
    full_path.MakeAbsolute();
    wxFileDialog dlg(pg->GetPanel(), title, full_path.GetPath(), full_path.GetFullName(), wildcard,
                     wxFD_SAVE);
    if (dlg.ShowModal() == wxID_OK)
    {
        full_path.Assign(dlg.GetPath());
        full_path.MakeRelativeTo(Project.get_wxFileName()->GetPath());
        tt_string final_path = full_path.GetFullPath().utf8_string();
        final_path.backslashestoforward();
        if (!final_path.contains("/"))
            final_path = "./" + full_path.GetFullPath().utf8_string();
        value = final_path.make_wxString();
        return true;
    }
    return false;
}

wxString ttFileProperty::ValueToString(wxVariant& value,
                                       wxPGPropValFormatFlags /* flags unused */) const
{
    // auto result = value.GetString();
    return value.GetString();
}

bool ttFileProperty::StringToValue(wxVariant& variant, const wxString& text,
                                   wxPGPropValFormatFlags /* flags unused */) const
{
    if (auto filename = variant.GetString(); filename != text)
    {
        variant = text;
        return true;
    }

    return false;
}

void ttFileProperty::OnSetValue() {}

bool ttFileProperty::DoSetAttribute(const wxString& name, wxVariant& value)
{
    if (name == wxPG_FILE_INITIAL_PATH || name == wxPG_FILE_SHOW_RELATIVE_PATH)
    {
        return true;
    }
    return wxEditorDialogProperty::DoSetAttribute(name, value);
}

wxValidator* ttFileProperty::GetClassValidator()
{
    static wxValidator* pValidator = nullptr;
    if (pValidator)
        return pValidator;

    pValidator = new wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST);

    wxStaticCast(pValidator, wxTextValidator)->SetCharExcludes(wxString("?*|<>\""));

    wxPGGlobalVars->m_arrValidators.push_back(pValidator);
    return pValidator;
}

wxValidator* ttFileProperty::DoGetValidator() const
{
    return GetClassValidator();
}
