/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for custom control parameters
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "custom_param_prop.h"

#include "../nodes/node.h"       // Node class
#include "../nodes/node_prop.h"  // NodeProperty class

#include "wxui/editstringdialog_base.h"  // auto-generated: wxui/editstringdialog_base.h wxui/editstringdialog_base.cpp

EditParamProperty::EditParamProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

class EditParamDialog : public EditStringDialogBase
{
public:
    EditParamDialog(wxWindow* parent, NodeProperty* prop) : EditStringDialogBase(parent)
    {
        SetTitle(ttlib::cstr() << prop->DeclName() << " property editor");
        m_value = prop->as_wxString();
        m_static_hdr_text->Show();
        m_node = prop->GetNode();

        m_textCtrl->Bind(wxEVT_TEXT, &EditParamDialog::UpdateStaticText, this);
        Fit();
    };

    void UpdateStaticText(wxCommandEvent& /* event */)
    {
        ttlib::cstr ctor;
        if (m_node->isPropValue(prop_class_access, "none"))
            ctor << "auto ";
        ctor << m_node->prop_as_string(prop_var_name) << " = new " << m_node->prop_as_string(prop_class_name);
        ctor << m_textCtrl->GetValue().utf8_string() << ';';
        m_static_hdr_text->SetLabel(ctor.wx_str());
    }

private:
    Node* m_node;
};

bool EditParamDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    EditParamDialog dlg(propGrid->GetPanel(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
