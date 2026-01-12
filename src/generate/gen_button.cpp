/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>  // wxButtonBase class

#include "gen_button.h"

#include "bitmaps.h"        // Contains various images handling functions
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "node_creator.h"   // NodeCreator class
#include "panels/navpopupmenu.h"  // NavPopupMenu
#include "undo_cmds.h"      // InsertNodeAction -- Undoable command classes
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // Write code to Scintilla or file
#include "wxue_namespace/wxue_string.h"  // wxue::string

wxObject* ButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxButton(wxStaticCast(parent, wxWindow), node->as_id(prop_id), wxEmptyString,
                     DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_label))
    {
        if (node->as_bool(prop_markup))
            widget->SetLabelMarkup(node->as_wxString(prop_label));
        else
            widget->SetLabel(node->as_wxString(prop_label));
    }

    if (node->as_bool(prop_default))
    {
        widget->SetDefault();
        if (auto dlg = wxDynamicCast(parent, wxDialog); dlg && node->as_id(prop_id) != wxID_ANY)
            dlg->SetAffirmativeId(node->as_id(prop_id));
    }
    else
    {
        if (auto dlg = wxDynamicCast(parent, wxDialog); dlg && node->as_id(prop_id) != wxID_ANY)
        {
            switch (node->as_id(prop_id))
            {
                case wxID_OK:
                case wxID_YES:
                case wxID_SAVE:
                    dlg->SetAffirmativeId(node->as_id(prop_id));
                    break;

                case wxID_CANCEL:
                case wxID_CLOSE:
                case wxID_NO:
                    dlg->SetEscapeId(node->as_id(prop_id));
                    break;

                default:
                    break;
            }
        }
    }

    if (node->as_bool(prop_auth_needed))
        widget->SetAuthNeeded();

    if (node->HasValue(prop_bitmap))
    {
        widget->SetBitmap(node->as_wxBitmapBundle(prop_bitmap));

        if (node->HasValue(prop_disabled_bmp))
            widget->SetBitmapDisabled(node->as_wxBitmapBundle(prop_disabled_bmp));

        if (node->HasValue(prop_pressed_bmp))
            widget->SetBitmapPressed(node->as_wxBitmapBundle(prop_pressed_bmp));

        if (node->HasValue(prop_focus_bmp))
            widget->SetBitmapFocus(node->as_wxBitmapBundle(prop_focus_bmp));

        if (node->HasValue(prop_current))
            widget->SetBitmapCurrent(node->as_wxBitmapBundle(prop_current));

        if (node->HasValue(prop_position))
            widget->SetBitmapPosition(static_cast<wxDirection>(node->as_int(prop_position)));

        if (node->HasValue(prop_margins))
            widget->SetBitmapMargins(node->as_wxSize(prop_margins));
    }

    if (!node->is_PropValue(prop_variant, "normal"))
    {
        if (node->is_PropValue(prop_variant, "small"))
            widget->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
        else if (node->is_PropValue(prop_variant, "mini"))
            widget->SetWindowVariant(wxWINDOW_VARIANT_MINI);
        else
            widget->SetWindowVariant(wxWINDOW_VARIANT_LARGE);
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    // In case the widget hasn't been fully specified yet
    if (!widget || !node || !prop)
        return false;

    // We do not support changing the "markup" property because while the control displays correctly
    // when markup is set, it does not revert when markup is cleared (at least on Windows where
    // markup controls whether a generic or native version of the button is displayed).

    if (prop->isProp(prop_label) && prop->HasValue())
    {
        auto ctrl = wxStaticCast(widget, wxButton);
        if (node->as_bool(prop_markup))
            ctrl->SetLabelMarkup(node->as_wxString(prop_label));
        else
            ctrl->SetLabel(node->as_wxString(prop_label));

        return true;
    }
    else if (prop->isProp(prop_markup))
    {
        // Turning markup on switches to generic rending of the button. However, you have to
        // recreate it to switch it off and go back to native rendering.

        if (node->as_bool(prop_markup))
        {
            wxStaticCast(widget, wxButton)->SetLabelMarkup(node->as_wxString(prop_label));
            return true;
        }
    }
    else if (prop->isProp(prop_default))
    {
        // You can change a button to be the default, but you cannot change it back without
        // recreating it.
        if (prop->as_bool())
        {
            wxStaticCast(widget, wxButton)->SetDefault();
            return true;
        }
    }

    return false;
}

