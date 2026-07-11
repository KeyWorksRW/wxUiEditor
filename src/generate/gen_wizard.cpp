/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxWizard generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-04-2026]

#include <wx/wizard.h>  // wxWizard class: a GUI control presenting the user with a

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "../mockup/mockup_wizard.h"  // WizardPageSimple
#include "../panels/navpopupmenu.h"   // NavPopupMenu -- Context-menu for Navigation Panel

#include "gen_wizard.h"

wxObject* WizardFormGenerator::CreateMockup(Node* /* node */, wxObject* /* parent */)
{
    FAIL_MSG(
        "Do not call CreateMockup() for wxWizard -- you must use the MockupWizard class instead!");
    return nullptr;
}

// Static per-language FFI construction code helpers for the wizard form.
// Each generates the class/module structure plus the wx_wizard_create call.
static void WizardFormFfiConstructionCodeFortran(Code& code)
{
    // module NodeName_mod
    //     use kwx_fortran
    //     implicit none
    //     type(wx_wizard_t) :: self
    // contains
    // subroutine create(parent)
    //     type(c_ptr), intent(in) :: parent
    //     call wx_wizard_create(self, parent)

    code.Str("module ").NodeName().Str("_mod").Eol();
    code.Tab().Str("use kwx_fortran").Eol();
    code.Tab().Str("implicit none").Eol();
    code.Tab().Str("type(wx_wizard_t) :: self").Eol();
    code.Eol();
    code.Str("contains").Eol();
    code.Eol();
    code.Str("subroutine create(parent)").Eol();
    code.Tab().Str("type(c_ptr), intent(in) :: parent").Eol();
    code.Eol();
    code.Tab().Str("call wx_wizard_create(self, parent)").Eol();
}

static void WizardFormFfiConstructionCodeGo(Code& code)
{
    // type NodeName struct {
    //     wizard *wx.Wizard
    // }
    //
    // func NewNodeName(parent wx.Pointer) *NodeName {
    //     self := &NodeName{}
    //     self.wizard = wx_wizard_create(parent)

    code.Str("type ").NodeName().Str(" struct {").Eol();
    code.Tab().Str("wizard *wx.Wizard").Eol();
    code.Str("}").Eol();
    code.Eol();
    code.Str("func New").NodeName().Str("(parent wx.Pointer) *").NodeName().Str(" {").Eol();
    code.Tab().Str("self := &").NodeName().Str("{}").Eol();
    code.Eol();
    code.Tab().Str("self.wizard = wx_wizard_create(parent)").Eol();
}

static void WizardFormFfiConstructionCodeJulia(Code& code)
{
    // mutable struct NodeName
    //     wizard::Ptr{Cvoid}
    //
    //     function NodeName(parent=nothing)
    //         self = new()
    //         self.wizard = wx_wizard_create(parent)

    code.Str("mutable struct ").NodeName().Eol();
    code.Indent();
    code.Tab().Str("wizard::Ptr{Cvoid}").Eol();
    code.Eol();
    code.Tab().Str("function ").NodeName().Str("(parent=nothing)").Eol();
    code.Indent();
    code.Tab().Str("self = new()").Eol();
    code.Eol();
    code.Tab().Str("self.wizard = wx_wizard_create(parent)").Eol();
}

static void WizardFormFfiConstructionCodeLuaJIT(Code& code)
{
    // local NodeName = {}
    // NodeName.__index = NodeName
    //
    // function NodeName:new(parent)
    //     local self = setmetatable({}, NodeName)
    //     self.wizard = wx_wizard_create(parent)

    code.Str("local ").NodeName().Str(" = {}").Eol();
    code.NodeName().Str(".__index = ").NodeName().Eol();
    code.Eol();
    code.Str("function ").NodeName().Str(":new(parent)").Eol();
    code.Tab().Str("local self = setmetatable({}, ").NodeName().Str(")").Eol();
    code.Eol();
    code.Tab().Str("self.wizard = wx_wizard_create(parent)").Eol();
}

