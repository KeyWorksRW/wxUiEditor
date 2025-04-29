/////////////////////////////////////////////////////////////////////////////
// Purpose:   DocMdiParentFrameGenerator generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/docview.h>

#include "gen_doc_mdi_frame.h"

#include "code.h"              // Code -- Helper class for generating code
#include "gen_base.h"          // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"        // GeneratorLibrary -- Generator classes
#include "gen_frame_common.h"  // wxDocParent common generator functions
#include "gen_xrc_utils.h"     // Common XRC generating functions
#include "node.h"              // Node class
#include "node_prop.h"         // NodeProperty -- NodeProperty class
#include "utils.h"             // Utility functions that work with properties
#include "write_code.h"        // WriteCode -- Write code to Scintilla or file

bool DocMdiParentFrameGenerator::ConstructionCode(Code& code)
{
    return FrameCommon::ConstructionCode(code, FrameCommon::frame_mdi_doc);
}

bool DocMdiParentFrameGenerator::SettingsCode(Code& code)
{
    return FrameCommon::SettingsCode(code, FrameCommon::frame_mdi_doc);
}

bool DocMdiParentFrameGenerator::AfterChildrenCode(Code& code)
{
    return FrameCommon::AfterChildrenCode(code, FrameCommon::frame_mdi_doc);
}

bool DocMdiParentFrameGenerator::HeaderCode(Code& code)
{
    return FrameCommon::HeaderCode(code);
}

bool DocMdiParentFrameGenerator::BaseClassNameCode(Code& code)
{
    return FrameCommon::BaseClassNameCode(code);
}

bool DocMdiParentFrameGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                               GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/docview.h>", set_src, set_hdr);

    return true;
}

bool DocMdiParentFrameGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    return FrameCommon::AllowPropertyChange(event, prop, node);
}

bool DocMdiParentFrameGenerator::GetImports(Node* /* node */, std::set<std::string>& /* set_imports */, GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
    }
    return false;
}
