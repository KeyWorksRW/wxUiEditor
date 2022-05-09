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

    m_is_popup_menu = false;
    size_t popups = 0;
    int nesting = 0;
    for (size_t pos = curTxtLine + 1; pos < txtfile.size(); ++pos)
    {
        auto menu_line = txtfile[pos].subview(txtfile[pos].find_nonspace());
        if (menu_line.empty() || menu_line.at(0) == '/')  // ignore blank lines and comments
            continue;

        if (menu_line.is_sameprefix("END") || menu_line.is_sameprefix("}"))
        {
            --nesting;
            if (nesting > 0)
                continue;
            else
                break;
        }
        else if (menu_line.is_sameprefix("BEGIN") || menu_line.is_sameprefix("{"))
        {
            ++nesting;
            continue;
        }
        if (menu_line.is_sameprefix("POPUP"))
        {
            ++popups;

            // If there is more than one POPUP or the text of the popup contains an accelerator, then assume this is a
            // wxMenuBar.

            if (popups > 1 || menu_line.contains("&"))
            {
                ++popups;  // so logic after the for loop will know this is a wxMenuBar
                break;
            }
        }
    }

    // If there was only one POPUP directive and it didn't contain an acclerator, then assume this is a popup menu

    if (popups <= 1)
        m_is_popup_menu = true;

    m_form_type = form_menu;
    m_form_node = g_NodeCreator.NewNode(m_is_popup_menu ? gen_PopupMenu : gen_MenuBar);

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    m_form_node->prop_set_value(prop_base_src_includes, ttlib::cstr() << "// " << txtfile.filename());
#endif  // _DEBUG

    ttlib::cstr value;  // General purpose string we can use throughout this function

    value = line.substr(0, end);
    m_form_node->prop_set_value(prop_class_name, ConvertFormID(value));

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    m_form_id = m_form_node->prop_as_string(prop_class_name);
#endif  // _DEBUG

    for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.is_sameprefix("BEGIN") || line.is_sameprefix("{"))
        {
            ++curTxtLine;
            ParseMenus(txtfile, curTxtLine);
            break;
        }
    }
}

void resForm::ParseMenus(ttlib::textfile& txtfile, size_t& curTxtLine)
{
    NodeSharedPtr parent = m_is_popup_menu ? m_form_node : nullptr;

    for (; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        auto line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.empty() || line.at(0) == '/')  // ignore blank lines and comments
            continue;

        if (line.is_sameprefix("END") || line.is_sameprefix("}"))
        {
            break;
        }

        if (line.is_sameprefix("BEGIN") || line.is_sameprefix("{"))
        {
            if (parent)
            {
                ++curTxtLine;
                ParseMenuItem(parent.get(), txtfile, curTxtLine);
                parent = nullptr;
            }
            continue;
        }

        if (line.is_sameprefix("POPUP") && !m_is_popup_menu)
        {
            auto& control = m_ctrls.emplace_back();
            parent = control.SetNodePtr(g_NodeCreator.NewNode(gen_wxMenu));
            m_form_node->Adopt(parent);
            line.moveto_nextword();
            parent->prop_set_value(prop_label, m_pWinResource->ConvertCodePageString(line.view_substr(0)));
        }
    }
}

void resForm::ParseMenuItem(Node* parent, ttlib::textfile& txtfile, size_t& curTxtLine)
{
    NodeSharedPtr sub_parent { nullptr };
    for (; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        auto line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.empty() || line.at(0) == '/')  // ignore blank lines and comments
            continue;

        else if (line.is_sameprefix("END") || line.is_sameprefix("}"))
        {
            break;
        }

        else if (line.is_sameprefix("BEGIN") || line.is_sameprefix("{"))
        {
            if (sub_parent)
            {
                ++curTxtLine;
                ParseMenuItem(sub_parent.get(), txtfile, curTxtLine);
                sub_parent = nullptr;
            }
            continue;
        }

        else if (line.is_sameprefix("POPUP"))
        {
            auto& control = m_ctrls.emplace_back();
            sub_parent = control.SetNodePtr(g_NodeCreator.NewNode(gen_submenu));
            parent->Adopt(sub_parent);
            line.moveto_nextword();
            sub_parent->prop_set_value(prop_label, m_pWinResource->ConvertCodePageString(line.view_substr(0)));
        }
        else if (line.is_sameprefix("MENUITEM"))
        {
            line.moveto_nextword();
            if (line.is_sameprefix("SEPARATOR"))
            {
                auto& control = m_ctrls.emplace_back();
                auto separator = control.SetNodePtr(g_NodeCreator.NewNode(gen_separator));
                parent->Adopt(separator);
            }
            else
            {
                auto& control = m_ctrls.emplace_back();
                auto item = control.SetNodePtr(g_NodeCreator.NewNode(gen_wxMenuItem));
                parent->Adopt(item);
                ttlib::sview label = line.view_substr(0);
                auto end = label.find("\\t");
                if (ttlib::is_found(end))
                {
                    item->prop_set_value(prop_label, m_pWinResource->ConvertCodePageString(label.substr(0, end)));
                    label.remove_prefix(end < label.size() ? end + 2 : end);
                    item->prop_set_value(prop_shortcut, label);
                }
                else
                {
                    item->prop_set_value(prop_label, m_pWinResource->ConvertCodePageString(label));
                }

                auto pos = line.find("\",");
                if (ttlib::is_found(pos))
                {
                    ttlib::sview id = line.subview(pos + 3);
                    id.moveto_nonspace();
                    end = id.find_first_of(',');
                    if (!ttlib::is_found(end))
                    {
                        id.trim(tt::TRIM::right);
                        item->prop_set_value(prop_id, id);
                        auto help = m_pWinResource->FindStringID(ttlib::cstr() << id);
                        if (help)
                        {
                            item->prop_set_value(prop_help, help.value());
                        }
                    }
                    else
                    {
                        ttlib::sview item_id = id.substr(0, end);
                        item_id.trim(tt::TRIM::right);
                        item->prop_set_value(prop_id, item_id);
                        id.remove_prefix(end < id.size() ? end + 1 : end);
                        id.moveto_nonspace();
                        if (id.contains("CHECKED"))
                        {
                            item->prop_set_value(prop_checked, true);
                        }
                        if (id.contains("INACTIVE"))
                        {
                            item->prop_set_value(prop_disabled, true);
                        }
                        auto help = m_pWinResource->FindStringID(ttlib::cstr() << item_id);
                        if (help)
                        {
                            item->prop_set_value(prop_help, help.value());
                        }
                    }
                }
            }
        }
    }
}
