# Limiting events in a component

Components which inherit from the **Window Events** base class will have all of the normal **wxWindow** event categories (mouse, keyboard, etc.). You can prevent specific categories from displaying in the Property Grid Panel by adding one or more flags to the `compinfo` object (`flags="no_name_events"` where _name_ is they category to block). The following flag strings are supported:

- no_win_events: all wxWindow events are hidden
- no_key_events: all wxKeyEvent events are hidden
- no_mouse_events: all wxMouseEvent events are hidden
- no_focus_events: all wxFocusEvent events are hidden

# Adding a component or property

The files `enum_class.h` and `enum_classes.cpp` _must_ be updated any time you add a new component or a property, or if you change an existing class name or property type. In a DEBUG build you will get warnings if you forget to update one or more of the enumeration lists and there's a fairly good chance the program will not work correctly or even crash if you try to use a component with the missing enumeration.
