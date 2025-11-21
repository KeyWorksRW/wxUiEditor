# wxUiEditor Code Architecture

This document provides architectural overviews of key wxUiEditor systems, automatically extracted from AI Context comments in header files.

## Undo/Redo System

**File:** `src/undo_stack.h`

wxUiEditor implements a command pattern-based undo/redo system. The `UndoAction` abstract base class requires derived classes to implement three core methods:
- `Change()` - Applies or re-applies the action (used when pushing to undo stack and during redo)
- `Revert()` - Reverses the action (used during undo)
- `GetMemorySize()` - Returns the memory footprint of the action

The `GroupUndoActions` class allows multiple actions to be treated as a single undoable operation with a unified undo string.

The `UndoStack` manages two vectors (undo and redo stacks):
- `Push()` executes `Change()` and adds the action to the undo stack
- `Undo()` calls `Revert()` on the last action and moves it to the redo stack
- `Redo()` calls `Change()` on the last redone action and moves it back to the undo stack

The stack can be locked to execute actions without affecting undo/redo history, useful for programmatic changes that shouldn't be undoable. Actions may optionally store Node pointers and control UI selection events to maintain proper editor state during undo/redo operations.

---

*This document is automatically generated from AI Context comments in header files. To add a new section, ensure your header file has a properly formatted AI Context comment block.*
