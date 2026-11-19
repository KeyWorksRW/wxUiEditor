/////////////////////////////////////////////////////////////////////////////
// Purpose:   Write the kwx conformance kit (one minimal .wxui per generator class)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

// Writes one minimal `<class>.wxui` per generator declaration in `directory`, plus `kit.json`.
//
// The kit is derived only from the compiled generator library: the parent chain of a class is
// discovered through NodeDeclaration::get_AllowableChildren(), so no form is hand-authored and
// the same library always produces the same files. Each file puts the class under a form (the
// class itself when the Project node allows it as a form), sets representative properties that
// the declaration actually has, and adds one child of a type the class allows.
//
// Classes that no form can reach (`sizeritem`, `folder`, `proportion`, ...) are not written; they
// are listed in `kit.json` under `omitted` with the reason `no-parent-path`. Files in `directory`
// that the current run did not generate are deleted, so the directory is always exactly the kit.
//
// Returns false when a file cannot be written. A failure is reported once through wxLogStderr.
[[nodiscard]] bool WriteConformanceKit(const std::string& directory);
