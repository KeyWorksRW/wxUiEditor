# .wxui File Format Overview

This document provides a high-level overview of the `.wxui` file format used by wxUiEditor.

## Purpose

The `.wxui` format is an XML-based project file that defines wxWidgets user interface layouts. wxUiEditor reads these files and generates code in multiple languages (C++, Python, Ruby, Perl).

## Basic Structure

A `.wxui` file contains:
- Project metadata (languages, preferences)
- Widget definitions (buttons, text controls, etc.)
- Layout information (sizers, positioning)
- Event handler bindings
- Property specifications

## File Extension

All wxUiEditor project files use the `.wxui` extension.

## Further Reading

- [XML Structure](01-xml-structure.md) - Detailed XML schema
- [Widgets](02-widgets.md) - Widget catalog and properties
- [Sizers](03-sizers.md) - Layout management
- [Events](04-events.md) - Event handler bindings
- [Code Generation](05-code-generation.md) - How .wxui maps to code
- [Examples](06-examples.md) - Complete working examples
- [Advanced Features](07-advanced.md) - Inheritance, custom widgets
- [Troubleshooting](08-troubleshooting.md) - Common issues and solutions
