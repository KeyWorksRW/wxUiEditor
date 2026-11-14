/////////////////////////////////////////////////////////////////////////////
// Purpose:   Emit the body of a kwxFFI form from a schema-1 descriptor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "gen_enums.h"  // GenLang

class Code;
class Node;
class WriteCode;

// Identifier spellings derived from the form's output filename. `quick_mainframe` gives
// snake "quick_main_frame", pascal "QuickMainFrame", camel "quickMainFrame"; `quick_dlg` gives
// snake "quick_dialog", pascal "QuickDialog" (the C3 acceptance spelling).
struct FfiFormInfo
{
    std::string basename;  // raw output basename, e.g. quick_mainframe
    std::string snake;     // quick_main_frame
    std::string pascal_name;  // QuickMainFrame
    std::string camel;     // quickMainFrame
};

// Derives the C3 names from the output path configured for the form. Falls back to the form's
// class name when the project has no per-language file property yet.
[[nodiscard]] FfiFormInfo GetFfiFormInfo(Node* form, GenLang language);

// One (node, property, role) record for `<output>.map.json`.
struct FfiSourceMapEntry
{
    int line {};
    std::string node_class;
    std::string var_name;
    std::string property;
    std::string role;
};

// Emits the construction statements for `form` -- form creation, children, sizer attachment,
// std-button sizer, font, embedded bitmap and close-event handler. It never emits an
// application entry, an event loop or SetTopWindow (C3). Every call goes through the Code
// descriptor primitives, so an unresolved role produces one diagnostic and no line.
//
// `form_receiver` is the expression that holds the created form (for example "state.frame" for
// Go or "frame" for TypeScript). `parent_expr` names the expression the form constructor takes
// as its parent; Fortran passes a derived-type handle rather than the `parent` c_ptr argument.
// `declarations` receives Fortran type declarations; `handlers` receives handler definitions
// that must be written outside the entry function.
[[nodiscard]] bool EmitFfiFormBody(Code& code, WriteCode* writer, Node* form,
                                   const FfiFormInfo& info, const std::string& form_receiver,
                                   std::string& declarations, std::string& handlers,
                                   const std::string& parent_expr = "parent");

// The derived-type name a Fortran local uses for `wx_class`, from the descriptor's `classes`
// entry (`wxDialog` -> `wxdialog_t`). Classes the descriptor does not publish (wxFont) follow
// the same documented `<lowercase class>_t` rule.
[[nodiscard]] std::string GetFortranLocalType(GenLang language, std::string_view wx_class);

// Module names a generated Fortran form module must `use`: kwx_types, kwxffi, one module per
// descriptor class, and one module per class that owns a role's procedure. Sorted and unique.
[[nodiscard]] std::vector<std::string> GetFortranUseModules(GenLang language);

// Source-map records for the last EmitFfiFormBody() call in this process.
[[nodiscard]] const std::vector<FfiSourceMapEntry>& GetFfiSourceMapEntries();
void ResetFfiSourceMap();

// Writes `<output>.map.json` when WXUE_FFI_SOURCEMAP=1. No-op otherwise.
void WriteFfiSourceMap(const std::string& output_path);
