/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiMDIParentFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/frame.h>              // wxFrame class interface
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_aui_mdi_frame.h"

#include "code.h"
#include "gen_base.h"          // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"        // GeneratorLibrary -- Generator classes
#include "gen_frame_common.h"  // wxDocParent common generator functions
#include "gen_xrc_utils.h"     // Common XRC generating functions
#include "node.h"              // Node class
#include "node_prop.h"         // NodeProperty -- NodeProperty class
#include "utils.h"             // Utility functions that work with properties
#include "write_code.h"        // WriteCode -- Write code to Scintilla or file

bool AuiMdiFrameGenerator::ConstructionCode(Code& code)
{
    return FrameCommon::ConstructionCode(code, FrameCommon::frame_aui);
}

bool AuiMdiFrameGenerator::SettingsCode(Code& code)
{
    return FrameCommon::SettingsCode(code, FrameCommon::frame_aui);
}

bool AuiMdiFrameGenerator::AfterChildrenCode(Code& code)
{
    return FrameCommon::AfterChildrenCode(code, FrameCommon::frame_aui);
}

bool AuiMdiFrameGenerator::HeaderCode(Code& code)
{
    return FrameCommon::HeaderCode(code, FrameCommon::frame_aui);
}

bool AuiMdiFrameGenerator::BaseClassNameCode(Code& code)
{
    return FrameCommon::BaseClassNameCode(code);
}

bool AuiMdiFrameGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                       std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/aui/tabmdi.h>", set_src, set_hdr);

    return true;
}

bool AuiMdiFrameGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop,
                                               Node* node)
{
    return FrameCommon::AllowPropertyChange(event, prop, node);
}

bool AuiMdiFrameGenerator::GetImports(Node* /* node */, std::set<std::string>& /* set_imports */,
                                      GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
    }

    return false;
}

/////////////////////////////// AuiMDIChildFrame //////////////////////////////////////

bool AuiMDIChildFrame::ConstructionCode(Code& code)
{
    return FrameCommon::ConstructionCode(code, FrameCommon::frame_aui_child);
}

bool AuiMDIChildFrame::SettingsCode(Code& code)
{
    return FrameCommon::SettingsCode(code, FrameCommon::frame_aui_child);
}

bool AuiMDIChildFrame::AfterChildrenCode(Code& code)
{
    return FrameCommon::AfterChildrenCode(code, FrameCommon::frame_aui_child);
}

bool AuiMDIChildFrame::HeaderCode(Code& code)
{
    return FrameCommon::HeaderCode(code, FrameCommon::frame_aui_child);
}

bool AuiMDIChildFrame::BaseClassNameCode(Code& code)
{
    return FrameCommon::BaseClassNameCode(code);
}

bool AuiMDIChildFrame::GetIncludes(Node* node, std::set<std::string>& set_src,
                                   std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/aui/tabmdi.h>", set_src, set_hdr);

    return true;
}

bool AuiMDIChildFrame::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop,
                                           Node* node)
{
    return FrameCommon::AllowPropertyChange(event, prop, node);
}

bool AuiMDIChildFrame::GetImports(Node* /* node */, std::set<std::string>& /* set_imports */,
                                  GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
    }

    return false;
}
