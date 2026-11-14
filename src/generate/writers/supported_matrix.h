/////////////////////////////////////////////////////////////////////////////
// Purpose:   Write the full kwx capability matrix (.private/kwxSupported.md)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

// Writes the C4 capability matrix for the full generator surface to `path`.
//
// Rows are `(class, role)` pairs -- never one collapsed cell per class. The class list is the
// generator library (every generator declaration except interfaces), unioned with the classes
// named by the six descriptors and by src/generate/ffi_ui/coverage.json. Columns are the six
// C4 language states plus a ratchet cell. `---` marks a class that has no FFI path at all.
//
// The output is deterministic: it is derived only from the checked-in descriptors,
// coverage.json and the compiled generator list, and every collection is written in sorted
// order.
//
// Returns false when the matrix cannot be derived (coverage.json missing or invalid) or the file
// cannot be written. A failure is reported once through wxLogStderr.
[[nodiscard]] bool WriteSupportedMatrix(const std::string& path);
