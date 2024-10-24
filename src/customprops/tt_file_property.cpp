/////////////////////////////////////////////////////////////////////////////
// Purpose:   Version of wxFileProperty specific to wxUiEditor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filedlg.h>

#include "node_prop.h"        // for wxNodeProperty
#include "project_handler.h"  // ProjectHandler class

#include "tt_file_property.h"

wxPG_IMPLEMENT_PROPERTY_CLASS(ttFileProperty, wxEditorDialogProperty, TextCtrlAndButton);

ttFileProperty::ttFileProperty(NodeProperty* prop) : wxEditorDialogProperty(prop->declName().make_wxString(), wxPG_LABEL)
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
    tt_string root_path;
    wxString wildcard;
    wxString title;
    auto* form = m_prop->getNode()->getForm();
    auto* folder = form ? form->getFolder() : static_cast<Node*>(nullptr);  // this will usually be a nullptr

    switch (m_prop->get_name())
    {
        case prop_base_file:
            if (folder && folder->hasValue(prop_folder_base_directory))
                root_path = folder->as_string(prop_folder_base_directory);
            else if (Project.getProjectNode()->hasValue(prop_base_directory))
                root_path = Project.getProjectNode()->as_string(prop_base_directory);
            else
                root_path = Project.getProjectPath();
            title = "Base class filename";
            wildcard = "C++ Files|*.cpp;*.cc;*.cxx";
            break;

        case prop_derived_file:

            if (folder && folder->hasValue(prop_folder_derived_directory))
                root_path = folder->as_string(prop_folder_derived_directory);
            else if (Project.getProjectNode()->hasValue(prop_derived_directory))
                root_path = Project.getProjectNode()->as_string(prop_derived_directory);
            else
                root_path = Project.getProjectPath();
            title = "Derived class filename";
            wildcard = "C++ Files|*.cpp;*.cc;*.cxx";
            break;

        case prop_xrc_file:
        case prop_combined_xrc_file:
        case prop_folder_combined_xrc_file:
            if (folder && folder->hasValue(prop_folder_xrc_directory))
                root_path = folder->as_string(prop_folder_xrc_directory);
            else if (Project.getProjectNode()->hasValue(prop_xrc_directory))
                root_path = Project.getProjectNode()->as_string(prop_xrc_directory);
            else
                root_path = Project.getProjectPath();
            title = "XRC filename";
            wildcard = "XRC Files|*.xrc";
            break;

        case prop_python_file:
        case prop_python_combined_file:
            if (folder && folder->hasValue(prop_folder_python_output_folder))
                root_path = folder->as_string(prop_folder_python_output_folder);
            else if (Project.getProjectNode()->hasValue(prop_python_output_folder))
                root_path = Project.getProjectNode()->as_string(prop_python_output_folder);
            else
                root_path = Project.getProjectPath();
            title = "Python filename";
            wildcard = "Python Files|*.py";
            break;

        case prop_ruby_file:
        case prop_ruby_combined_file:
            if (folder && folder->hasValue(prop_folder_ruby_output_folder))
                root_path = folder->as_string(prop_folder_ruby_output_folder);
            else if (Project.getProjectNode()->hasValue(prop_ruby_output_folder))
                root_path = Project.getProjectNode()->as_string(prop_ruby_output_folder);
            else
                root_path = Project.getProjectPath();
            title = "Ruby filename";
            wildcard = "Ruby Files|*.rb;*.rbw";
            break;

        case prop_haskell_file:
            if (folder && folder->hasValue(prop_folder_haskell_output_folder))
                root_path = folder->as_string(prop_folder_haskell_output_folder);
            else if (Project.getProjectNode()->hasValue(prop_haskell_output_folder))
                root_path = Project.getProjectNode()->as_string(prop_haskell_output_folder);
            else
                root_path = Project.getProjectPath();
            title = "Haskell filename";
            wildcard = "Haskell Files|*.hs;*.lhs";
            break;

        case prop_lua_file:
            if (folder && folder->hasValue(prop_folder_lua_output_folder))
                root_path = folder->as_string(prop_folder_lua_output_folder);
            else if (Project.getProjectNode()->hasValue(prop_lua_output_folder))
                root_path = Project.getProjectNode()->as_string(prop_lua_output_folder);
            else
                root_path = Project.getProjectPath();
            title = "Lua filename";
            wildcard = "Lua Files|*.lua";
            break;

        case prop_perl_file:
            if (folder && folder->hasValue(prop_folder_perl_output_folder))
                root_path = folder->as_string(prop_folder_perl_output_folder);
            else if (Project.getProjectNode()->hasValue(prop_perl_output_folder))
                root_path = Project.getProjectNode()->as_string(prop_perl_output_folder);
            else
                root_path = Project.getProjectPath();
            title = "Perl filename";
            wildcard = "Perl Files|*.pl;*.pm";
            break;

        case prop_rust_file:
            if (folder && folder->hasValue(prop_folder_rust_output_folder))
                root_path = folder->as_string(prop_folder_rust_output_folder);
            else if (Project.getProjectNode()->hasValue(prop_rust_output_folder))
                root_path = Project.getProjectNode()->as_string(prop_rust_output_folder);
            else
                root_path = Project.getProjectPath();
            title = "Rust filename";
            wildcard = "Rust Files|*.rust";
            break;

        case prop_cmake_file:
        case prop_folder_cmake_file:
            root_path = Project.getProjectPath();
            title = "CMake filename";
            wildcard = "CMake Files|*.cmake";
            break;

        // Currently this is for a wxFrame window
        case prop_subclass_header:
            root_path = Project.getProjectPath();
            title = "Subclass Header";
            wildcard = "Header Files|*.h;*.hh;*.hpp;*.hxx";
            break;

        case prop_output_file:
            root_path = Project.getProjectPath();
            title = "Data output filename";
            wildcard = "C++ Files|*.cpp;*.cc;*.cxx";
            break;

        case prop_data_file:
            if (m_prop->as_string().size())
            {
                root_path = m_prop->as_string();
                root_path.remove_filename();
            }
            else
            {
                auto result = Project.GetOutputPath(m_prop->getNode()->getForm(), GEN_LANG_CPLUSPLUS);
                root_path = result.first;
                if (result.second)  // true if the the base filename was returned
                    root_path.remove_filename();
            }
            if (m_prop->getNode()->isGen(gen_data_xml))
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

        default:
            FAIL_MSG(tt_string() << "Unknown property type: " << m_prop->declName().substr());
            break;

    }  // switch (m_prop->get_name())

    tt_string full_path = root_path;
    auto cur_path = value.GetString().utf8_string();
    if (cur_path.starts_with("./"))
    {
        full_path = cur_path;
    }
    else
    {
        full_path.append_filename(cur_path);
    }
    full_path.make_absolute();
    wxString filename = full_path.filename().make_wxString();
    full_path.remove_filename();
    wxFileDialog dlg(pg->GetPanel(), title, full_path.make_wxString(), filename, wildcard, wxFD_SAVE);
    if (dlg.ShowModal() == wxID_OK)
    {
        full_path = dlg.GetPath().utf8_string();
        // Note that no matter whether the filename is in one of the base or output directories, we
        // always make it relative to the project path.
        full_path.make_relative(Project.getProjectPath());
        full_path.backslashestoforward();
        if (!full_path.contains("/"))
            full_path = "./" + full_path;
        value = full_path.make_wxString();
        return true;
    }
    return false;
}

wxString ttFileProperty::ValueToString(wxVariant& value, wxPGPropValFormatFlags WXUNUSED(flags)) const
{
    auto result = value.GetString();
    return value.GetString();
}

bool ttFileProperty::StringToValue(wxVariant& variant, const wxString& text, wxPGPropValFormatFlags WXUNUSED(flags)) const
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
