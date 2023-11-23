//////////////////////////////////////////////////////////////////////////
// Purpose:   wxTimer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/timer.h>

#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_timer.h"

bool TimerGenerator::GetIncludes(Node* /* node */, std::set<std::string>& /* set_src */, std::set<std::string>& set_hdr,
                                 int /* language */)
{
    set_hdr.insert("#include <wx/timer.h>");
    return true;
}

int TimerGenerator::GetRequiredVersion(Node* /* node */)
{
    return 19;  // Introduced in version 1.2.0
}

bool TimerGenerator::StartIfChildTimer(Node* form, Code& code)
{
    if (!form->isGen(gen_wxFrame) && !form->isGen(gen_wxDialog) && !form->isGen(gen_PanelForm) &&
        !form->isGen(gen_wxMdiWindow) && !form->isGen(gen_wxAuiMDIChildFrame) && !form->isGen(gen_wxPopupTransientWindow))
        return false;

    for (auto& iter: form->getChildNodePtrs())
    {
        if (iter->isGen(gen_wxTimer))
        {
            if (iter->as_bool(prop_auto_start))
            {
                auto save_node = code.m_node;
                code.m_node = iter.get();
                if (code.is_ruby() || code.is_python())
                {
                    code.Eol().NodeName().CreateClass();
                    code.Str("self").Comma().Add(prop_id).EndFunction();
                }
                else if (code.is_cpp())
                {
                    code.Eol().NodeName().VariableMethod("SetOwner(").Str("this").Comma();
                    code.Add(prop_id).EndFunction();
                }

                code.Eol().NodeName().VariableMethod("Start(").as_string(prop_interval).Comma();
                code.TrueFalseIf(prop_one_shot).EndFunction();
                code.m_node = save_node;
                return true;
            }
            break;
        }
    }
    return false;
}

void TimerGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    BaseGenerator::GenEvent(code, event, class_name);

    // Since this is the base class, we don't want to use the pointer that GenEventCode() would normally create
    code.Replace(tt_string() << event->getNode()->as_string(prop_var_name) << "->", "");
}
