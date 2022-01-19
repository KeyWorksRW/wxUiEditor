/////////////////////////////////////////////////////////////////////////////
// Purpose:   Form component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_base.h"    // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "node_prop.h"   // NodeProperty -- NodeProperty class
#include "utils.h"       // Utility functions that work with properties
#include "write_code.h"  // WriteCode -- Write code to Scintilla or file

#include "form_widgets.h"

//////////////////////////////////////////  DialogFormGenerator  //////////////////////////////////////////

bool DialogFormGenerator::GenConstruction(Node* node, BaseCodeGenerator* code_gen)
{
    auto src_code = code_gen->GetSrcWriter();

    ttlib::cstr code;

    code << "bool " << node->prop_as_string(prop_class_name)
         << "::Create(wxWindow *parent, wxWindowID id, const wxString &title,\n\t\tconst wxPoint&pos, const wxSize& size, "
            "long "
            "style, const wxString &name)\n{\n\tif (!wxDialog::Create(parent, id, title, pos, size, style, "
            "name))\n\t\treturn "
            "false;\n\n";

    src_code->writeLine(code, indent::none);
    src_code->Indent();
    code.clear();

    if (node->HasValue(prop_extra_style))
    {
        code << "SetExtraStyle(GetExtraStyle() | " << node->prop_as_string(prop_extra_style) << ");";
        src_code->writeLine(code);
        code.clear();
    }

    if (node->HasValue(prop_icon))
    {
        code_gen->GenerateHandlers();

        auto image_code = GenerateBitmapCode(node->prop_as_string(prop_icon));
        if (!image_code.contains(".Scale") && image_code.is_sameprefix("wxImage("))
        {
            code << "SetIcon(wxIcon(" << image_code.subview(sizeof("wxImage")) << ");";
            src_code->writeLine(code);
        }
        else
        {
            src_code->writeLine("{");
            src_code->Indent();
            src_code->writeLine("wxIcon icon;");
            code << "icon.CopyFromBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_icon)) << ");\n";
            code << "SetIcon(wxIcon(icon));";
            src_code->writeLine(code);
            src_code->writeLine("}");
            src_code->Unindent();
        }
        code.clear();
    }

    code << GenFontColourSettings(node);
    if (code.size())
    {
        src_code->writeLine(code);
        code.clear();
    }

    src_code->Unindent();
    src_code->writeLine();

    return true;
}

