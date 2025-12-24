# Nodes

The Node system is the core data model of wxUiEditor, representing all UI components in a hierarchical tree structure. Nodes are displayed to the user in a TreeView control, with each node representing a Project file, a Folder or subfolder, or a wxWidgets control (sizers, bitmaps, buttons, text controls, etc.).

## Class Hierarchy Overview

```
NodeCreator (Singleton Factory)
    │
    ├── Creates ──► Node (Runtime Instance)
    │                  │
    │                  ├── References ──► NodeDeclaration (Immutable Metadata)
    │                  │                      │
    │                  │                      ├── Contains ──► PropDeclaration (Property Definitions)
    │                  │                      ├── Contains ──► NodeEventInfo (Event Definitions)
    │                  │                      ├── Contains ──► NodeType (Parent-Child Rules)
    │                  │                      └── Contains ──► NodeCategory (Property Grid Organization)
    │                  │
    │                  ├── Contains ──► NodeProperty (User Property Values)
    │                  │                      └── References ──► PropDeclaration
    │                  │
    │                  └── Contains ──► NodeEvent (User Event Bindings)
    │                                          └── References ──► NodeEventInfo
    │
    └── GridBag (Helper for wxGridBagSizer operations)
```

## Core Classes

### Node (`node.h`)

The central class representing a single UI component in the project tree. Each Node:

- Uses `shared_ptr` semantics (`NodeSharedPtr`, `enable_shared_from_this`) for safe parent-child relationships
- References immutable metadata via `m_declaration` (`NodeDeclaration*`)
- Stores user-configured properties in `m_properties` (`vector<NodeProperty>`) indexed by `m_prop_indices` map
- Tracks events in `m_map_events` (`unordered_map`)
- Maintains parent reference in `m_parent` and children in `m_children` vector

**Key Method Categories:**

| Category | Methods | Purpose |
|----------|---------|---------|
| Property Access | `as_string`, `as_int`, `as_bool`, `as_view` | Type-safe property value retrieval |
| Property Checks | `HasValue`, `HasProp`, `is_PropValue` | Check property existence and values |
| Type Queries | `is_Form`, `is_Sizer`, `is_Gen`, `is_Widget` | Query node type (delegates to NodeDeclaration) |
| Tree Navigation | `get_Parent`, `get_Child`, `get_Form`, `get_Folder` | Navigate the node hierarchy |
| Child Management | `AdoptChild`, `AddChild`, `RemoveChild` | Modify tree structure with validation |

### NodeDeclaration (`node_decl.h`)

Immutable metadata class defining a component type (e.g., `wxButton`, `wxBoxSizer`). Shared across all Node instances of that type.

**Contains:**
- Property definitions in `m_properties` (`map<string, PropDeclaration*>`)
- Event definitions in `m_events` (`map<string, NodeEventInfo*>`)
- Inheritance hierarchy in `m_base` (`vector<NodeDeclaration*>`)
- Parent-child rules in `m_type` (`NodeType*`)
- Code generator in `m_generator` (`BaseGenerator*`)
- Category organization in `m_category` (`NodeCategory`)
- SVG icon function in `m_bundle_function`

**Key Methods:**
- `get_PropDeclaration` / `get_EventInfo` - Property/event lookup
- `isSubclassOf` / `GetBaseClasses` - Inheritance traversal
- `get_AllowableChildren` - Query NodeType rules

### NodeProperty (`node_prop.h`)

Stores user-configured property values for Node instances.

**Structure:**
- `m_declaration` - Pointer to `PropDeclaration` for metadata
- `m_node` - Parent `Node*`
- `m_value` - String storage for the value
- `m_constant` - Cached friendly-name-to-wx-constant conversion

**Features:**
- Type-safe setters: `set_value` overloads for `int`, `wxColour`, `wxPoint`, etc.
- Type-safe getters: `as_int`, `as_bool`, `as_color`, `as_font`, etc.
- Lazy parsing of complex values
- `as_constant` method converts friendly names (e.g., "Default") to wxWidgets constants (e.g., "wxID_ANY")

### PropDeclaration (`prop_decl.h`)

Immutable metadata defining a property type. Shared across all property instances.

