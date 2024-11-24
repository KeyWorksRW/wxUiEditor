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
    auto widget = new wxDataViewCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                     DlgSize(node, prop_size), GetStyleInt(node));

    wxObjectDataPtr<DataViewModel> model;
    model = new DataViewModel;
    widget->AssociateModel(model.get());

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void DataViewCtrl::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* node, bool /* is_preview */)
{
    auto list = wxStaticCast(wxobject, wxDataViewCtrl);

    size_t count = node->getChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->getChild(i);
        if (childObj->isGen(gen_dataViewColumn))
        {
            if (childObj->as_string(prop_type) == "Text")
            {
                auto* col = list->AppendTextColumn(
                    childObj->as_wxString(prop_label), childObj->as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)), childObj->as_int(prop_width),
                    static_cast<wxAlignment>(childObj->as_int(prop_align)), childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Toggle")
            {
                auto* col = list->AppendToggleColumn(
                    childObj->as_wxString(prop_label), childObj->as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)), childObj->as_int(prop_width),
                    static_cast<wxAlignment>(childObj->as_int(prop_align)), childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Progress")
            {
                auto* col = list->AppendProgressColumn(
                    childObj->as_wxString(prop_label), childObj->as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)), childObj->as_int(prop_width),
                    static_cast<wxAlignment>(childObj->as_int(prop_align)), childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "IconText")
            {
                auto* col = list->AppendIconTextColumn(
                    childObj->as_wxString(prop_label), childObj->as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)), childObj->as_int(prop_width),
                    static_cast<wxAlignment>(childObj->as_int(prop_align)), childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Date")
            {
                auto* col = list->AppendDateColumn(
                    childObj->as_wxString(prop_label), childObj->as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)), childObj->as_int(prop_width),
                    static_cast<wxAlignment>(childObj->as_int(prop_align)), childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Bitmap")
            {
                auto* col = list->AppendBitmapColumn(
                    childObj->as_wxString(prop_label), childObj->as_int(prop_model_column),
                    static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)), childObj->as_int(prop_width),
                    static_cast<wxAlignment>(childObj->as_int(prop_align)), childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
        }
    }
}

bool DataViewCtrl::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, true);

    return true;
}

bool DataViewCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                               GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_dataview.cpp
// ../../../wxWidgets/src/xrc/xh_dataview.cpp
// See HandleCtrl()

int DataViewCtrl::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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

std::pair<bool, tt_string> DataViewCtrl::isLanguageVersionSupported(GenLang language)
{
    if (language == GEN_LANG_NONE || (language & (GEN_LANG_CPLUSPLUS | GEN_LANG_PYTHON)))
        return { true, {} };

    return { false, tt_string() << "wxDataViewCtrl is not supported by " << ConvertFromGenLang(language) };
}

std::optional<tt_string> DataViewCtrl::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_RUBY:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxRuby currently does not support Wx::DataViewCtrl";
                return msg;
            }
        default:
            return {};
    }
}

//////////////////////////////////////////  DataViewListCtrl  //////////////////////////////////////////

wxObject* DataViewListCtrl::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxDataViewListCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                         DlgSize(node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void DataViewListCtrl::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* node, bool /* is_preview */)
{
    auto list = wxStaticCast(wxobject, wxDataViewListCtrl);

    size_t count = node->getChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->getChild(i);
        if (childObj->isGen(gen_dataViewListColumn))
        {
            if (childObj->as_string(prop_type) == "Text")
            {
                auto col = list->AppendTextColumn(
                    childObj->as_wxString(prop_label), static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)),
                    childObj->as_int(prop_width), static_cast<wxAlignment>(childObj->as_int(prop_align)),
                    childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Toggle")
            {
                auto col = list->AppendToggleColumn(
                    childObj->as_wxString(prop_label), static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)),
                    childObj->as_int(prop_width), static_cast<wxAlignment>(childObj->as_int(prop_align)),
                    childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "Progress")
            {
                auto col = list->AppendProgressColumn(
                    childObj->as_wxString(prop_label), static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)),
                    childObj->as_int(prop_width), static_cast<wxAlignment>(childObj->as_int(prop_align)),
                    childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
            else if (childObj->as_string(prop_type) == "IconText")
            {
                auto col = list->AppendIconTextColumn(
                    childObj->as_wxString(prop_label), static_cast<wxDataViewCellMode>(childObj->as_int(prop_mode)),
                    childObj->as_int(prop_width), static_cast<wxAlignment>(childObj->as_int(prop_align)),
                    childObj->as_int(prop_flags));
                if (childObj->hasValue(prop_ellipsize))
                {
                    col->GetRenderer()->EnableEllipsize(static_cast<wxEllipsizeMode>(childObj->as_int(prop_ellipsize)));
                }
            }
        }
    }
}