std::optional<ttlib::cstr> DialogFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> DialogFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_after_children)
    {
        ttlib::cstr code;

        Node* dlg;
        if (node->IsForm())
        {
            dlg = node;
            ASSERT_MSG(dlg->GetChildCount(), "Trying to generate code for a dialog with no children.")
            if (!dlg->GetChildCount())
                return {};  // empty dialog, so nothing to do
            ASSERT_MSG(dlg->GetChild(0)->IsSizer(), "Expected first child of a dialog to be a sizer.");
            if (dlg->GetChild(0)->IsSizer())
                node = dlg->GetChild(0);
        }
        else
        {
            dlg = node->FindParentForm();
        }

        auto min_size = dlg->prop_as_wxSize(prop_minimum_size);
        auto max_size = dlg->prop_as_wxSize(prop_maximum_size);
        auto size = dlg->prop_as_wxSize(prop_size);

        if (min_size == wxDefaultSize && max_size == wxDefaultSize)
        {
            code << "\tSetSizerAndFit(" << node->get_node_name() << ");";
        }
        else
        {
            code << "\tSetSizer(" << node->get_node_name() << ");";
            if (min_size != wxDefaultSize)
            {
                code << "\n\tSetMinSize(wxSize(" << min_size.GetWidth() << ", " << min_size.GetHeight() << "));";
            }
            if (max_size != wxDefaultSize)
            {
                code << "\n\tSetMaxSize(wxSize(" << max_size.GetWidth() << ", " << max_size.GetHeight() << "));";
            }
            code << "\n\tFit();";
        }

        if (size != wxDefaultSize)
        {
            code << "\n\tSetSize(wxSize(" << size.GetWidth() << ", " << size.GetHeight() << "));";
        }

        auto& center = dlg->prop_as_string(prop_center);
        if (center.size() && !center.is_sameas("no"))
        {
            code << "\n\tCentre(" << center << ");";
        }

        return code;
    }
    else if (cmd == code_header)
    {
        ttlib::cstr code;
        code.reserve(256);

        code << node->get_node_name() << "() {}\n";
        code << node->get_node_name() << "(wxWindow *parent, ";
        code << "wxWindowID id = " << node->prop_as_string(prop_id) << ", ";
        code << "const wxString& title = " << GenerateQuotedString(node, prop_title) << ",\n\t";
        code << "const wxPoint& pos = ";

        auto position = node->prop_as_wxPoint(prop_pos);
        if (position == wxDefaultPosition)
            code << "wxDefaultPosition, ";
        else
            code << "wxPoint(" << position.x << ", " << position.y << "), ";

        code << "const wxSize& size = ";

        auto size = node->prop_as_wxSize(prop_size);
        if (size == wxDefaultSize)
            code << "wxDefaultSize";
        else
            code << "wxSize(" << size.x << ", " << size.y << ")";

        code << ",\n\tlong style = ";
        if (node->HasValue(prop_style))
            code << node->prop_as_string(prop_style);
        else
            code << "wxDEFAULT_DIALOG_STYLE";

        code << ", const wxString &name = ";
        if (node->HasValue(prop_window_name))
            code << GenerateQuotedString(node, prop_window_name);
        else
            code << "wxDialogNameStr";

        code << ")\n{\n\tCreate(parent, id, title, pos, size, style, name);\n}\n";

        code << "\nbool Create(wxWindow *parent, ";
        code << "wxWindowID id = " << node->prop_as_string(prop_id) << ", ";
        code << "const wxString& title = " << GenerateQuotedString(node, prop_title) << ",\n\t";
        code << "const wxPoint& pos = ";

        if (position == wxDefaultPosition)
            code << "wxDefaultPosition, ";
        else
            code << "wxPoint(" << position.x << ", " << position.y << "), ";

        code << "const wxSize& size = ";

        if (size == wxDefaultSize)
            code << "wxDefaultSize";
        else
            code << "wxSize(" << size.x << ", " << size.y << ")";

        code << ",\n\tlong style = ";
        if (node->HasValue(prop_style))
            code << node->prop_as_string(prop_style);
        else
            code << "wxDEFAULT_DIALOG_STYLE";

        code << ", const wxString &name = ";
        if (node->HasValue(prop_window_name))
            code << GenerateQuotedString(node, prop_window_name);
        else
            code << "wxDialogNameStr";

        code << ");\n\n";

        return code;
    }

    else
    {
        return GenFormCode(cmd, node);
    }
}

bool DialogFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  FrameFormGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> FrameFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, wxWindowID id, const wxString& title,";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxFrame(parent, id, title, pos, size, style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> FrameFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    return GenFormCode(cmd, node);
}

std::optional<ttlib::cstr> FrameFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> FrameFormGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    return GenFormSettings(node);
}

bool FrameFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/frame.h>", set_src, set_hdr);

    return true;
}

bool FrameFormGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_extra_style))
    {
        auto property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        ttString newValue = property->ValueToString(variant);
        if (newValue.IsEmpty())
            return true;

        if (newValue.contains("wxFRAME_EX_CONTEXTHELP"))
        {
            auto& style = node->prop_as_string(prop_style);
            if (style.contains("wxDEFAULT_FRAME_STYLE") || style.contains("wxMINIMIZE_BOX") ||
                style.contains("wxMINIMIZE_BOX"))
            {
                wxMessageBox("You can't add a context help button if there is a minimize or maximize button "
                             "(wxDEFAULT_FRAME_STYLE contains these).",
                             "Invalid button");
                event->Veto();
                return false;
            }
        }
    }

    return true;
}

//////////////////////////////////////////  PopupWinGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> PopupWinGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, int style) : wxPopupTransientWindow(parent, style)\n{";

    return code;
}

std::optional<ttlib::cstr> PopupWinGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_header)
    {
        code << node->get_node_name() << "(wxWindow* parent, int style = " << node->prop_as_string(prop_border);
        if (node->HasValue(prop_style))
        {
            code << " | " << node->prop_as_string(prop_style);
        }
        code << ");";
        return code;
    }
    else if (cmd == code_base_class)
        return GenFormCode(cmd, node);

    return {};
}