static void WizardFormFfiConstructionCodeTypeScript(Code& code)
{
    // TypeScript wizard creation inside the constructor body.
    // A wxString is created for the title, then wx_wizard_create is called.

    const LanguageTraits& traits = code.get_traits();

    code.Tab().Str("const title_wxstr = createWxString(").QuotedString(prop_title).Str(")");
    code.Str(traits.stmt_end).Eol();

    code.Tab().Str(traits.self_reference).Str(".wizard = wx_wizard_create(").Eol().Tab(2);
    code.Str("parent").Comma();
    if (code.HasValue(prop_id))
    {
        code.Add(prop_id);
    }
    else
    {
        code.Add("wxID_ANY");
    }
    code.Comma().Eol().Tab(2).Str("title_wxstr.ptr").Comma();
    code.Str("WX_NULL_BITMAP").Comma();
    code.Str("-1, -1").Comma();
    if (code.HasValue(prop_style))
    {
        code.Add(prop_style);
    }
    else
    {
        code.Add("wxDEFAULT_DIALOG_STYLE");
    }
    code.Str(")!").Str(traits.stmt_end).Eol();
    code.Tab().Str("title_wxstr.Delete()").Str(traits.stmt_end);
}

bool WizardFormGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.as_string(prop_class_name).Str("::").as_string(prop_class_name);
        code += "(wxWindow* parent, wxWindowID id, const wxString& title";
        code.Comma().Str("const wxPoint& pos").Comma().Str("long style)");
        code.Str(" : wxWizard()").Eol() += "{";
    }
    else if (code.is_python())
    {
        code.Add("class ").NodeName().Add("(wx.adv.Wizard):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").Add(prop_id);
        code.Indent(3);
        code.Comma().Str("title=").QuotedString(prop_title).Comma().Str("pos=").Pos(prop_pos);
        // wxWizard does not use a size parameter
        code.CheckLineLength(sizeof(", style=") + code.node()->as_string(prop_style).size() + 4);
        code.Comma().Add("style=").Style().Str("):");
        code.Unindent();
        code.Eol() += "wx.adv.Wizard.__init__(self)";

        code.ResetIndent();
    }
    else if (code.is_ruby())
    {
        code.Add("class ").NodeName().Add(" < Wx::Wizard").Eol();
        code.AddPublicRubyMembers();
        code.Eol(eol_if_needed).Tab().Add("def initialize(parent");
        // Indent any wrapped lines
        code.Indent(3);
        code.Str(", id = ");
        if (code.HasValue(prop_id))
        {
            code.Add(prop_id);
        }
        else
        {
            code.Add("Wx::ID_ANY");
        }
        code.Comma().Str("title = ").QuotedString(prop_title);
        code.Comma().Str("bitmap = ");
        if (code.HasValue(prop_bitmap))
        {
            code.Bundle(prop_bitmap);
        }
        else
        {
            code.Str("(Wx::BitmapBundle.new())");
        }
        code.Comma().Str("pos = ").Pos(prop_pos);
        // wxWizard does not use a size parameter
        code.Comma()
            .CheckLineLength(sizeof("style = Wx::DEFAULT_DIALOG_STYLE"))
            .Str("style = ")
            .Style();
        code.EndFunction();
        code.Unindent();

        if (auto indent_pos = code.GetCode().find("parent"); wxue::is_found(indent_pos))
        {
            auto rfind_pos = code.GetCode().rfind("\n", indent_pos - 1);
            if (wxue::is_found(rfind_pos))
            {
                indent_pos = indent_pos - (rfind_pos + 1);
                const std::string spaces(indent_pos, ' ');
                code.GetCode().Replace("\t\t\t\t", spaces, true);
            }
        }

        code.ResetIndent();
    }
    else if (code.is_ffi())
    {
        switch (code.get_language())
        {
            case GenLang::fortran:
                WizardFormFfiConstructionCodeFortran(code);
                break;
            case GenLang::go:
                WizardFormFfiConstructionCodeGo(code);
                break;
            case GenLang::julia:
                WizardFormFfiConstructionCodeJulia(code);
                break;
            case GenLang::luajit:
                WizardFormFfiConstructionCodeLuaJIT(code);
                break;
            case GenLang::typescript:
                WizardFormFfiConstructionCodeTypeScript(code);
                break;
            default:
                code.AddComment("Unsupported FFI language", true);
                break;
        }
        code.ResetIndent();
        code.ResetBraces();
    }
    else
    {
        code.AddComment("Unknown language", true);
    }

    return true;
}