bool ButtonGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();

    // If prop_markup is set, then the label will be set in SettingsCode() -- with the exception of
    // wxPerl which doesn't support SetLabelMarkup
    if (code.HasValue(prop_label) && (!code.IsTrue(prop_markup) || code.is_perl()))
    {
        code.QuotedString(prop_label);
    }
    else
    {
        code.Add("wxEmptyString");
    }

    code.PosSizeFlags(code::allow_scaling, true);

    return true;
}

bool ButtonGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_markup) && code.HasValue(prop_label))
    {
        if (!code.is_perl())
        {
            code.Eol(eol_if_needed)
                .NodeName()
                .Function("SetLabelMarkup(")
                .QuotedString(prop_label)
                .EndFunction();
        }
        else
        {
            code.AddComment("wxPerl does not support SetLabelMarkup", true);
        }
    }

    if (code.IsTrue(prop_default))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetDefault(").EndFunction();
    }

    if (code.IsTrue(prop_auth_needed))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetAuthNeeded(").EndFunction();
    }

    if (code.HasValue(prop_bitmap))
    {
        if (code.HasValue(prop_position))
        {
            code.Eol(eol_if_needed)
                .NodeName()
                .Function("SetBitmapPosition(")
                .as_string(prop_position)
                .EndFunction();
        }

        if (code.HasValue(prop_margins))
        {
            auto size = code.node()->as_wxSize(prop_margins);
            code.Eol(eol_if_needed)
                .NodeName()
                .Function("SetBitmapMargins(")
                .itoa(size.GetWidth())
                .Comma()
                .itoa(size.GetHeight())
                .EndFunction();
        }

        if (code.is_cpp())
            GenBtnBimapCode(code.node(), code);
        else
            PythonBtnBimapCode(code);
    }

    return true;
}

int ButtonGenerator::GetRequiredVersion(Node* node)
{
    // Code generation was invalid in minRequiredVer when there no label was set
    if (!node->HasValue(prop_label) && !node->as_bool(prop_markup))
    {
        return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
    }

    return BaseGenerator::GetRequiredVersion(node);
}

int ButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxButton");

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_markup, "markup")
    ADD_ITEM_BOOL(prop_default, "default")

    GenXrcBitmap(node, item, xrc_flags);
    GenXrcStylePosSize(node, item);

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_bool(prop_markup))
        {
            ADD_ITEM_COMMENT(" markup cannot be be set in the XRC file. ")
        }
        if (node->as_bool(prop_auth_needed))
        {
            ADD_ITEM_COMMENT(" authentication cannot be be set in the XRC file. ")
        }
        GenXrcComments(node, item);
    }

    return result;
}

void ButtonGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    bool old_button = (node->HasValue(prop_disabled_bmp) || node->HasValue(prop_pressed_bmp) ||
                       node->HasValue(prop_focus_bmp) || node->HasValue(prop_current));

    handlers.emplace(old_button ? "wxBitmapButtonXmlHandler" : "wxButtonXmlHandler");
}

bool ButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                  std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    if (node->HasValue(prop_validator_variable))
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

bool ButtonGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                 GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use Wx qw[:button];");
        return true;
    }
    return false;
}

namespace
{
    // Check if the button ID is a standard dialog button ID
    [[nodiscard]] auto IsStandardDialogButtonId(std::string_view id) -> bool
    {
        return (id == "wxID_OK" || id == "wxID_CANCEL" || id == "wxID_YES" || id == "wxID_NO" ||
                id == "wxID_APPLY" || id == "wxID_HELP" || id == "wxID_SAVE" || id == "wxID_CLOSE" ||
                id == "wxID_CONTEXT_HELP");
    }