bool DataViewListCtrl::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, true);

    return true;
}

bool DataViewListCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                   GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_dataview.cpp
// ../../../wxWidgets/src/xrc/xh_dataview.cpp
// See HandleListCtrl()

int DataViewListCtrl::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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

std::pair<bool, tt_string> DataViewListCtrl::isLanguageVersionSupported(GenLang language)
{
    if (language == GEN_LANG_NONE || (language & (GEN_LANG_CPLUSPLUS | GEN_LANG_PYTHON)))
        return { true, {} };

    return { false, tt_string() << "wxDataViewListCtrl is not supported by " << ConvertFromGenLang(language) };
}

std::optional<tt_string> DataViewListCtrl::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_RUBY:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxRuby currently does not support Wx::DataViewListCtrl";
                return msg;
            }
        default:
            return {};
    }
}

//////////////////////////////////////////  DataViewTreeCtrl  //////////////////////////////////////////

wxObject* DataViewTreeCtrl::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxDataViewTreeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                         DlgSize(node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool DataViewTreeCtrl::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, true);

    return true;
}

bool DataViewTreeCtrl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                   GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/dataview.h>", set_src, set_hdr);
    return true;
}

std::pair<bool, tt_string> DataViewTreeCtrl::isLanguageVersionSupported(GenLang language)
{
    if (language == GEN_LANG_NONE || (language & (GEN_LANG_CPLUSPLUS | GEN_LANG_PYTHON)))
        return { true, {} };

    return { false, tt_string() << "wxDataViewTreeCtrl is not supported by " << ConvertFromGenLang(language) };
}

std::optional<tt_string> DataViewTreeCtrl::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_RUBY:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxRuby currently does not support Wx::DataViewTreeCtrl";
                return msg;
            }
        default:
            return {};
    }
}

// ../../wxSnapShot/src/xrc/xh_dataview.cpp
// ../../../wxWidgets/src/xrc/xh_dataview.cpp
// See HandleTreeCtrl()

int DataViewTreeCtrl::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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
    code.AddAuto().NodeName().Str(" = ").ParentName().Function("Append").as_string(prop_type).Str("Column(");
    code.QuotedString(prop_label)
        .Comma()
        .as_string(prop_model_column)
        .Comma()
        .as_string(prop_mode)
        .Comma()
        .as_string(prop_width);
    code.Comma();
    if (code.is_cpp())
        code.Str("static_cast<wxAlignment>(");
    code.Add(prop_align);
    if (code.is_cpp())
        code << ')';
    code.Comma().Add(prop_flags).EndFunction();

    if (code.hasValue(prop_ellipsize))
    {
        code.Eol().NodeName().Function("GetRenderer()").Function("EnableEllipsize(").Add(prop_ellipsize).EndFunction();
    }

    return true;
}

std::pair<bool, tt_string> DataViewColumn::isLanguageVersionSupported(GenLang language)
{
    if (language == GEN_LANG_NONE || (language & (GEN_LANG_CPLUSPLUS | GEN_LANG_PYTHON)))
    {
        return { true, {} };
    };

    return { false, tt_string() << "DataViewColumn is not supported by " << ConvertFromGenLang(language) };
}

std::optional<tt_string> DataViewColumn::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_RUBY:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxRuby currently does not support DataViewColumn";
                return msg;
            }
        default:
            return {};
    }
}

//////////////////////////////////////////  DataViewListColumn  //////////////////////////////////////////

bool DataViewListColumn::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().Str(" = ").ParentName().Function("Append").as_string(prop_type).Str("Column(");
    code.QuotedString(prop_label).Comma().as_string(prop_mode).Comma().as_string(prop_width);
    code.Comma();
    if (code.is_cpp())
        code.Str("static_cast<wxAlignment>(");
    code.Add(prop_align);
    if (code.is_cpp())
        code << ')';
    code.Comma().Add(prop_flags).EndFunction();

    if (code.hasValue(prop_ellipsize))
    {
        code.Eol().NodeName().Function("GetRenderer()").Function("EnableEllipsize(").Add(prop_ellipsize).EndFunction();
    }

    return true;
}

std::pair<bool, tt_string> DataViewListColumn::isLanguageVersionSupported(GenLang language)
{
    if (language == GEN_LANG_NONE || (language & (GEN_LANG_CPLUSPLUS | GEN_LANG_PYTHON)))
        return { true, {} };

    return { false, tt_string() << "DataViewListColumn is not supported by " << ConvertFromGenLang(language) };
}

std::optional<tt_string> DataViewListColumn::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_RUBY:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxRuby currently does not support DataViewListColumn";
                return msg;
            }
        default:
            return {};
    }
}