bool WizardFormGenerator::SettingsCode(Code& code)
{
    if (code.is_ruby())
    {
        code.Str("super(parent, id, title, bitmap, pos, style)\n");
    }

    if (!code.node()->is_PropValue(prop_variant, "normal"))
    {
        if (code.is_ffi())
        {
            const LanguageTraits& traits = code.get_traits();
            code.Eol(eol_if_empty).Str("wx_wizard_set_window_variant(self, ");
            if (code.node()->is_PropValue(prop_variant, "small"))
            {
                code.Add("wxWINDOW_VARIANT_SMALL");
            }
            else if (code.node()->is_PropValue(prop_variant, "mini"))
            {
                code.Add("wxWINDOW_VARIANT_MINI");
            }
            else
            {
                code.Add("wxWINDOW_VARIANT_LARGE");
            }
            code.Str(")").Str(traits.stmt_end);
        }
        else
        {
            code.Eol(eol_if_empty).FormFunction("SetWindowVariant(");
            if (code.node()->is_PropValue(prop_variant, "small"))
            {
                code.Add("wxWINDOW_VARIANT_SMALL");
            }
            else if (code.node()->is_PropValue(prop_variant, "mini"))
            {
                code.Add("wxWINDOW_VARIANT_MINI");
            }
            else
            {
                code.Add("wxWINDOW_VARIANT_LARGE");
            }
            code.EndFunction();
        }
    }

    if (code.HasValue(prop_extra_style))
    {
        if (code.is_ffi())
        {
            const LanguageTraits& traits = code.get_traits();
            code.Eol(eol_if_needed)
                .Str("wx_wizard_set_extra_style(self, ")
                .Add(prop_extra_style)
                .Str(")")
                .Str(traits.stmt_end);
        }
        else
        {
            code.Eol(eol_if_needed)
                .FormFunction("SetExtraStyle(")
                .FormFunction("GetExtraStyle() | ")
                .Add(prop_extra_style);
            code.EndFunction();
        }
    }

    if (code.HasValue(prop_border) && !code.is_PropValue(prop_border, 5))
    {
        if (code.is_ffi())
        {
            const LanguageTraits& traits = code.get_traits();
            code.Eol(eol_if_needed)
                .Str("wx_wizard_set_border(self, ")
                .as_string(prop_border)
                .Str(")")
                .Str(traits.stmt_end);
        }
        else
        {
            code.Eol(eol_if_needed).FormFunction("SetBorder(").as_string(prop_border).EndFunction();
        }
    }

    if (code.IntValue(prop_bmp_placement))
    {
        if (code.is_ffi())
        {
            const LanguageTraits& traits = code.get_traits();
            code.Eol(eol_if_needed)
                .Str("wx_wizard_set_bitmap_placement(self, ")
                .as_string(prop_bmp_placement)
                .Str(")")
                .Str(traits.stmt_end);
            if (code.IntValue(prop_bmp_min_width) > 0)
            {
                code.Eol()
                    .Str("wx_wizard_set_bitmap_min_width(self, ")
                    .as_string(prop_bmp_min_width)
                    .Str(")")
                    .Str(traits.stmt_end);
            }
            if (code.HasValue(prop_bmp_background_colour))
            {
                code.Eol()
                    .Str("wx_wizard_set_bitmap_background_colour(self, ")
                    .ColourCode(prop_bmp_background_colour)
                    .Str(")")
                    .Str(traits.stmt_end);
            }
        }
        else
        {
            code.Eol(eol_if_needed)
                .FormFunction("SetBitmapPlacement(")
                .as_string(prop_bmp_placement)
                .EndFunction();
            if (code.IntValue(prop_bmp_min_width) > 0)
            {
                code.Eol()
                    .FormFunction("SetBitmapMinWidth(")
                    .as_string(prop_bmp_min_width)
                    .EndFunction();
            }
            if (code.HasValue(prop_bmp_background_colour))
            {
                code.Eol()
                    .FormFunction("SetBitmapBackgroundColour(")
                    .ColourCode(prop_bmp_background_colour)
                    .EndFunction();
            }
        }
    }

    // Ruby passed the bitmap via the constructor; FFI uses wx_wizard_create in ConstructionCode
    if (code.HasValue(prop_bitmap) && !code.is_ruby() && !code.is_ffi())
    {
        const bool is_bitmaps_list = BitmapList(code, prop_bitmap);
        if (code.is_cpp())
        {
            code.Eol(eol_if_needed).Str("if (!Create(parent, id, title").Comma();
        }
        else if (code.is_python())
        {
            code.Eol(eol_if_needed).Str("if not self.Create(parent, id, title").Comma();
        }
        if (is_bitmaps_list)
        {
            if (code.is_cpp())
            {
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
            }
            else if (code.is_python())
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
        }
        else
        {
            if (code.is_cpp())
            {
                wxue::string bundle_code;
                GenerateBundleCode(code.node()->as_string(prop_bitmap), bundle_code);
                code.CheckLineLength(bundle_code.size());
                code += bundle_code;
            }
            else
            {
                code.Bundle(prop_bitmap);
            }
        }
        if (code.is_cpp())
        {
            code.Comma().Str("pos").Comma().Str("style))");
            code.Eol().OpenBrace().Str("return;").CloseBrace();
        }
        else
        {
            code.Comma().Str("pos").Comma().Str("style):");
            code.Eol().Tab().Str("return");
        }
    }
    else
    {
        if (code.is_cpp())
        {
            code.Eol(eol_if_needed)
                .FormFunction("if (!Create(")
                .Str("parent, id, title, wxNullBitmap, pos, style))");
            code.Eol().OpenBrace().Str("return;").CloseBrace();
        }
        else if (code.is_python())
        {
            code.Eol(eol_if_needed)
                .Str("if not self.Create(parent, id, title, wx.BitmapBundle(), pos, style):");
            code.Eol().Tab().Str("return");
        }
        // wxRuby3 and FFI code generation don't use 2-step construction here.
    }

    return true;
}

