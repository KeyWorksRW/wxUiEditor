# AI Context Documentation

This directory contains **specification and reference documentation** designed for AI agents and tools to parse and understand the wxUiEditor system.

## Purpose

Documentation in this directory is structured to help AI:
- Generate valid `.wxui` project files
- Understand wxUiEditor's internal architecture
- Generate code that works with wxUiEditor's output
- Validate and troubleshoot wxUiEditor projects

## Contents

### [wxui-format/](wxui-format/)
Complete specification of the `.wxui` XML file format including:
- XML schema and structure
- Widget catalog and properties
- Sizer types and layout rules
- Event binding syntax
- Code generation mappings
- Working examples

This is the primary resource for AI assistants helping users create or modify `.wxui` files.

## For Human Developers

- **Contributors**: See [../contributors/](../contributors/) for development documentation
- **Users**: See [../users/](../users/) for end-user documentation

## Distinction from Other Documentation

| Directory | Purpose | Audience |
|-----------|---------|----------|
| `ai-context/` | Formal specifications, schemas, reference data | AI agents & tools |
| `contributors/` | Development practices, architecture, build process | wxUiEditor contributors |
| `users/` | Usage guides, tutorials, feature documentation | wxUiEditor users |

AI agents should primarily focus on parsing `ai-context/` documentation. Contributor and user documentation may contain helpful context but is not designed as machine-readable specifications.