std::optional<ttlib::cstr> PopupWinGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> PopupWinGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    auto code = GenFontColourSettings(node);

    if (code.empty())
    {
        return {};
    }
    else
    {
        return code;
    }
}

bool PopupWinGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/popupwin.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  PanelFormGenerator  //////////////////////////////////////////

bool PanelFormGenerator::GenConstruction(Node* node, BaseCodeGenerator* code_gen)
{
    auto src_code = code_gen->GetSrcWriter();

    ttlib::cstr code;
    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);

    code << "(wxWindow* parent, wxWindowID id) : wxPanel()\n{";
    src_code->writeLine(code, indent::none);
    src_code->Indent();
    code.clear();

    code << "Create(parent, id";
    auto position = node->prop_as_wxPoint(prop_pos);
    if (position == wxDefaultPosition && node->prop_as_string(prop_window_style) == "wxTAB_TRAVERSAL")
    {
        code << ");";
        src_code->writeLine(code);
    }
    else
    {
        code << ", ";
        if (position != wxDefaultPosition)
        {
            code << "wxPoint(" << position.x << ", " << position.y << ")";
        }
        else
        {
            code << "wxDefaultPosition";
        }
        code << ", wxDefaultSize,";
        if (node->HasValue(prop_window_style))
        {
            src_code->writeLine(code);
            src_code->Indent();
            code = node->prop_as_string(prop_window_style) + ");";
            src_code->writeLine(code);
            src_code->Unindent();
        }
        else
        {
            code << " 0);";
            src_code->writeLine(code);
        }
    }

    code.clear();

    if (node->HasValue(prop_extra_style))
    {
        code << "SetExtraStyle(GetExtraStyle() | " << node->prop_as_string(prop_extra_style) << ");";
        src_code->writeLine(code);
        code.clear();
    }

    src_code->Unindent();
    src_code->writeLine();

    return true;
}

std::optional<ttlib::cstr> PanelFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_header)
    {
        ttlib::cstr code;

        code << node->get_node_name() << "(wxWindow* parent, wxWindowID id = " << node->prop_as_string(prop_id);
        code << ");\n\n";
        return code;
    }
    else if (cmd == code_after_children)
    {
        ttlib::cstr code;

        Node* panel;
        if (node->IsForm())
        {
            panel = node;
            ASSERT_MSG(panel->GetChildCount(), "Trying to generate code for a wxPanel with no children.")
            if (!panel->GetChildCount())
                return {};  // empty dialog, so nothing to do
            ASSERT_MSG(panel->GetChild(0)->IsSizer(), "Expected first child of a wxPanel to be a sizer.");
            if (panel->GetChild(0)->IsSizer())
                node = panel->GetChild(0);
        }
        else
        {
            panel = node->FindParentForm();
        }

        auto min_size = panel->prop_as_wxSize(prop_minimum_size);
        auto max_size = panel->prop_as_wxSize(prop_maximum_size);
        auto size = panel->prop_as_wxSize(prop_size);

        if (min_size == wxDefaultSize && max_size == wxDefaultSize)
        {
            code << "\tSetSizerAndFit(" << node->get_node_name() << ");";
        }
        else
        {
            code << "\tSetSizer(" << node->get_node_name() << ");";
            if (min_size != wxDefaultSize)
            {
                code << "\n\tSetMinSize(wxSize(" << min_size.GetWidth() << ", " << min_size.GetHeight() << "));";
            }
            if (max_size != wxDefaultSize)
            {
                code << "\n\tSetMaxSize(wxSize(" << max_size.GetWidth() << ", " << max_size.GetHeight() << "));";
            }
            code << "\n\tFit();";
        }

        if (size != wxDefaultSize)
        {
            code << "\n\tSetSize(wxSize(" << size.GetWidth() << ", " << size.GetHeight() << "));";
        }

        return code;
    }
    else if (cmd == code_base_class)
    {
        ttlib::cstr code;
        code << "wxPanel";
        return code;
    }

    return {};
}

std::optional<ttlib::cstr> PanelFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool PanelFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);

    return true;
}
