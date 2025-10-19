/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common functions for generating Script Languages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <thread>
#include <unordered_set>

#include "../panels/base_panel.h"  // PANEL_PAGE

// Forward declarations
class Node;
class NodeEvent;
class Code;
class tt_string;

enum GenLang : std::uint16_t;

namespace ScriptCommon
{
    // Safely joins a thread with proper error handling.
    // Returns immediately if thread is not joinable.
    // Catches std::system_error and displays appropriate error message.
    void JoinThreadSafely(std::thread& thread);

    // Collects existing event handlers from a generated file.
    // Returns true if user-defined handlers were found.
    //
    // Parameters:
    //   form_node - The form node to get output path from
    //   language - The scripting language (PERL, PYTHON, RUBY)
    //   panel_type - The panel type (NOT_PANEL means it's a standalone file)
    //   code_lines - Set to store found handler definitions
    //   func_prefix - The function/method prefix ("sub ", "def ")
    [[nodiscard]] auto CollectExistingEventHandlers(Node* form_node, GenLang language,
                                                    PANEL_PAGE panel_type,
                                                    std::unordered_set<std::string>& code_lines,
                                                    std::string_view func_prefix) -> bool;

    // Generates a comment for event handlers based on whether user handlers were found.
    // For Python, adds triple-quote string start.
    //
    // Parameters:
    //   found_user_handlers - True if user has defined their own handlers
    //   code - Code object to write the comment to
    //   language - The scripting language (for Python-specific handling)
    auto GenerateEventHandlerComment(bool found_user_handlers, Code& code, GenLang language)
        -> void;

    // Generates the body of a standard event handler based on event name.
    // Handles common button click events (Close, Yes, No) and defaults to Skip/skip.
    //
    // Parameters:
    //   event - The event to generate handler body for
    //   code - Code object to write the handler body to
    //   language - The scripting language (determines syntax)
    auto GenerateEventHandlerBody(NodeEvent* event, Code& code, GenLang language) -> void;

    // Creates a path for script language generated files.
    // Returns "./" if path is empty, otherwise returns the directory path.
    //
    // Parameters:
    //   node - The node to get the form and output path from
    //   language - The scripting language
    [[nodiscard]] auto MakeScriptPath(Node* node, GenLang language) -> tt_string;

}  // namespace ScriptCommon
