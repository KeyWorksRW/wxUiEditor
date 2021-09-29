# XML Files

The XML files in this directory declare every type node that can be created. During the build process, these files are converted into gzip and saved as a `*.hgz` file which is a `unsigned char` array that is added to the program via `#include` statements.

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

## Adding a new generator

- Add the declaration to one of the XML files.
- Add the generator class to gen_enums.cpp and gen_enums.h.
- Add any unique property types to gen_enums.cpp and gen_enums.h.
- Add the generator class to one of the files in generate/ or create a new file
- Add any required wxWidgets header file to gen_initialize.cpp
- Add the generator class name to the InitGenerators() in gen_initialize.cpp

While testing, you can use any existing image, and insert the control using the Edit menu's `Insert Widget` command. Once ready for release, then take the following steps:

- Create a new graphic for the class -- if it won't be embedded, then you must use the `Convert Image...` tool to create a _file_.h_img.
- Add the header file (if not embedded) and name matching to xpm.cpp -- the name must match to declaration at the top of this list of todo items
- Change the name of the bitmap in the XML declaration to match the shortname you specified in xpm.cpp.
- Add the control to ribbon.wxui. If it's part of a dropdown list of controls, add it to ribbon_tools.
