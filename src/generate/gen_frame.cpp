/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/frame.h>              // wxFrame class interface
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_frame.h"

#include "code.h"
#include "gen_base.h"          // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"        // GeneratorLibrary -- Generator classes
#include "gen_frame_common.h"  // wxFrame and derivative common generator functions
#include "gen_xrc_utils.h"     // Common XRC generating functions
#include "node.h"              // Node class
#include "node_prop.h"         // NodeProperty -- NodeProperty class
#include "pugixml.hpp"         // xml_object_range
#include "utils.h"             // Utility functions that work with properties
#include "write_code.h"        // WriteCode -- Write code to Scintilla or file

bool FrameFormGenerator::ConstructionCode(Code& code)
{
    return FrameCommon::ConstructionCode(code, FrameCommon::frame_normal);
}

bool FrameFormGenerator::SettingsCode(Code& code)
{
    return FrameCommon::SettingsCode(code, FrameCommon::frame_normal);
}

bool FrameFormGenerator::AfterChildrenCode(Code& code)
{
    return FrameCommon::AfterChildrenCode(code, FrameCommon::frame_normal);
}

bool FrameFormGenerator::HeaderCode(Code& code)
{
    return FrameCommon::HeaderCode(code, FrameCommon::frame_normal);
}

bool FrameFormGenerator::BaseClassNameCode(Code& code)
{
    return FrameCommon::BaseClassNameCode(code);
}

int FrameFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    // We use item so that the macros in base_generator.h work, and the code looks the same as other
    // widget XRC generatorsl
    auto item = object;

    GenXrcObjectAttributes(node, item, "wxFrame");
    if (!node->is_PropValue(prop_variant, "normal"))
    {
        ADD_ITEM_PROP(prop_variant, "variant")
    }
    ADD_ITEM_PROP(prop_title, "title")

    if (node->HasValue(prop_center))
    {
        if (node->as_string(prop_center) == "wxVERTICAL" ||
            node->as_string(prop_center) == "wxHORIZONTAL" ||
            node->as_string(prop_center) == "wxBOTH")
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
        tt_string_vector parts(node->as_string(prop_icon), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            tt_string_vector art_parts(parts[IndexArtID], '|');
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

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, object);

        if (node->as_string(prop_center) == "wxVERTICAL")
        {
            object.append_child(pugi::node_comment)
                .set_value((" For centering, you cannot set only one direction in the XRC file "
                            "(set wxBOTH instead)."));
        }

        if (node->as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            object.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }

        if (node->as_bool(prop_persist))
        {
            object.append_child(pugi::node_comment)
                .set_value(" persist is not supported in the XRC file. ");
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

bool FrameFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                     std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/frame.h>", set_src, set_hdr);

    return true;
}

bool FrameFormGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop,
                                             Node* node)
{
    return FrameCommon::AllowPropertyChange(event, prop, node);
}

bool FrameFormGenerator::GetImports(Node* node, std::set<std::string>& set_imports,
                                    GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use base qw[Wx::Frame];");
        set_imports.emplace("use Wx qw[:frame];");
        set_imports.emplace("use Wx qw[:misc];");  // for wxDefaultPosition and wxDefaultSize

        if (auto qw_events = GatherPerlNodeEvents(node); qw_events.size())
        {
            set_imports.emplace(qw_events);
        }

        return true;
    }

    return false;
}