bool WizardFormGenerator::AfterChildrenCode(Code& code)
{
    if (auto panes = GetChildPanes(code.node()); !panes.empty())
    {
        if (panes.size() > 1)
        {
            if (code.is_ffi())
            {
                // FFI uses a standalone function for chaining consecutive wizard pages.
                const LanguageTraits& ffi_chain_traits = code.get_traits();
                for (size_t pos = 0; pos + 1 < panes.size(); ++pos)
                {
                    code.Eol(eol_if_needed);
                    if (code.get_language() == GenLang::fortran)
                    {
                        code.Str("call ");
                    }
                    code.Str("wx_wizard_page_simple_chain(");
                    code.Str(panes[pos]->as_string(prop_var_name)).Comma();
                    code.Str(panes[pos + 1]->as_string(prop_var_name));
                    code.Str(")").Str(ffi_chain_traits.stmt_end);
                }
            }
            else
            {
                code.Eol(eol_if_needed).Str(panes[0]->as_string(prop_var_name)).Function("Chain(");
                code.Str(panes[1]->as_string(prop_var_name)) += ")";
                for (size_t pos = 1; pos + 1 < panes.size(); ++pos)
                {
                    // In C++, Chain() returns a reference, so "." is used instead of "->"
                    // Python and Ruby both use "."
                    if (code.is_cpp())
                    {
                        code.Str(".").Add("Chain(");
                    }
                    else
                    {
                        code.Function("Chain(");
                    }
                    code.Str(panes[pos + 1]->as_string(prop_var_name)) += ")";
                }
                if (code.is_cpp())
                {
                    code += ";";
                }
            }
        }
        if (code.is_ffi())
        {
            const LanguageTraits& ffi_sizer_traits = code.get_traits();
            code.Eol(eol_if_needed);
            if (code.get_language() == GenLang::fortran)
            {
                code.Str("call wx_sizer_add(wx_wizard_get_page_area_sizer(self), ");
            }
            else
            {
                code.Str("wx_sizer_add(wx_wizard_get_page_area_sizer(self), ");
            }
            code.Str(panes[0]->as_string(prop_var_name)).Str(")").Str(ffi_sizer_traits.stmt_end);
        }
        else
        {
            code.Eol(eol_if_needed).FormFunction("GetPageAreaSizer()").Function("Add(");
            code.Str(panes[0]->as_string(prop_var_name)).EndFunction();
        }
    }

    if (const auto& center = code.node()->as_string(prop_center);
        !center.empty() && !center.is_sameas("no"))
    {
        if (code.is_ffi())
        {
            const LanguageTraits& ffi_center_traits = code.get_traits();
            if (code.get_language() == GenLang::fortran)
            {
                code.Eol(eol_if_needed).Str("call wx_wizard_center(self, ").Add(center).Str(")");
            }
            else
            {
                code.Eol(eol_if_needed).Str("wx_wizard_center(self, ").Add(center).Str(")");
            }
            code.Str(ffi_center_traits.stmt_end);
        }
        else
        {
            code.Eol(eol_if_needed).FormFunction("Center(").Add(center).EndFunction();
        }
    }

    return true;
}

bool WizardFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += "wxWizard";
    }
    return true;
}