**Contains:**
- `m_name_enum` - `PropName` enum identifier
- `m_prop_type` - `PropType` (e.g., `type_string`, `type_int`, `type_bool`)
- `m_def_value` - Default value string
- `m_help` - Description for property grid tooltips
- `m_options` - Valid choices for `type_option`/`type_bitlist` properties

### NodeType (`node_types.h`)

Defines parent-child relationship rules for component types.

**Structure:**
- `m_gen_type` - Parent's `GenType`
- `m_map_children` - Map of allowed child types with maximum counts

**Child Count Constants:**
- `child_count::none` (0) - No children allowed
- `child_count::one` (1) - Exactly one child
- `child_count::two` (2) - At most two children
- `child_count::infinite` (-1) - Unlimited children

**Examples:**
- "wxBoxSizer accepts infinite type_widget children"
- "wxDialog accepts one type_menubar child"

This system enforces valid wxWidgets hierarchies, preventing invalid structures like multiple menubars in a frame.

### NodeEvent & NodeEventInfo (`node_event.h`)

Two-class pattern separating immutable event metadata from user-specific instances:

**NodeEventInfo** (immutable, shared):
- `m_name` - Event name (e.g., "OnButtonClick")
- `m_event_class` - wxWidgets class (e.g., "wxCommandEvent")
- `m_help` - Description text

**NodeEvent** (user instance):
- `m_info` - Pointer to shared `NodeEventInfo`
- `m_node` - Owning `Node*`
- `m_value` - User's handler function name

This separation optimizes memory by sharing metadata while allowing unique handler names per node.

### NodeCreator (`node_creator.h`)

Singleton factory (Meyer's singleton pattern) for creating Node instances and managing component metadata.

**Responsibilities:**
- Parses XML generator definitions during `Initialize()`
- Populates `m_a_declarations` (NodeDeclaration array indexed by GenName)
- Populates `m_a_node_types` (parent-child relationship rules)
- `CreateNode` validates parent-child compatibility
- `CreateNodeFromXml` deserializes `.wxui` projects
- `MakeCopy` performs deep node cloning

**Key Methods:**
- `CreateNode` - Returns `pair<NodeSharedPtr, Validity>` with error codes
- `get_declaration` / `get_NodeDeclaration` - Declaration lookup
- `get_ConstantAsInt` - wxWidgets constant mappings

### NodeCategory (`category.h`)

Hierarchical container for organizing properties in the wxUiEditor property grid UI.

**Contains:**
- Category name (as `wxString` for wxPropertyGrid compatibility)
- Property names (`vector<PropName>`)
- Event names (`vector<std::string>`)
- Nested subcategories (`vector<NodeCategory>`)

**Purpose:**
Groups related properties (e.g., "Window Styles", "Common", "Events") with collapsible headers in the property panel.

### GridBag (`node_gridbag.h`)

Helper class for managing `wxGridBagSizer` node operations.

**Features:**
- Wraps a wxGridBagSizer node and tracks grid dimensions
- `InsertNode` handles complex insertion logic (finding empty cells, adjusting spans)
- Static methods for movement: `MoveLeft`, `MoveRight`, `MoveUp`, `MoveDown`
- `GridBagSort` reorders children to match visual layout

## Data Flow

### Node Creation

1. User requests new component (menu, toolbar, drag-drop)
2. `NodeCreator::CreateNode()` is called with parent and `GenName`
3. Creator validates parent-child compatibility via `NodeType` rules
4. New `Node` is created with reference to appropriate `NodeDeclaration`
5. Properties are initialized from `PropDeclaration` defaults
6. Node is added to parent's children vector

### Property Access

1. Code requests property: `node->as_string(prop_var_name)`
2. Node looks up `NodeProperty` via `m_prop_indices` map
3. `NodeProperty::as_string()` returns the value
4. For constants, `as_constant()` converts friendly names using cached mapping

### Tree Navigation

Nodes support navigation in multiple directions:
- **Up:** `get_Parent()` returns parent node
- **Down:** `get_Child(index)` returns child at position
- **Form:** `get_Form()` traverses up to find containing form
- **Folder:** `get_Folder()` finds containing folder node

## Thread Safety

The Node system uses `shared_ptr` for memory management but is **not thread-safe** for concurrent modification. All tree operations should occur on the main UI thread.

## See Also

- [Generators](generators.md) - How generators use nodes for code generation
- [Overview](Overview.md) - Architecture overview
