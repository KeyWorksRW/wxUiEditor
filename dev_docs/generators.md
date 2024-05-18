# Generators

The outer function that generates code does so by getting a pointer to the generator that can create that code, and then calling various functions in that generator. In most cases the `BaseGenerator` will return a value indicating no code was generated. The generator must override any functions where it needs to provide specific code generation.

A typical generator header file will look something like the following:

```c++
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

    std::optional<ttlib::sview> CommonConstruction(Code& code) override;
    std::optional<ttlib::sview> CommonSettings(Code&) override;
    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
```

- `CreateMockup` is required to display the object in the Mockup panel and any of the preview windows.
- `OnPropertyChange` is used if something specific needs to be done if a property is changed without recreating the object.
- `CommonConstruction` generates the code necessary to create the object in C++ or Python.
- `CommonSettings` generates code after the object has been constructed.
- `GenXrcObject` generates the XRC xml node for the object
- `GetIncludes` should add any required wxWidgets C++ header files that must be included to create the object.
- `RequiredHandlers` should return the XRC required handler for the object.

Look at base_generator.h to view all of the functions that can be called. Note that if a function in the base_generator.h header file does not define the return code in the header file itself, then it means there is a default definition in base_generator.cpp. In that case, if a generator needs to override this function, you should first look at the default definition to see if you need to first call the BaseGenerator version before adding to it with a generator-specific version.