bool WizardFormGenerator::HeaderCode(Code& code)
{
    const Node* node = code.node();

    code.as_string(prop_class_name).Str("(wxWindow* parent, wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxString& title = ");
    const std::string& title = node->as_string(prop_title);
    if (code.HasValue(prop_title))
    {
        code.QuotedString(title);
    }
    else
    {
        code.Str("wxEmptyString");
    }

    code.Comma().Str("const wxPoint& pos = ");

    const wxPoint position = node->as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
    {
        code.Str("wxDefaultPosition");
    }
    else
    {
        code.Pos(prop_pos, no_dpi_scaling);
    }

    const std::string& style = node->as_string(prop_style);
    const std::string& win_style = node->as_string(prop_window_style);
    if (style.empty() && win_style.empty())
    {
        code.Comma().Str("long style = 0");
    }
    else
    {
        code.Comma();
        code.CheckLineLength(style.size() + win_style.size() + sizeof("long style = "));
        code.Str("long style = ");
        if (!style.empty())
        {
            code.CheckLineLength(style.size() + win_style.size());
            code += style;
            if (!win_style.empty())
            {
                code << '|' << win_style;
            }
        }
        else if (!win_style.empty())
        {
            code.Str(win_style);
        }
    }
    code.EndFunction();
    code.Eol().Eol() += "bool Run() { return RunWizard((wxWizardPage*) "
                        "GetPageAreaSizer()->GetItem((size_t) 0)->GetWindow()); }";
    code.Eol().Eol();

    return true;
}

bool WizardFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                      std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/wizard.h>", set_src, set_hdr);

    // This is needed for the Run() command
    set_hdr.insert("#include <wx/sizer.h>");
    return true;
}

std::vector<Node*> WizardFormGenerator::GetChildPanes(Node* parent)
{
    std::vector<Node*> panes;

    for (const auto& child: parent->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_wxWizardPageSimple))
        {
            panes.emplace_back(child.get());
        }
    }

    return panes;
}

std::optional<wxue::string> WizardFormGenerator::GetHint(NodeProperty* prop)
{
    if (prop->isProp(prop_title) && !prop->getNode()->HasValue(prop_title))
    {
        return wxue::string("Title bar text");
    }
    return {};
}

bool WizardFormGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* node)
{
    menu->Append(std::to_underlying(NavPopupMenu::Menu::AddWizardPage), "Add Page");
    menu->Bind(
        wxEVT_MENU,
        [=](wxCommandEvent&)
        {
            std::ignore = node->CreateToolNode(gen_wxWizardPageSimple);
        },
        std::to_underlying(NavPopupMenu::Menu::AddWizardPage));

    return false;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp

int WizardFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    // We use item so that the macros in base_generator.h work, and the code looks the same
    // as other widget XRC generatorsl
    pugi::xml_node item = object;
    GenXrcObjectAttributes(node, item, "wxWizard");

    ADD_ITEM_PROP(prop_title, "title")
    GenXrcBitmap(node, item, xrc_flags);

    if (node->HasValue(prop_center))
    {
        if (node->as_string(prop_center).is_sameas("wxVERTICAL") ||
            node->as_string(prop_center).is_sameas("wxHORIZONTAL"))
        {
            if (xrc_flags & xrc::add_comments)
            {
                item.append_child(pugi::node_comment)
                    .set_value((wxue::string(node->as_string(prop_center))
                                << " cannot be set in the XRC file."));
            }
            item.append_child("centered").text().set(1);
        }
        else
        {
            item.append_child("centered")
                .text()
                .set(node->as_string(prop_center).is_sameas("no") ? 0 : 1);
        }
    }

    if (node->HasValue(prop_style))
    {
        if ((xrc_flags & xrc::add_comments) &&
            node->as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            item.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }
        if (!node->HasValue(prop_extra_style))
        {
            item.append_child("style").text().set(node->as_string(prop_style));
        }
        else
        {
            wxue::string all_styles = node->as_string(prop_style);
            all_styles << '|' << node->as_string(prop_extra_style);
            item.append_child("style").text().set(all_styles);
        }
    }

    if (node->HasValue(prop_pos))
    {
        item.append_child("pos").text().set(node->as_string(prop_pos));
    }
    if (node->HasValue(prop_size))
    {
        item.append_child("size").text().set(node->as_string(prop_size));
    }

    if (node->HasValue(prop_border) && node->as_int(prop_border) > 0)
    {
        item.append_child("border").text().set(node->as_string(prop_border));
    }

    if (node->HasValue(prop_bmp_placement))
    {
        item.append_child("bitmap-placement").text().set(node->as_string(prop_bmp_placement));
        if (node->as_int(prop_bmp_min_width) > 0)
        {
            item.append_child("bitmap-minwidth").text().set(node->as_string(prop_bmp_min_width));
        }
        if (node->HasValue(prop_bmp_background_colour))
        {
            item.append_child("bitmap-bg")
                .text()
                .set(node->as_wxColour(prop_bmp_background_colour)
                         .GetAsString(wxC2S_HTML_SYNTAX)
                         .ToUTF8()
                         .data());
        }
    }

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_bool(prop_persist))
        {
            item.append_child(pugi::node_comment)
                .set_value(" persist is not supported in the XRC file. ");
        }

        GenXrcComments(node, item);
    }

    return xrc_updated;
}

void WizardFormGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxWizardXmlHandler");
    if (node->HasValue(prop_bitmap))
    {
        handlers.emplace("wxBitmapXmlHandler");
    }
}

//////////////////////////////////////////  WizardPageGenerator
/////////////////////////////////////////////

wxObject* WizardPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    return new MockupWizardPage(node, parent);
}

bool WizardPageGenerator::ConstructionCode(Code& code)
{
    if (!code.HasValue(prop_bitmap))
    {
        code.AddAuto().as_string(prop_var_name).CreateClass();
        if (code.is_cpp())
        {
            code << "this";
        }
        else if (code.is_python() || code.is_ruby())
        {
            code << "self";
        }
        else if (code.is_ffi())
        {
            const LanguageTraits& ffi_page_traits = code.get_traits();
            if (!ffi_page_traits.self_reference.empty())
            {
                code << ffi_page_traits.self_reference;
            }
        }
        code.EndFunction();
    }
    else
    {
        const bool is_bitmaps_list = BitmapList(code, prop_bitmap);
        code.AddAuto().as_string(prop_var_name).CreateClass();
        if (code.is_cpp())
        {
            code << "this";
        }
        else if (code.is_python() || code.is_ruby())
        {
            code << "self";
        }
        else if (code.is_ffi())
        {
            const LanguageTraits& ffi_bmp_page_traits = code.get_traits();
            if (!ffi_bmp_page_traits.self_reference.empty())
            {
                code << ffi_bmp_page_traits.self_reference;
            }
        }
        if (code.is_cpp())
        {
            code.Comma().Str("nullptr, nullptr").Comma();
        }
        else if (code.is_python())
        {
            code.Comma().Str("None, None").Comma();
        }
        else if (code.is_ruby())
        {
            code.Comma().Str("nil, nil").Comma();
        }
        else if (code.is_ffi())
        {
            const LanguageTraits& ffi_null_traits = code.get_traits();
            code.Comma()
                .Str(ffi_null_traits.null_literal)
                .Comma()
                .Str(ffi_null_traits.null_literal)
                .Comma();
        }
        if (is_bitmaps_list)
        {
            if (code.is_cpp())
            {
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
            }
            else if (code.is_python())
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
            else if (code.is_ruby())
            {
                code += "Wx::BitmapBundle.from_bitmaps(bitmaps)";
            }
            else if (code.is_ffi())
            {
                // BundleFfi always returns WX_NULL_BITMAP; use that constant directly.
                code += "WX_NULL_BITMAP";
            }
        }
        else
        {
            if (code.is_cpp())
            {
                wxue::string bundle_code;
                GenerateBundleCode(code.node()->as_string(prop_bitmap), bundle_code);
                code.CheckLineLength(bundle_code.size());
                code += bundle_code;
            }
            else
            {
                code.Bundle(prop_bitmap);
            }
        }
        code.EndFunction();
    }

    return true;
}

bool WizardPageGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* node)
{
    menu->Append(std::to_underlying(NavPopupMenu::Menu::AddWizardPage), "Add Page");
    menu->Bind(
        wxEVT_MENU,
        [=](wxCommandEvent&)
        {
            std::ignore = node->CreateToolNode(gen_wxWizardPageSimple);
        },
        std::to_underlying(NavPopupMenu::Menu::AddWizardPage));

    if (node->get_ChildCount() && node->get_Child(0)->is_Sizer())
    {
        menu->MenuAddChildSizerCommands(node->get_Child(0));
    }

    return true;
}

int WizardPageGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    constexpr int result = BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxWizardPageSimple");
    GenXrcBitmap(node, item, xrc_flags);
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void WizardPageGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxWizardXmlHandler");
}
