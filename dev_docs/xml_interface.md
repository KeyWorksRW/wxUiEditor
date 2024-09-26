# XML Files

The XML files in the `src/xml` directory declare every node type that can be created. Properties and events are either specified in the XML file, or they are created by the generator class calling `AddPropsAndEvents()`. Before changes to these files show up in wxUiEditor, you _must_ regenerate the `wxUiEditor.wxui` project file in order to update the generated `wxue_data` files. Generating `wxue_data.cpp` and `wxue_data.h` strips out all comments and whitespace before compressing it using `zlib`.

## Inheriting interface classes

A generator can inherit an interface class either by using a `<inherits class=` node, or by creating a `<category` node with a `base_name=` attribute that specifies the class to begin inheriting from. Note that currently if you use `base_name`, you cannot hide the included properties.

If you need to override a default property value from an inherited class, then add the property value as a child of the `inherits` node.

## Limiting events in a declaration

Declarations which inherit from the **Window Events** base class will have all of the normal **wxWindow** event categories (mouse, keyboard, etc.). You can prevent specific categories from displaying in the Property Grid Panel by adding one or more flags to the `gen` object (`flags="no_name_events"`) where _name_ is they category to block). The following flag strings are supported:

- no_win_events: all wxWindow events are hidden
- no_key_events: all wxKeyEvent events are hidden
- no_mouse_events: all wxMouseEvent events are hidden
- no_focus_events: all wxFocusEvent events are hidden

## Limiting inherited properties

When an interface is inherited (`<inherits class=`) you can add child nodes called `<hide name=` where the name specifies the property you do not want inherited. This will typically be used when inheriting from the `wxWindow` interface. For example, to prevent a tooltip property, you could encode the following in the XML file:

```xml
    <inherits class="wxWindow">
        <!-- Indicate which properties should not be shown in the Property Panel -->
        <hide name="tooltip" />
    </inherits>
```

## Adding a declaration or property

The files `gen_enums.h` and `gen_enums.cpp` _must_ be updated any time you add a new component or a property, or if you change an existing class name or property type. In a DEBUG build you will get warnings if you forget to update one or more of the enumeration lists and there's a fairly good chance the program will not work correctly or even crash if you try to use a component with the missing enumeration.

You can hide a specific property by including the attribute `hide="1"`.

## Adding a new form

- Add the type (if new) to lst_form_types in Node.cpp
- Add type and possible children to node_init.cpp

## Adding a new category

Adding a new category is similar to creating a generator. You need to add the class name to the generator lists in gen_enum.h and gen_enums.cpp (there is a category section at the top of each generator list). The type must be set to `interface`. If it's going to be used by multiple generators, then it make sense to place it in one of the interface xml files. However, this is not a requirement -- for example, `project_xml.xml` has a category definition that only Projects use.
