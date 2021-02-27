/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDataView component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/dataview.h>  // wxDataViewCtrl base classes
#include <wx/event.h>     // Event classes

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "dataview_widgets.h"

//////////////////////////////////////////  DataViewCtrl  //////////////////////////////////////////

class DataViewModel : public wxDataViewModel
{
public:
    unsigned int GetChildren(const wxDataViewItem&, wxDataViewItemArray& /*children*/) const override { return 0; }
    unsigned int GetColumnCount() const override { return 0; }
    wxString GetColumnType(unsigned int /*col*/) const override { return wxVariant("Dummy").GetType(); }
    wxDataViewItem GetParent(const wxDataViewItem&) const override { return wxDataViewItem(NULL); }
    bool IsContainer(const wxDataViewItem&) const override { return false; }
    void GetValue(wxVariant&, const wxDataViewItem&, unsigned int /*col*/) const override {}
    bool SetValue(const wxVariant&, const wxDataViewItem&, unsigned int /*col*/) override { return true; }
};

wxObject* DataViewCtrl::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxDataViewCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                           node->prop_as_wxSize("size"), node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    wxObjectDataPtr<DataViewModel> model;
    model = new DataViewModel;
    widget->AssociateModel(model.get());

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void DataViewCtrl::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */)
{
    auto list = wxStaticCast(wxobject, wxDataViewCtrl);

    auto node = GetMockup()->GetNode(wxobject);
    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->GetClassName() == "dataViewColumn")
        {
            if (childObj->GetPropertyAsString("type") == "Text")
            {
                auto* col = list->AppendTextColumn(
                    childObj->GetPropertyAsString(txtLabel), childObj->prop_as_int("model_column"),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")), childObj->prop_as_int(txtWidth),
                    static_cast<wxAlignment>(childObj->prop_as_int("align")), childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
            else if (childObj->GetPropertyAsString("type") == "Toggle")
            {
                auto* col = list->AppendToggleColumn(
                    childObj->GetPropertyAsString(txtLabel), childObj->prop_as_int("model_column"),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")), childObj->prop_as_int(txtWidth),
                    static_cast<wxAlignment>(childObj->prop_as_int("align")), childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
            else if (childObj->GetPropertyAsString("type") == "Progress")
            {
                auto* col = list->AppendProgressColumn(
                    childObj->GetPropertyAsString(txtLabel), childObj->prop_as_int("model_column"),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")), childObj->prop_as_int(txtWidth),
                    static_cast<wxAlignment>(childObj->prop_as_int("align")), childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
            else if (childObj->GetPropertyAsString("type") == "IconText")
            {
                auto* col = list->AppendIconTextColumn(
                    childObj->GetPropertyAsString(txtLabel), childObj->prop_as_int("model_column"),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")), childObj->prop_as_int(txtWidth),
                    static_cast<wxAlignment>(childObj->prop_as_int("align")), childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
            else if (childObj->GetPropertyAsString("type") == "Date")
            {
                auto* col = list->AppendDateColumn(
                    childObj->GetPropertyAsString(txtLabel), childObj->prop_as_int("model_column"),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")), childObj->prop_as_int(txtWidth),
                    static_cast<wxAlignment>(childObj->prop_as_int("align")), childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
            else if (childObj->GetPropertyAsString("type") == "Bitmap")
            {
                auto* col = list->AppendBitmapColumn(
                    childObj->GetPropertyAsString(txtLabel), childObj->prop_as_int("model_column"),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")), childObj->prop_as_int(txtWidth),
                    static_cast<wxAlignment>(childObj->prop_as_int("align")), childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
        }
    }
}

std::optional<ttlib::cstr> DataViewCtrl::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxDataViewCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, true);

    return code;
}

std::optional<ttlib::cstr> DataViewCtrl::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool DataViewCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  DataViewListCtrl  //////////////////////////////////////////

wxObject* DataViewListCtrl::Create(Node* node, wxObject* parent)
{
    auto widget = new wxDataViewListCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                                         node->prop_as_wxSize("size"),
                                         node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void DataViewListCtrl::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */)
{
    auto list = wxStaticCast(wxobject, wxDataViewListCtrl);

    auto node = GetMockup()->GetNode(wxobject);
    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->GetClassName() == "dataViewListColumn")
        {
            if (childObj->GetPropertyAsString("type") == "Text")
            {
                auto col = list->AppendTextColumn(
                    childObj->GetPropertyAsString(txtLabel), static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")),
                    childObj->prop_as_int(txtWidth), static_cast<wxAlignment>(childObj->prop_as_int("align")),
                    childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
            else if (childObj->GetPropertyAsString("type") == "Toggle")
            {
                auto col = list->AppendToggleColumn(
                    childObj->GetPropertyAsString(txtLabel), static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")),
                    childObj->prop_as_int(txtWidth), static_cast<wxAlignment>(childObj->prop_as_int("align")),
                    childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
            else if (childObj->GetPropertyAsString("type") == "Progress")
            {
                auto col = list->AppendProgressColumn(
                    childObj->GetPropertyAsString(txtLabel), static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")),
                    childObj->prop_as_int(txtWidth), static_cast<wxAlignment>(childObj->prop_as_int("align")),
                    childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
            else if (childObj->GetPropertyAsString("type") == "IconText")
            {
                auto col = list->AppendIconTextColumn(
                    childObj->GetPropertyAsString(txtLabel), static_cast<wxDataViewCellMode>(childObj->prop_as_int("mode")),
                    childObj->prop_as_int(txtWidth), static_cast<wxAlignment>(childObj->prop_as_int("align")),
                    childObj->prop_as_int(txtFlags));
                if (childObj->HasValue("ellipsize"))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int("ellipsize")));
                }
            }
        }
    }
}

std::optional<ttlib::cstr> DataViewListCtrl::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxDataViewListCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, true);

    return code;
}

std::optional<ttlib::cstr> DataViewListCtrl::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool DataViewListCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  DataViewTreeCtrl  //////////////////////////////////////////

wxObject* DataViewTreeCtrl::Create(Node* node, wxObject* parent)
{
    auto widget = new wxDataViewTreeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                                         node->prop_as_wxSize("size"),
                                         node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> DataViewTreeCtrl::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxDataViewTreeCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, true);

    return code;
}

std::optional<ttlib::cstr> DataViewTreeCtrl::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool DataViewTreeCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  DataViewColumn  //////////////////////////////////////////

std::optional<ttlib::cstr> DataViewColumn::GenConstruction(Node* node)
{
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = " << node->get_parent_name() << "->Append";
    code << node->prop_as_string("type") << "Column(" << GenerateQuotedString(node->prop_as_string(txtLabel))
         << ",\n            ";

    code << node->prop_as_string("model_column") << ", " << node->prop_as_string("mode") << ", ";
    code << node->prop_as_string(txtWidth) << ", ";

    // BUGBUG: [KeyWorks - 12-14-2020] Currently the user is allowed to combine multiple alignment types such as right and
    // left which is invalid.

    code << "static_cast<wxAlignment>(" << node->prop_as_string("align") << "), ";
    code << node->prop_as_string(txtFlags) << ");";

    if (node->HasValue("ellipsize"))
    {
        code << "\n    " << node->get_node_name() << "->GetRenderer()->EnableEllipsize(" << node->prop_as_string("ellipsize")
             << ");";
    }

    return code;
}

//////////////////////////////////////////  DataViewListColumn  //////////////////////////////////////////

std::optional<ttlib::cstr> DataViewListColumn::GenConstruction(Node* node)
{
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = " << node->get_parent_name() << "->Append";
    code << node->prop_as_string("type") << "Column(" << GenerateQuotedString(node->prop_as_string(txtLabel))
         << ",\n            ";
    code << node->prop_as_string("mode") << ", " << node->prop_as_string(txtWidth) << ", ";

    // BUGBUG: [KeyWorks - 12-14-2020] Currently the user is allowed to combine multiple alignment types such as right and
    // left which is invalid.

    code << "static_cast<wxAlignment>(" << node->prop_as_string("align") << "), ";
    code << node->prop_as_string(txtFlags) << ");";

    if (node->HasValue("ellipsize"))
    {
        code << "\n    " << node->get_node_name() << "->GetRenderer()->EnableEllipsize(" << node->prop_as_string("ellipsize")
             << ");";
    }

    return code;
}
