/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for single line code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "code_single_prop.h"

#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "../nodes/node.h"       // Node class
#include "../nodes/node_prop.h"  // NodeProperty class

#include "wxui/editstringdialog_base.h"  // auto-generated: wxui/editstringdialog_base.h wxui/editstringdialog_base.cpp

EditCodeSingleProperty::EditCodeSingleProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

class EditCodeSingleDialog : public EditStringDialogBase
{
public:
    EditCodeSingleDialog(wxWindow* parent, NodeProperty* prop) :
        EditStringDialogBase(parent),
        m_node(prop->getNode()),
        m_prop(prop)
    {
        SetTitle((wxue::string() << prop->get_DeclName() << " property editor").wx());
        m_value = prop->as_wxString();
        m_static_hdr_text->Show();

        m_textCtrl->Bind(wxEVT_TEXT, &EditCodeSingleDialog::UpdateStaticText, this);
        Fit();
    };

    void UpdateStaticText(wxCommandEvent& /* event */)
    {
        wxString static_text;
        if (m_prop->isProp(prop_cpp_conditional))
        {
            const wxString val = m_textCtrl->GetValue();
            if (!val.StartsWith("#"))
            {
                static_text << "#if ";
            }
            static_text << val;
        }
        else
        {
            if (m_node->is_PropValue(prop_class_access, "none"))
            {
                static_text << "auto ";
            }
            static_text << m_node->as_string(prop_var_name) << " = new "
                        << m_node->as_string(prop_class_name);
            static_text << m_textCtrl->GetValue() << ';';
        }
        m_static_hdr_text->SetLabel(static_text);
    }

private:
    Node* m_node;
    NodeProperty* m_prop;
};

bool EditCodeSingleDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid,
                                               wxPGProperty* /* property unused */)
{
    EditCodeSingleDialog code_dialog(propGrid->GetPanel(), m_prop);
    if (code_dialog.ShowModal() == wxID_OK)
    {
        SetValue(code_dialog.GetResults());
        return true;
    }

    return false;
}
