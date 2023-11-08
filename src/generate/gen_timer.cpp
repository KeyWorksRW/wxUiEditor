//////////////////////////////////////////////////////////////////////////
// Purpose:   wxTimer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/timer.h>

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_timer.h"

bool TimerGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags();

    return true;
}

bool TimerGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_auto_start))
        code.NodeName().Function("Start(").as_string(prop_interval).TrueFalseIf(prop_one_shot).EndFunction();
    return true;
}

bool TimerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/timer.h>", set_src, set_hdr);
    return true;
}

int TimerGenerator::GetRequiredVersion(Node* node)
{
    return 19;  // Introduced in version 1.2.0
}
