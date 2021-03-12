Components which inherit from the **wxWindow** base class will also include all of the normal **wxWindow** event categories, which are not appropriate for some components. You can prevent these categories from displaying in the Property Grid Panel by adding one or more flags to the `compinfo` object (`flags="no_name_events"` where _name_ is they category to block). The following flag strings are supported:

- no_win_events: all wxWindow events are hidden
- no_key_events: all wxKeyEvent events are hidden
- no_mouse_events: all wxMouseEvent events are hidden
- no_focus_events: all wxFocusEvent events are hidden
