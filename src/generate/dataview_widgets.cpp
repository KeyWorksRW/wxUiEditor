/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDataView component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
    wxDataViewItem GetParent(const wxDataViewItem&) const override { return wxDataViewItem(nullptr); }
    bool IsContainer(const wxDataViewItem&) const override { return false; }
    void GetValue(wxVariant&, const wxDataViewItem&, unsigned int /*col*/) const override {}
    bool SetValue(const wxVariant&, const wxDataViewItem&, unsigned int /*col*/) override { return true; }
};

wxObject* DataViewCtrl::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxDataViewCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                     DlgSize(parent, node, prop_size), GetStyleInt(node));

    wxObjectDataPtr<DataViewModel> model;
    model = new DataViewModel;
    widget->AssociateModel(model.get());

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void DataViewCtrl::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* node, bool /* is_preview */)
{
    auto list = wxStaticCast(wxobject, wxDataViewCtrl);

    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->isGen(gen_dataViewColumn))
        {
            if (childObj->as_string(prop_type) == "Text")
            {
                auto* col = list->AppendTextColumn(
                    childObj->prop_as_wxString(prop_label), childObj->prop_as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Toggle")
            {
                auto* col = list->AppendToggleColumn(
                    childObj->prop_as_wxString(prop_label), childObj->prop_as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Progress")
            {
                auto* col = list->AppendProgressColumn(
                    childObj->prop_as_wxString(prop_label), childObj->prop_as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "IconText")
            {
                auto* col = list->AppendIconTextColumn(
                    childObj->prop_as_wxString(prop_label), childObj->prop_as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Date")
            {
                auto* col = list->AppendDateColumn(
                    childObj->prop_as_wxString(prop_label), childObj->prop_as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Bitmap")
            {
                auto* col = list->AppendBitmapColumn(
                    childObj->prop_as_wxString(prop_label), childObj->prop_as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
        }
    }
}

bool DataViewCtrl::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().Add(prop_id);
    code.PosSizeFlags(true);

    return true;
}

bool DataViewCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_dataview.cpp
// ../../../wxWidgets/src/xrc/xh_dataview.cpp
// See HandleCtrl()

int DataViewCtrl::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxDataViewCtrl");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void DataViewCtrl::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxDataViewXmlHandler");
}

//////////////////////////////////////////  DataViewListCtrl  //////////////////////////////////////////

wxObject* DataViewListCtrl::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxDataViewListCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                         DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void DataViewListCtrl::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* node, bool /* is_preview */)
{
    auto list = wxStaticCast(wxobject, wxDataViewListCtrl);

    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->isGen(gen_dataViewListColumn))
        {
            if (childObj->as_string(prop_type) == "Text")
            {
                auto col = list->AppendTextColumn(
                    childObj->prop_as_wxString(prop_label),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Toggle")
            {
                auto col = list->AppendToggleColumn(
                    childObj->prop_as_wxString(prop_label),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Progress")
            {
                auto col = list->AppendProgressColumn(
                    childObj->prop_as_wxString(prop_label),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "IconText")
            {
                auto col = list->AppendIconTextColumn(
                    childObj->prop_as_wxString(prop_label),
                    static_cast<wxDataViewCellMode>(childObj->prop_as_int(prop_mode)), childObj->prop_as_int(prop_width),
                    static_cast<wxAlignment>(childObj->prop_as_int(prop_align)), childObj->prop_as_int(prop_flags));
                if (childObj->HasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->prop_as_int(prop_ellipsize)));
                }
            }
        }
    }
}

bool DataViewListCtrl::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().Add(prop_id);
    code.PosSizeFlags(true);

    return true;
}

bool DataViewListCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_dataview.cpp
// ../../../wxWidgets/src/xrc/xh_dataview.cpp
// See HandleListCtrl()

int DataViewListCtrl::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxDataViewListCtrl");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void DataViewListCtrl::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxDataViewXmlHandler");
}

//////////////////////////////////////////  DataViewTreeCtrl  //////////////////////////////////////////

wxObject* DataViewTreeCtrl::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxDataViewTreeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                         DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool DataViewTreeCtrl::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().Add(prop_id);
    code.PosSizeFlags(true);

    return true;
}

bool DataViewTreeCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_dataview.cpp
// ../../../wxWidgets/src/xrc/xh_dataview.cpp
// See HandleTreeCtrl()

int DataViewTreeCtrl::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxDataViewListCtrl");

    // TODO: [KeyWorks - 06-06-2022] XRC supports "imagelist"

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void DataViewTreeCtrl::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxDataViewXmlHandler");
}

//////////////////////////////////////////  DataViewColumn  //////////////////////////////////////////

bool DataViewColumn::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().Str(" = ").ParentName().Function("Append").Str(prop_type).Str("Column(");
    code.QuotedString(prop_label).Comma().Str(prop_model_column).Comma().Str(prop_mode).Comma().Str(prop_width);
    if (code.is_cpp())
        code.Str("static_cast<wxAlignment>(");
    code.Add(prop_align);
    if (code.is_cpp())
        code << ')';
    code.Comma().Add(prop_flags).EndFunction();

    if (code.HasValue(prop_ellipsize))
    {
        code.Eol().NodeName().Function("GetRenderer()").Function("EnableEllipsize(").Add(prop_ellipsize).EndFunction();
    }

    return true;
}

//////////////////////////////////////////  DataViewListColumn  //////////////////////////////////////////

bool DataViewListColumn::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().Str(" = ").ParentName().Function("Append").Str(prop_type).Str("Column(");
    code.QuotedString(prop_label).Comma().Str(prop_mode).Comma().Str(prop_width);
    if (code.is_cpp())
        code.Str("static_cast<wxAlignment>(");
    code.Add(prop_align);
    if (code.is_cpp())
        code << ')';
    code.Comma().Add(prop_flags).EndFunction();

    if (code.HasValue(prop_ellipsize))
    {
        code.Eol().NodeName().Function("GetRenderer()").Function("EnableEllipsize(").Add(prop_ellipsize).EndFunction();
    }

    return true;
}
