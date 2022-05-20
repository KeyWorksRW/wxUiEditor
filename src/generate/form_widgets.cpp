/////////////////////////////////////////////////////////////////////////////
// Purpose:   Form component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_base.h"     // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"   // GeneratorLibrary -- Generator classes
#include "mainapp.h"      // App -- App class
#include "node.h"         // Node class
#include "node_prop.h"    // NodeProperty -- NodeProperty class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"        // Utility functions that work with properties
#include "write_code.h"   // WriteCode -- Write code to Scintilla or file

#include "form_widgets.h"

#include "pugixml.hpp"

// Generates code for any class inheriting from wxTopLevelWindow -- this will generate
// everything needed to set the window's icon.
ttlib::cstr GenerateIconCode(const ttlib::cstr& description);

//////////////////////////////////////////  DialogFormGenerator  //////////////////////////////////////////

bool DialogFormGenerator::GenConstruction(Node* node, BaseCodeGenerator* code_gen)
{
    auto src_code = code_gen->GetSrcWriter();

    ttlib::cstr code;

    code << "bool " << node->prop_as_string(prop_class_name)
         << "::Create(wxWindow* parent, wxWindowID id, const wxString& title,\n\t\tconst wxPoint& pos, const wxSize& size, "
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
        auto icon_code = GenerateIconCode(node->prop_as_string(prop_icon));
        src_code->writeLine(icon_code, indent::auto_keep_whitespace);
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

int DialogFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    object.append_attribute("class").set_value("wxDialog");
    object.append_attribute("name").set_value(node->prop_as_string(prop_class_name).c_str());
    if (node->HasValue(prop_style))
    {
        if (add_comments && node->prop_as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            object.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }
        if (!node->HasValue(prop_extra_style))
        {
            object.append_child("style").text().set(node->prop_as_string(prop_style).c_str());
        }
        else
        {
            ttlib::cstr all_styles = node->prop_as_string(prop_style);
            all_styles << '|' << node->prop_as_string(prop_extra_style);
            object.append_child("style").text().set(all_styles.c_str());
        }
    }
    if (node->HasValue(prop_pos))
    {
        object.append_child("pos").text().set(node->prop_as_string(prop_pos).c_str());
    }
    if (node->HasValue(prop_size))
    {
        object.append_child("size").text().set(node->prop_as_string(prop_size).c_str());
    }
    if (node->HasValue(prop_title))
    {
        object.append_child("title").text().set(node->prop_as_string(prop_title).c_str());
    }
    if (node->HasValue(prop_center))
    {
        if (node->prop_as_string(prop_center).is_sameas("wxVERTICAL") ||
            node->prop_as_string(prop_center).is_sameas("wxHORIZONTAL"))
        {
            if (add_comments)
            {
                object.append_child(pugi::node_comment)
                    .set_value(
                        (ttlib::cstr(node->prop_as_string(prop_center)) << " cannot be be set in the XRC file.").c_str());
            }
            object.append_child("centered").text().set(1);
        }
        else
        {
            object.append_child("centered").text().set(node->prop_as_string(prop_center).is_sameas("no") ? 0 : 1);
        }
    }
    if (node->HasValue(prop_icon))
    {
        ttlib::multistr parts(node->prop_as_string(prop_icon), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            ttlib::multistr art_parts(parts[IndexArtID], '|');
            auto icon = object.append_child("icon");
            icon.append_attribute("stock_id").set_value(art_parts[0].c_str());
            icon.append_attribute("stock_client").set_value(art_parts[1].c_str());
        }
        else
        {
            // REVIEW: [KeyWorks - 05-13-2022] As of wxWidgets 3.1.6, SVG files do not work here
            object.append_child("icon").text().set(parts[IndexImage].c_str());
        }
    }
    if (add_comments)
    {
        GenXrcComments(node, object);
        if (node->prop_as_bool(prop_persist))
        {
            object.append_child(pugi::node_comment).set_value(" persist is not supported in the XRC file. ");
        }
    }

    return xrc_updated;
}

void DialogFormGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxDialogXmlHandler");
    if (node->HasValue(prop_icon))
    {
        handlers.emplace("wxIconXmlHandler");
        handlers.emplace("wxBitmapXmlHandler");
    }
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
            dlg = node->get_form();
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

int FrameFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    object.append_attribute("class").set_value("wxFrame");
    object.append_attribute("name").set_value(node->prop_as_string(prop_class_name).c_str());
    if (node->HasValue(prop_style))
    {
        if (add_comments && node->prop_as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            object.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }
        if (!node->HasValue(prop_extra_style))
        {
            object.append_child("style").text().set(node->prop_as_string(prop_style).c_str());
        }
        else
        {
            ttlib::cstr all_styles = node->prop_as_string(prop_style);
            all_styles << '|' << node->prop_as_string(prop_extra_style);
            object.append_child("style").text().set(all_styles.c_str());
        }
    }
    if (node->HasValue(prop_pos))
    {
        object.append_child("pos").text().set(node->prop_as_string(prop_pos).c_str());
    }
    if (node->HasValue(prop_size))
    {
        object.append_child("size").text().set(node->prop_as_string(prop_size).c_str());
    }
    if (node->HasValue(prop_title))
    {
        object.append_child("title").text().set(node->prop_as_string(prop_title).c_str());
    }
    if (node->HasValue(prop_center))
    {
        if (node->prop_as_string(prop_center).is_sameas("wxVERTICAL") ||
            node->prop_as_string(prop_center).is_sameas("wxHORIZONTAL"))
        {
            if (add_comments)
            {
                object.append_child(pugi::node_comment)
                    .set_value(
                        (ttlib::cstr(node->prop_as_string(prop_center)) << " cannot be be set in the XRC file.").c_str());
            }
            object.append_child("centered").text().set(1);
        }
        else
        {
            object.append_child("centered").text().set(node->prop_as_string(prop_center).is_sameas("no") ? 0 : 1);
        }
    }
    if (node->HasValue(prop_icon))
    {
        ttlib::multistr parts(node->prop_as_string(prop_icon), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            ttlib::multistr art_parts(parts[IndexArtID], '|');
            auto icon = object.append_child("icon");
            icon.append_attribute("stock_id").set_value(art_parts[0].c_str());
            icon.append_attribute("stock_client").set_value(art_parts[1].c_str());
        }
        else
        {
            // REVIEW: [KeyWorks - 05-13-2022] As of wxWidgets 3.1.6, SVG files do not work here
            object.append_child("icon").text().set(parts[IndexImage].c_str());
        }
    }
    if (add_comments)
    {
        GenXrcComments(node, object);
        if (node->prop_as_bool(prop_persist))
        {
            object.append_child(pugi::node_comment).set_value(" persist is not supported in the XRC file. ");
        }
    }

    return xrc_updated;
}

void FrameFormGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxFrameXmlHandler");
    if (node->HasValue(prop_icon))
    {
        handlers.emplace("wxIconXmlHandler");
        handlers.emplace("wxBitmapXmlHandler");
    }
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
    if (auto code = GenerateIconCode(node->prop_as_string(prop_icon)); code.size())
    {
        code << GenFormSettings(node);
        return code;
    }

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
                event->SetValidationFailureMessage(
                    "You can't add a context help button if there is a minimize or maximize button "
                    "(wxDEFAULT_FRAME_STYLE contains these).");
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
    code << "bool " << node->prop_as_string(prop_class_name) << "::Create";

    code << "(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString "
            "&name)\n{";
    code << "\n\tif (!wxPanel::Create(parent, id, pos, size, style, name))\n\t\treturn false;\n\n";

    src_code->writeLine(code, indent::none);
    code.clear();
    src_code->Indent();

    if (node->HasValue(prop_extra_style))
    {
        code << "SetExtraStyle(GetExtraStyle() | " << node->prop_as_string(prop_extra_style) << ");";
        src_code->writeLine(code);
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

std::optional<ttlib::cstr> PanelFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_header)
    {
        ttlib::cstr code;

        code << node->get_node_name() << "() {}\n";

        code << node->get_node_name() << "(wxWindow* parent, wxWindowID id = " << node->prop_as_string(prop_id) << ", ";
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
            code << "wxTAB_TRAVERSAL";

        code << ", const wxString &name = ";
        if (node->HasValue(prop_window_name))
            code << GenerateQuotedString(node, prop_window_name);
        else
            code << "wxPanelNameStr";

        code << ")\n{\n\tCreate(parent, id, pos, size, style, name);\n}\n";

        code << "\nbool Create(wxWindow *parent, ";
        code << "wxWindowID id = " << node->prop_as_string(prop_id) << ", ";
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
            code << "wxTAB_TRAVERSAL";

        code << ", const wxString &name = ";
        if (node->HasValue(prop_window_name))
            code << GenerateQuotedString(node, prop_window_name);
        else
            code << "wxPanelNameStr";

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
            panel = node->get_form();
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

//////////////////////////////////////////  Utility Code  //////////////////////////////////////////

// Generates code for any class inheriting from wxTopLevelWindow -- this will generate everything needed to set the
// window's icon.

ttlib::cstr GenerateIconCode(const ttlib::cstr& description)
{
    ttlib::cstr code;

    if (description.empty())
    {
        return code;
    }

    ttlib::multiview parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts.size() < 2 || parts[IndexImage].empty())
    {
        return code;
    }

    if (parts[IndexType].is_sameas("XPM"))
    {
        // In theory, we could create an alpha channel using black as the transparency, but it just doesn't make sense
        // for the user to be using XPM files as an icon.
        code << "// XPM files do not contain an alpha channel and cannot be used as an icon.";
        return code;
    }

    if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1" && !parts[IndexType].is_sameas("SVG"))
    {
        code << "#if wxCHECK_VERSION(3, 1, 6)\n";
    }

    if (parts[IndexType].contains("Art"))
    {
        ttlib::cstr art_id(parts[IndexArtID]);
        ttlib::cstr art_client;
        if (auto pos = art_id.find('|'); ttlib::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code << "SetIcon(wxArtProvider::GetBitmapBundle(" << art_id << ", ";
        // Note that current documentation states that the client is required, but the header file says otherwise
        if (art_client.size())
            code << art_client;
        code << ").GetIconFor(this));\n";
    }
    else if (description.starts_with("SVG"))
    {
        auto embed = wxGetApp().GetProjectSettings()->GetEmbeddedImage(parts[IndexImage]);
        if (!embed)
        {
            FAIL_MSG(ttlib::cstr() << description << " not embedded!")
            return code;
        }

        auto svg_size = get_image_prop_size(parts[IndexSize]);

        ttlib::cstr name = "wxue_img::" + embed->array_name;
        code << "SetIcon(wxueBundleSVG(" << name << ", " << (embed->array_size & 0xFFFFFFFF) << ", ";
        code << (embed->array_size >> 32) << ", wxSize(" << svg_size.x << ", " << svg_size.y << "))";
        code << ".GetIconFor(this));\n";
        return code;
    }
    else
    {
        if (auto bundle = wxGetApp().GetProjectSettings()->GetPropertyImageBundle(description); bundle)
        {
            if (bundle->lst_filenames.size() == 1)
            {
                code << "SetIcon(wxBitmapBundle::FromBitmap(wxueImage(";
                ttlib::cstr name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                name.Replace(".", "_", true);  // fix wxFormBuilder header files

                if (parts[IndexType].starts_with("Embed"))
                {
                    auto embed = wxGetApp().GetProjectSettings()->GetEmbeddedImage(bundle->lst_filenames[0]);
                    if (embed)
                    {
                        name = "wxue_img::" + embed->array_name;
                    }
                }

                code << name << ", sizeof(" << name << ")))";
                code << ".GetIconFor(this));\n";
            }
            else
            {
                code << "{\n\twxIconBundle icon_bundle;\n\twxIcon icon;\n";
                for (auto& iter: bundle->lst_filenames)
                {
                    ttlib::cstr name(iter.filename());
                    name.remove_extension();
                    name.Replace(".", "_", true);  // fix wxFormBuilder header files
                    if (parts[IndexType].starts_with("Embed"))
                    {
                        auto embed = wxGetApp().GetProjectSettings()->GetEmbeddedImage(iter);
                        if (embed)
                        {
                            name = "wxue_img::" + embed->array_name;
                        }
                    }
                    code << "\ticon.CopyFromBitmap(wxueImage(" << name << ", sizeof(" << name << ")));\n";
                    code << "\ticon_bundle.AddIcon(icon);\n";
                }
                code << "\tSetIcons(icon_bundle);\n}\n";
            }
        }
        else
        {
            FAIL_MSG(ttlib::cstr(description) << " was not converted to a bundle ahead of time!")
            return code;
        }
    }

    if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
    {
        code << "#else\n";
        auto image_code = GenerateBitmapCode(description);
        if (!image_code.contains(".Scale") && image_code.starts_with("wxImage("))
        {
            code << "SetIcon(wxIcon(" << image_code.subview(sizeof("wxImage")) << ");\n";
        }
        else
        {
            code << "{\n";
            code << "\twxIcon icon;\n";
            code << "\ticon.CopyFromBitmap(" << GenerateBitmapCode(description) << ");\n";
            code << "\tSetIcon(wxIcon(icon));\n";
            code << "}\n";
        }
        code << "#endif\n";
    }

    return code;
}
