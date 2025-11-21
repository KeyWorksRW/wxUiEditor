# .wxui File Format Reference

This directory contains comprehensive documentation for the `.wxui` file format used by wxUiEditor.

## Purpose

This documentation enables:
- **AI-Assisted Development** - AI tools can generate correct `.wxui` files
- **Developer Onboarding** - New contributors can understand the format
- **Better Tooling** - Validators, converters, and utilities can be created
- **Migration Support** - Users can migrate from other UI designers
- **Quality Assurance** - Clear specification reduces bugs and inconsistencies

## Documentation Structure

Read the documentation in order for a comprehensive understanding:

1. [00-overview.md](00-overview.md) - High-level format description
2. [01-xml-structure.md](01-xml-structure.md) - XML schema and root structure
3. [02-widgets.md](02-widgets.md) - Widget catalog with properties
4. [03-sizers.md](03-sizers.md) - Sizer types and layout
5. [04-events.md](04-events.md) - Event binding reference
6. [05-code-generation.md](05-code-generation.md) - How .wxui maps to code
7. [06-examples.md](06-examples.md) - Complete working examples
8. [07-advanced.md](07-advanced.md) - Advanced features
9. [08-troubleshooting.md](08-troubleshooting.md) - Common issues and solutions

## Implementation Status

These files currently contain placeholder content (TODO sections) and will be progressively filled with detailed documentation based on the wxUiEditor implementation.

## Contributing

When adding documentation:
- Use clear, concise language
- Include working code examples
- Show both XML and generated code
- Cross-reference related sections
- Keep AI-assisted development use cases in mind
