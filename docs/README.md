# wxUiEditor Documentation

This directory contains all documentation for wxUiEditor, organized by audience.

## Documentation Structure

### [ai-context/](ai-context/)
**For: AI agents and automated tools**

Formal specifications, schemas, and reference documentation designed for machine parsing:
- `.wxui` file format specification
- API schemas and data structures
- Code generation mappings

AI agents should focus primarily on this directory for accurate, structured information.

### [architecture/](architecture/)
**For: Project successors and maintainers**

Technical documentation for anyone taking over maintenance of wxUiEditor:
- [SUCCESSOR.md](architecture/SUCCESSOR.md) - Essential information for new maintainers
- Build instructions and release procedures
- Architecture and internal design
- Testing procedures

### [users/](users/)
**For: Developers using wxUiEditor to build applications**

End-user documentation including:
- Feature guides and tutorials
- Import/export documentation
- Image handling
- Tool migration guides

## Quick Links

- **Taking over this project?**: [architecture/SUCCESSOR.md](architecture/SUCCESSOR.md)
- **Architecture Overview**: [architecture/Overview.md](architecture/Overview.md)
- **Building wxUiEditor**: [architecture/build_notes.md](architecture/build_notes.md)
- **.wxui File Format**: [ai-context/wxui-format/](ai-context/wxui-format/)
- **Working with Images**: [users/images.md](users/images.md)

## Legacy

The `dev_docs/` directory at the project root has been deprecated. All content has been reorganized into this structure.
