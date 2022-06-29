/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/frame.h>              // wxFrame class interface
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "node.h"           // Node class
#include "node_prop.h"      // NodeProperty -- NodeProperty class
#include "project_class.h"  // Project class
#include "pugixml.hpp"      // xml_object_range
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_frame.h"

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
    object.append_attribute("name").set_value(node->prop_as_string(prop_class_name));

    if (node->HasValue(prop_title))
    {
        object.append_child("title").text().set(node->prop_as_string(prop_title));
    }
    if (node->HasValue(prop_center))
    {
        if (node->prop_as_string(prop_center) == "wxVERTICAL" || node->prop_as_string(prop_center) == "wxHORIZONTAL" ||
            node->prop_as_string(prop_center) == "wxBOTH")
        {
            object.append_child("centered").text().set(1);
        }
        else
        {
            object.append_child("centered").text().set(0);
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
            icon.append_attribute("stock_id").set_value(art_parts[0]);
            icon.append_attribute("stock_client").set_value(art_parts[1]);
        }
        else
        {
            // REVIEW: [KeyWorks - 05-13-2022] As of wxWidgets 3.1.6, SVG files do not work here
            object.append_child("icon").text().set(parts[IndexImage]);
        }
    }

    GenXrcWindowSettings(node, object);

    if (add_comments)
    {
        GenXrcComments(node, object);

        if (node->prop_as_string(prop_center) == "wxVERTICAL")
        {
            object.append_child(pugi::node_comment)
                .set_value((" For centering, you cannot set only one direction in the XRC file (set wxBOTH instead)."));
        }

        if (node->prop_as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            object.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }

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
