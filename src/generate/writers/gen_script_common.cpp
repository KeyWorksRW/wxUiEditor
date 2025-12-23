/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common functions for generating Script Languages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <wx/msgdlg.h>

#include "gen_script_common.h"  // Common functions for generating Script Languages

#include "code.h"             // Code -- Helper class for generating code
#include "comment_blocks.h"   // Comment block generators
#include "common_strings.h"   // Common strings used in code generation
#include "node.h"             // Node class
#include "node_event.h"       // NodeEvent -- NodeEvent class
#include "project_handler.h"  // ProjectHandler class
#include "tt_view_vector.h"   // tt_view_vector -- Read/Write line-oriented strings/files

namespace ScriptCommon
{
    void JoinThreadSafely(std::thread& thread)
    {
        if (!thread.joinable())
        {
            return;
        }

        try
        {
            thread.join();
        }
        catch (const std::system_error& err)
        {
#ifdef _DEBUG
            MSG_ERROR(err.what());
#else
            wxMessageDialog dlg_error(nullptr, wxString::FromUTF8(err.what()),
                                      "Internal Thread Error", wxICON_ERROR | wxOK);
            dlg_error.ShowModal();
#endif  // _DEBUG
        }
    }

    auto CollectExistingEventHandlers(Node* form_node, GenLang language, PANEL_PAGE panel_type,
                                      std::unordered_set<std::string>& code_lines,
                                      std::string_view func_prefix) -> bool
    {
        if (panel_type != PANEL_PAGE::NOT_PANEL)
        {
            return false;
        }

        tt_view_vector org_file;
        auto [path, has_base_file] = Project.GetOutputPath(form_node, language);

        if (has_base_file && path.extension().empty())
        {
            // Add appropriate extension based on language
            switch (language)
            {
                case GEN_LANG_PERL:
                    path += ".pl";
                    break;
                case GEN_LANG_PYTHON:
                    path += ".py";
                    break;
                case GEN_LANG_RUBY:
                    path += ".rb";
                    break;
                default:
                    return false;
            }
        }

        // If the user has defined any event handlers, add them to the code_lines set so we
        // don't generate them again.
        if (!has_base_file || !org_file.ReadFile(path))
        {
            return false;
        }

        // Get the appropriate end comment line based on language
        std::string_view end_comment_line;
        switch (language)
        {
            case GEN_LANG_PERL:
                end_comment_line = GetPerlEndCommentLine();
                break;
            case GEN_LANG_PYTHON:
                end_comment_line = GetPythonEndCommentLine();
                break;
            case GEN_LANG_RUBY:
                end_comment_line = GetRubyEndCommentLine();
                break;
            default:
                return false;
        }

        bool found_user_handlers = false;
        size_t line_index = 0;
        for (; line_index < org_file.size(); ++line_index)
        {
            if (org_file[line_index].is_sameprefix(end_comment_line))
            {
                break;
            }
        }

        for (++line_index; line_index < org_file.size(); ++line_index)
        {
            auto handler = org_file[line_index].view_nonspace();
            if (handler.starts_with(func_prefix))
            {
                code_lines.emplace(handler);
                found_user_handlers = true;
            }
        }

        return found_user_handlers;
    }

    auto GenerateEventHandlerComment(bool found_user_handlers, Code& code, GenLang language) -> void
    {
        if (found_user_handlers)
        {
            code.Str("# Unimplemented Event handler functions\n# Copy any listed and paste them "
                     "below the comment block, or "
                     "to your inherited class.");
        }
        else
        {
            code.Str("# Event handler functions\n# Add these below the comment block, or to your "
                     "inherited class.");
        }

        // Python needs triple-quote string start, Ruby needs extra newlines
        if (language == GEN_LANG_PYTHON)
        {
            code.Eol().Str(python_triple_quote).Eol();
        }
        else if (language == GEN_LANG_RUBY)
        {
            code.Eol().Eol();
        }
    }

    auto GenerateEventHandlerBody(NodeEvent* event, Code& code, GenLang language) -> void
    {
        if (event->get_name() == "CloseButtonClicked")
        {
            switch (language)
            {
                case GEN_LANG_PERL:
                    code.Tab().Str("$self->EndModal(wxID_CLOSE);");
                    break;
                case GEN_LANG_PYTHON:
                    code.Tab().Str("self.EndModal(wx.ID_CLOSE)").Eol().Eol();
                    return;  // Python adds extra newline
                case GEN_LANG_RUBY:
                    code.Tab().Str("end_modal(Wx::ID_CLOSE)");
                    break;
                default:
                    break;
            }
        }
        else if (event->get_name() == "YesButtonClicked")
        {
            switch (language)
            {
                case GEN_LANG_PERL:
                    code.Tab().Str("$self->EndModal(wxID_YES);");
                    break;
                case GEN_LANG_PYTHON:
                    code.Tab().Str("self.EndModal(wx.ID_YES)").Eol().Eol();
                    return;  // Python adds extra newline
                case GEN_LANG_RUBY:
                    code.Tab().Str("end_modal(Wx::ID_YES)");
                    break;
                default:
                    break;
            }
        }
        else if (event->get_name() == "NoButtonClicked")
        {
            switch (language)
            {
                case GEN_LANG_PERL:
                    code.Tab().Str("$self->EndModal(wxID_NO);");
                    break;
                case GEN_LANG_PYTHON:
                    code.Tab().Str("self.EndModal(wx.ID_NO)").Eol().Eol();
                    return;  // Python adds extra newline
                case GEN_LANG_RUBY:
                    code.Tab().Str("end_modal(Wx::ID_NO)");
                    break;
                default:
                    break;
            }
        }
        else
        {
            // Default event handler - call Skip/skip
            switch (language)
            {
                case GEN_LANG_PERL:
                    code.Tab().Str("$event->Skip();");
                    break;
                case GEN_LANG_PYTHON:
                    code.Tab().Str("event.Skip()").Eol().Eol();
                    return;  // Python adds extra newline
                case GEN_LANG_RUBY:
                    code.Tab().Str("event.skip");
                    break;
                default:
                    break;
            }
        }
    }

    auto MakeScriptPath(Node* node, GenLang language) -> tt_string
    {
        auto [path, has_base_file] = Project.GetOutputPath(node->get_Form(), language);

        if (path.empty())
        {
            path = "./";
        }
        else if (has_base_file)
        {
            path.remove_filename();
        }
        return path;
    }

}  // namespace ScriptCommon