    // Map standard button IDs to wxStdDialogButtonSizer property names
    [[nodiscard]] auto GetStdButtonProperty(std::string_view id) -> std::string_view
    {
        if (id == "wxID_OK")
        {
            return "OK";
        }
        if (id == "wxID_CANCEL")
        {
            return "Cancel";
        }
        if (id == "wxID_YES")
        {
            return "Yes";
        }
        if (id == "wxID_NO")
        {
            return "No";
        }
        if (id == "wxID_APPLY")
        {
            return "Apply";
        }
        if (id == "wxID_HELP")
        {
            return "Help";
        }
        if (id == "wxID_SAVE")
        {
            return "Save";
        }
        if (id == "wxID_CLOSE")
        {
            return "Close";
        }
        if (id == "wxID_CONTEXT_HELP")
        {
            return "ContextHelp";
        }
        return "";
    }
}  // namespace

bool ButtonGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* node)
{
    // Check if this button has a standard dialog button ID
    const auto id = node->as_string(prop_id);
    if (!IsStandardDialogButtonId(id))
    {
        return true;  // Return true to add default sizer commands
    }

    // Check if parent is a sizer in a dialog
    auto parent = node->get_Parent();
    if (!parent || !parent->is_Sizer())
    {
        return true;
    }

    // Check if we're in a dialog form
    if (!node->is_Parent(gen_wxDialog))
    {
        return true;
    }

    // Add menu separator and conversion command
    menu->AppendSeparator();
    const int MenuConvertToStdDialogButtonSizer = wxNewId();
    auto* menu_item = menu->Append(MenuConvertToStdDialogButtonSizer,
                                    "Convert to wxStdDialogButtonSizer");
    menu_item->SetBitmap(GetInternalImage("stddialogbuttonsizer"));

    menu->Bind(
        wxEVT_MENU,
        [node](wxCommandEvent&)
        {
            // Find parent sizer
            auto* parent_sizer = node->get_Parent();
            if (!parent_sizer || !parent_sizer->is_Sizer())
            {
                return;
            }

            // Collect all buttons with standard IDs in the same parent sizer
            std::vector<Node*> buttons_to_convert;
            for (const auto& child: parent_sizer->get_ChildNodePtrs())
            {
                if (child->is_Gen(gen_wxButton))
                {
                    const auto child_id = child->as_string(prop_id);
                    if (IsStandardDialogButtonId(child_id))
                    {
                        buttons_to_convert.push_back(child.get());
                    }
                }
            }

            if (buttons_to_convert.empty())
            {
                return;
            }

            // Create new wxStdDialogButtonSizer node
            auto std_btn_sizer = NodeCreation.CreateNode(gen_wxStdDialogButtonSizer, parent_sizer);
            if (!std_btn_sizer.first)
            {
                return;
            }

            // Set properties on the wxStdDialogButtonSizer based on the buttons found
            wxue::string default_btn;
            for (auto* btn: buttons_to_convert)
            {
                const auto btn_id = btn->as_string(prop_id);
                const auto prop_name = GetStdButtonProperty(btn_id);
                if (!prop_name.empty())
                {
                    std_btn_sizer.first->set_value(prop_name, true);

                    // Check if this button is the default button
                    if (btn->as_bool(prop_default))
                    {
                        // Map the wxID to the property value name
                        const auto default_name = GetStdButtonProperty(btn_id);
                        if (!default_name.empty())
                        {
                            default_btn = default_name;
                        }
                    }

                    // TODO: Transfer custom text (labels different from standard)
                    // TODO: Transfer event handlers
                }
            }

            if (!default_btn.empty())
            {
                std_btn_sizer.first->set_value(prop_default_button, default_btn);
            }

            // Start undo operation
            wxGetFrame().Freeze();

            // Find the position of the first button to insert the sizer at that position
            const auto first_btn_pos = parent_sizer->get_ChildPosition(buttons_to_convert[0]);

            // Insert the wxStdDialogButtonSizer
            wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(
                std_btn_sizer.first.get(), parent_sizer, "Convert to wxStdDialogButtonSizer",
                static_cast<int>(first_btn_pos)));

            // Remove all the buttons
            for (auto* btn: buttons_to_convert)
            {
                wxGetFrame().PushUndoAction(
                    std::make_shared<RemoveNodeAction>(btn, "Remove converted button"));
            }

            // Select the new sizer
            wxGetFrame().SelectNode(std_btn_sizer.first.get(),
                                    evt_flags::fire_event | evt_flags::force_selection);
            wxGetFrame().Thaw();
        },
        MenuConvertToStdDialogButtonSizer);

    return true;  // Return true to add default sizer commands
}
