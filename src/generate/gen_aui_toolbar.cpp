/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibar.h>  // wxaui: wx advanced user interface - docking window manager
#include <wx/toolbar.h>     // wxToolBar interface declaration

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_aui_toolbar.h"

wxObject* AuiToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxAuiToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node) | wxTB_NODIVIDER | wxNO_BORDER);

    if (node->HasValue(prop_bitmapsize))
        widget->SetToolBitmapSize(node->prop_as_wxSize(prop_bitmapsize));
    if (node->HasValue(prop_margins))
    {
        wxSize margins(node->prop_as_wxSize(prop_margins));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue(prop_packing))
        widget->SetToolPacking(node->prop_as_int(prop_packing));
    if (node->HasValue(prop_separation))
        widget->SetToolSeparation(node->prop_as_int(prop_separation));

    widget->Bind(wxEVT_TOOL, &AuiToolBarGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void AuiToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool is_preview)
{
    auto toolbar = wxStaticCast(wxobject, wxAuiToolBar);
    ASSERT(toolbar);
    if (!toolbar)
    {
        return;
    }
    auto count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->isGen(gen_auitool))
        {
            auto bmp = childObj->prop_as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            toolbar->AddTool(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, wxNullBitmap,
                             (wxItemKind) childObj->prop_as_int(prop_kind), childObj->prop_as_wxString(prop_help),
                             wxEmptyString, nullptr);
        }
        else if (childObj->isGen(gen_toolSeparator))
        {
            toolbar->AddSeparator();
        }
        else
        {
            const wxObject* child;
            if (!is_preview)
                child = GetMockup()->GetChild(wxobject, i);
            else
                child = node->GetChild(i)->GetMockupObject();

            if (auto control = wxDynamicCast(child, wxControl); control)
            {
                toolbar->AddControl(control);
            }
        }
    }
    toolbar->Realize();
}

std::optional<ttlib::cstr> AuiToolBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->prop_as_string(prop_var_name);

    code << " = new wxAuiToolBar(" << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, "wxAUI_TB_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> AuiToolBarGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;
    if (cmd == code_after_children)
    {
        code << '\t' << node->get_node_name() << "->Realize();";
    }
    else
    {
        return {};
    }

    return code;
}

std::optional<ttlib::cstr> AuiToolBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    auto code = GenFormSettings(node);
    if (node->HasValue(prop_bitmapsize))
    {
        if (code.size())
            code << '\n';
        code << "SetToolBitmapSize(" << node->prop_as_string(prop_bitmapsize) << ");";
    }

    if (node->prop_as_int(prop_separation) != 5)
    {
        if (code.size())
            code << '\n';
        code << "SetToolSeparation(" << node->prop_as_string(prop_separation) << ");";
    }

    if (node->HasValue(prop_margins))
    {
        if (code.size())
            code << '\n';
        code << "SetMargins(" << node->prop_as_string(prop_margins) << ");";
    }

    if (node->prop_as_int(prop_packing) != 1)
    {
        if (code.size())
            code << '\n';
        code << "SetToolPacking(" << node->prop_as_string(prop_packing) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> AuiToolBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool AuiToolBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/toolbar.h>", set_src, set_hdr);

    return true;
}

// REVIEW: [KeyWorks - 05-13-2021] Unlike wxToolBar, you can't store a pointer in a wxAuiToolBar item -- the closest
// equivalent is m_userdata, but that's a long so it won't work for storing a pointer.
void AuiToolBarGenerator::OnTool(wxCommandEvent& WXUNUSED(event))
{
#if 0
    auto toolbar = wxDynamicCast(event.GetEventObject(), wxAuiToolBar);

    auto wxobject = toolbar->GetToolClientData(event.GetToolId());
    if (wxobject)
    {
        wxGetFrame().GetMockup()->SelectNode(wxobject);
    }
#endif
}

// ../../wxSnapShot/src/xrc/xh_auitoolb.cpp
// ../../../wxWidgets/src/xrc/xh_auitoolb.cpp

int AuiToolBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxAuiToolBar");

    if (node->as_int(prop_packing) >= 0)
        ADD_ITEM_PROP(prop_packing, "packing")
    if (node->as_int(prop_separation) >= 0)
        ADD_ITEM_PROP(prop_separation, "separation")
    ADD_ITEM_PROP(prop_margins, "margins")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void AuiToolBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxAuiToolBarXmlHandler");
}

//////////////////////////////////////////  AuiToolGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> AuiToolGenerator::GenConstruction(Node* node)
{
    if (node->HasValue(prop_bitmap))
    {
        ttlib::cstr code;
        if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
        {
            code << "#if wxCHECK_VERSION(3, 1, 6)\n";
        }

        ttlib::cstr bundle_code;
        bool is_code_block = GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);
        if (is_code_block)
        {
            // GenerateBundleCode assumes an indent within an indent
            bundle_code.Replace("\t\t\t", "\t\t", true);
            code << '\t' << bundle_code;
            code << '\t' << GenToolCode(node, "wxBitmapBundle::FromBitmaps(bitmaps)");
            code << "\n\t}";
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                code << GenToolCode(node, GenerateBitmapCode(node->prop_as_string(prop_bitmap)));
                code << "\n#endif";
            }
        }
        else
        {
            code << GenToolCode(node, bundle_code);
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                code << GenToolCode(node, GenerateBitmapCode(node->prop_as_string(prop_bitmap)));
                code << "\n#endif";
            }
        }
        return code;
    }
    else
    {
        return GenToolCode(node);
    }
}

std::optional<ttlib::cstr> AuiToolGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

int AuiToolGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool /* add_comments */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item);

    return BaseGenerator::xrc_updated;
}
