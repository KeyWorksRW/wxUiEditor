/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDocParentFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/docview.h>

#include "gen_doc_parent_frame.h"

#include "code.h"              // Code -- Helper class for generating code
#include "gen_base.h"          // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"        // GeneratorLibrary -- Generator classes
#include "gen_frame_common.h"  // wxFrame and derivative common generator functions
#include "gen_xrc_utils.h"     // Common XRC generating functions
#include "node.h"              // Node class
#include "node_prop.h"         // NodeProperty -- NodeProperty class
#include "utils.h"             // Utility functions that work with properties
#include "write_code.h"        // WriteCode -- Write code to Scintilla or file

bool DocParentFrameGenerator::ConstructionCode(Code& code)
{
    return FrameCommon::ConstructionCode(code, FrameCommon::frame_sdi_doc);
}

bool DocParentFrameGenerator::SettingsCode(Code& code)
{
    return FrameCommon::SettingsCode(code, FrameCommon::frame_sdi_doc);
}

bool DocParentFrameGenerator::AfterChildrenCode(Code& code)
{
    return FrameCommon::AfterChildrenCode(code, FrameCommon::frame_sdi_doc);
}

bool DocParentFrameGenerator::HeaderCode(Code& code)
{
    return FrameCommon::HeaderCode(code, FrameCommon::frame_sdi_doc);
}

bool DocParentFrameGenerator::BaseClassNameCode(Code& code)
{
    return FrameCommon::BaseClassNameCode(code);
}

bool DocParentFrameGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                          std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/docview.h>", set_src, set_hdr);

    return true;
}

bool DocParentFrameGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop,
                                                  Node* node)
{
    return FrameCommon::AllowPropertyChange(event, prop, node);
}

bool DocParentFrameGenerator::GetImports(Node* /* node */, std::set<std::string>& /* set_imports */,
                                         GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
    }
    return false;
}

/////////////////////////////// DocChildFrame //////////////////////////////////////

bool DocChildFrame::ConstructionCode(Code& code)
{
    return FrameCommon::ConstructionCode(code, FrameCommon::frame_sdi_child);
}

bool DocChildFrame::SettingsCode(Code& code)
{
    return FrameCommon::SettingsCode(code, FrameCommon::frame_sdi_child);
}

bool DocChildFrame::AfterChildrenCode(Code& code)
{
    return FrameCommon::AfterChildrenCode(code, FrameCommon::frame_sdi_child);
}

bool DocChildFrame::HeaderCode(Code& code)
{
    return FrameCommon::HeaderCode(code, FrameCommon::frame_sdi_child);
}

bool DocChildFrame::BaseClassNameCode(Code& code)
{
    return FrameCommon::BaseClassNameCode(code);
}

bool DocChildFrame::GetIncludes(Node* node, std::set<std::string>& set_src,
                                std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/docview.h>", set_src, set_hdr);

    return true;
}

bool DocChildFrame::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    return FrameCommon::AllowPropertyChange(event, prop, node);
}

bool DocChildFrame::GetImports(Node* /* node */, std::set<std::string>& /* set_imports */,
                               GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
    }
    return false;
}
