# AI Context Documentation Template

## Purpose
AI Context comments provide concise architectural overview for AI agents while serving as source material for contributor documentation.

## Format

### Header File AI Context Block

Place immediately after the license header, before `#pragma once`:

```cpp
/////////////////////////////////////////////////////////////////////////////
// Purpose:   Brief description of the file's purpose
// Author:    Author Name
// Copyright: Copyright (c) YYYY KeyWorks Software (Author Name)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: [One-sentence summary of what this file implements]
// [Detailed explanation covering:
//  - Design patterns used (if applicable)
//  - Key classes/functions and their responsibilities
//  - How components interact
//  - Important behavioral details or constraints
//  - Relationships to other parts of the system]

#pragma once
```

### Guidelines

**One-sentence summary:**
- Should complete: "This file implements..."
- Examples:
  - "a command pattern-based undo/redo system for wxUiEditor"
  - "the node property system for storing and accessing widget properties"

**Detailed explanation:**
- Focus on architecture and design decisions
- Explain class relationships and responsibilities
- Describe the flow of operations
- Mention important behavioral characteristics
- Keep concise but complete (3-8 lines typical)

**What to include:**
- Design patterns (Command, Factory, Observer, etc.)
- Core class responsibilities
- Key method purposes and interactions
- Important constraints or assumptions
- Memory management approach (if relevant)

**What to omit:**
- Implementation details visible in code
- API documentation (use inline comments)
- Change history (use git)
- TODO items

## Example: Undo/Redo System

```cpp
// AI Context: This file implements a command pattern-based undo/redo system for wxUiEditor.
// UndoAction is an abstract base class requiring derived classes to implement Change()
// (apply/redo), Revert() (undo), and GetMemorySize(). GroupUndoActions allows multiple actions to
// be treated as a single undoable operation. UndoStack manages two vectors (undo/redo stacks) where
// Push() executes Change() and adds to undo stack, Undo() calls Revert() and moves to redo stack,
// and Redo() calls Change() and moves back to undo stack. The stack can be locked to execute
// actions without affecting undo/redo history. Actions may optionally store Node pointers and
// control UI selection events.
```

## Extraction to Contributor Documentation

AI Context blocks are automatically extracted to `docs/contributors/architecture.md`:

1. **One-sentence summary** becomes the section introduction
2. **Detailed explanation** is reformatted as readable paragraphs
3. **File path** is included for reference
4. Sections are organized by subsystem

## Agent Instructions for Adding AI Context

When adding AI Context comments to a header file:

1. **Read the entire header file** to understand:
   - Class hierarchy and relationships
   - Key methods and their purposes
   - Design patterns in use
   - How the code fits into the larger system

2. **Generate the AI Context comment** following the template above

3. **Generate/update contributor documentation** in `docs/contributors/architecture.md`:
   - Add a new section with the file path
   - Convert the AI Context to readable paragraphs
   - Organize by logical subsystem if multiple related files

4. **Verify accuracy** - AI Context should be:
   - Accurate representation of the code
   - Useful for both AI agents and human contributors
   - Focused on architecture, not implementation details

## Benefits

**For AI Agents:**
- Quick context for code generation
- Understanding of design patterns and constraints
- Ability to maintain architectural consistency

**For Contributors:**
- High-level understanding without reading implementation
- Documentation of design decisions
- Guide to system architecture
- Easy to maintain (lives with the code)

**For Maintenance:**
- Single source of truth (in code comments)
- Automatically extracted to documentation
- Updated when code changes
- Version controlled with code
