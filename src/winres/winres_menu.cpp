/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process a Windows Resource MENU
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "winres_form.h"

#include "import_winres.h"  // WinResource -- Parse a Windows resource file
#include "node_creator.h"   // NodeCreator -- Class used to create nodes

void resForm::ParseMenu(WinResource* pWinResource, ttlib::textfile& txtfile, size_t& curTxtLine)
{
    m_pWinResource = pWinResource;
    auto line = txtfile[curTxtLine].subview();
    auto end = line.find_space();
    if (end == tt::npos)
        throw std::invalid_argument("Expected an ID then a DIALOG or DIALOGEX.");

    m_form_type = form_menu;
    m_form_node = g_NodeCreator.NewNode(gen_MenuBar);

#if defined(_DEBUG)
    m_form_node->prop_set_value(prop_base_src_includes, ttlib::cstr() << "// " << txtfile.filename());
#endif  // _DEBUG

    ttlib::cstr value;  // General purpose string we can use throughout this function

    value = line.substr(0, end);
    m_form_node->prop_set_value(prop_class_name, ConvertFormID(value));

#if defined(_DEBUG)
    m_form_id = m_form_node->prop_as_string(prop_class_name);
#endif  // _DEBUG

    for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.is_sameprefix("BEGIN") || line.is_sameprefix("{"))
        {
            ++curTxtLine;
            ParseControls(txtfile, curTxtLine);
            break;
        }
    }
}
