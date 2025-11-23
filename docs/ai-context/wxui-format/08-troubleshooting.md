# Troubleshooting and Common Issues

Common problems when working with .wxui files and their solutions.

## XML Syntax Errors

### Unclosed Tags
**Error:** `XML parsing error: Mismatched tag`
```xml
<!-- WRONG -->
<object class="wxButton" name="m_btn">
  <label>Click Me
</object>

<!-- CORRECT -->
<object class="wxButton" name="m_btn">
  <label>Click Me</label>
</object>
```

### Special Characters Not Escaped
**Error:** `XML parsing error: Invalid character`
```xml
<!-- WRONG -->
<label>5 < 10 & x > 2</label>

<!-- CORRECT -->
<label>5 &lt; 10 &amp; x &gt; 2</label>
```

**XML Entity Reference:**
- `&lt;` for `<`
- `&gt;` for `>`
- `&amp;` for `&`
- `&quot;` for `"`
- `&apos;` for `'`

### Missing Required Attributes
**Error:** `Missing required 'class' attribute`
```xml
<!-- WRONG -->
<object name="my_widget">

<!-- CORRECT -->
<object class="wxButton" name="my_widget">
```

## Validation Errors

### Invalid Property Values

**Boolean properties:**
```xml
<!-- WRONG -->
<enabled>true</enabled>
<checked>yes</checked>

<!-- CORRECT -->
<enabled>1</enabled>  <!-- or 0 -->
<checked>1</checked>  <!-- or 0 -->
```

**Size format:**
```xml
<!-- WRONG -->
<size>400x300</size>
<size>400 300</size>

<!-- CORRECT -->
<size>400,300</size>  <!-- width,height -->
```

**Color format:**
```xml
<!-- WRONG -->
<fg>red</fg>
<bg>rgb(255,0,0)</bg>

<!-- CORRECT -->
<fg>#FF0000</fg>     <!-- Hex RGB -->
<bg>255,0,0</bg>    <!-- Decimal RGB -->
```

### Incompatible Property Combinations

**wxStdDialogButtonSizer:**
```xml
<!-- WRONG: OK and Yes are mutually exclusive -->
<OK>1</OK>
<Yes>1</Yes>

<!-- CORRECT: Choose one affirmative button -->
<OK>1</OK>
<Cancel>1</Cancel>
```

**wxGridSizer rows/cols:**
```xml
<!-- WRONG: Both rows and cols are 0 -->
<rows>0</rows>
<cols>0</cols>

<!-- CORRECT: At least one must be non-zero -->
<rows>0</rows>  <!-- Auto-calculate -->
<cols>2</cols>  <!-- Fixed -->
```

## Sizer Layout Issues

### Widget Not Expanding
**Problem:** Widget doesn't grow when window resizes

**Solution 1:** Add wxEXPAND flag
```xml
<object class="sizeritem">
  <object class="wxTextCtrl" name="m_text"/>
  <flag>wxEXPAND|wxALL</flag>  <!-- Added wxEXPAND -->
  <option>1</option>
</object>
```

**Solution 2:** Set proportion > 0
```xml
<object class="sizeritem">
  <object class="wxTextCtrl"/>
  <option>1</option>  <!-- proportion=1, will stretch -->
  <flag>wxEXPAND</flag>
</object>
```

### Unexpected Layout Direction
**Problem:** Items stacking vertically when expecting horizontal

**Check sizer orientation:**
```xml
<object class="wxBoxSizer">
  <orient>wxHORIZONTAL</orient>  <!-- NOT wxVERTICAL -->
</object>
```

### Borders Not Showing
**Problem:** Border size set but no border visible

**Solution:** Specify which sides have borders
```xml
<!-- WRONG: border_size but no border flags -->
<border>10</border>

<!-- CORRECT: Add border flags -->
<flag>wxALL</flag>      <!-- All sides -->
<border>10</border>

<!-- OR specific sides -->
<flag>wxLEFT|wxRIGHT</flag>
<border>10</border>
```

## Event Binding Problems

### Handler Not Called
**Problem:** Event handler never executes

**Check 1:** Verify event.Skip() in base class
```cpp
// Base class must call Skip()
virtual void OnButtonClick( wxCommandEvent& event ) { event.Skip(); }
```

**Check 2:** Override in derived class
```cpp
class MyDialog : public MyDialogBase
{
protected:
    void OnButtonClick( wxCommandEvent& event ) override;  // Note: override
};
```

**Check 3:** Verify event name matches
```xml
<!-- Event name in XML -->
<event name="wxEVT_BUTTON">OnButtonClick</event>
```
```cpp
// Method name must match exactly
void OnButtonClick( wxCommandEvent& event ) override
```

### Wrong Event Signature
**Problem:** Compilation error: incompatible override

**Solution:** Match event class type
```cpp
// WRONG: wxMouseEvent for wxEVT_BUTTON
void OnButtonClick( wxMouseEvent& event ) override;

// CORRECT: wxCommandEvent for wxEVT_BUTTON
void OnButtonClick( wxCommandEvent& event ) override;
```

See events.md for correct event class types.

## Code Generation Issues

### Member Variable Not Declared
**Problem:** `m_button` undeclared identifier

**Cause:** Widget missing `name` attribute
```xml
<!-- WRONG: No name -->
<object class="wxButton">
  <label>OK</label>
</object>

<!-- CORRECT: Named widget -->
<object class="wxButton" name="m_ok_button">
  <label>OK</label>
</object>
```

**Result:** `wxButton* m_ok_button;` declared in base class

### Widget Name Doesn't Follow Conventions
**Problem:** Generated code has warnings about naming

**Convention:** Use `m_` prefix for member variables
```xml
<!-- Recommended -->
<object class="wxButton" name="m_save_button">

<!-- Also valid but less conventional -->
<object class="wxButton" name="saveButton">
```

### Base Class Changes Not Reflected
**Problem:** Modified .wxui but code unchanged

**Solution:** Trigger regeneration
1. In wxUiEditor: File → Generate Code (or Ctrl+G)
2. Command line: `wxUiEditor --generate myfile.wxui`
3. Build system: Ensure .wxui file is in dependencies

### Compilation Errors After Regeneration
**Problem:** Code doesn't compile after regenerating base class

**Common Cause:** Base class changed function signature

**Solution:** Update derived class overrides to match
```cpp
// Base class changed from:
virtual void OnClick( wxCommandEvent& event );
// To:
virtual void OnButtonClick( wxCommandEvent& event );

// Update derived class:
void OnClick( wxCommandEvent& event ) override;        // REMOVE
void OnButtonClick( wxCommandEvent& event ) override;  // ADD
```

## Migration from Other Designers

### From wxFormBuilder

**Key Differences:**
- Property names may differ (check XML definitions)
- Event handler naming conventions may vary
- Sizer child properties structure different

**Conversion Strategy:**
1. Export to XRC from wxFormBuilder
2. Import XRC into wxUiEditor
3. Review imported properties
4. Regenerate code and update derived classes

### From wxGlade

**Differences:**
- Code generation templates completely different
- Base/derived class split may not exist
- Manual extraction of event handlers needed

### From DialogBlocks

**Note:** DialogBlocks uses proprietary format
- Manual recreation in wxUiEditor required
- Use screenshots/exported code as reference

## Common AI Agent Errors

### Missing Sizer Wrappers
**Error:** Widget added directly to parent without sizer
```xml
<!-- WRONG -->
<object class="wxDialog">
  <object class="wxButton" name="m_btn"/>  <!-- Direct child -->
</object>

<!-- CORRECT -->
<object class="wxDialog">
  <object class="wxBoxSizer">
    <orient>wxVERTICAL</orient>
    <object class="sizeritem">
      <object class="wxButton" name="m_btn"/>
    </object>
  </object>
</object>
```

### Forgetting sizeritem Wrapper
**Error:** Widget added directly to sizer
```xml
<!-- WRONG -->
<object class="wxBoxSizer">
  <object class="wxButton"/>  <!-- Missing sizeritem -->
</object>

<!-- CORRECT -->
<object class="wxBoxSizer">
  <object class="sizeritem">
    <object class="wxButton"/>
  </object>
</object>
```

### Wrong Sizer Item Type for GridBagSizer
**Error:** Using sizeritem in wxGridBagSizer
```xml
<!-- WRONG -->
<object class="wxGridBagSizer">
  <object class="sizeritem">  <!-- Wrong item type -->

<!-- CORRECT -->
<object class="wxGridBagSizer">
  <object class="gbsizeritem">  <!-- Grid bag sizer item -->
    <cellpos>0,0</cellpos>
```

## FAQ

**Q: Can I edit the base class files?**
A: No. Any changes will be lost on next regeneration. Add custom code to derived classes or after the "End of generated code" marker.

**Q: How do I add a custom member variable?**
A: Add it in the derived class .h file, not the base class.

**Q: Can I change the base class name after creating the project?**
A: Yes, edit the `<class_name>` property in the Project node. Regenerate and update your derived class's parent.

**Q: Why doesn't my dialog have a menu bar?**
A: wxDialog doesn't support menu bars. Use wxFrame instead.

**Q: How do I make a modal dialog?**
A: Call `ShowModal()` instead of `Show()` in your code. The .wxui file defines the UI, not the showing method.

**Q: Can I use the same .wxui file for multiple languages?**
A: Yes. Set output file paths for each language in Project properties. Use `--generate_all` flag.

**Q: How do I debug layout issues?**
A: Use wxUiEditor's Preview mode to see layout in real-time. Check sizer colors (red=issues, green=ok).

**Q: Can I convert existing C++ wxWidgets code to .wxui?**
A: Yes. Manually extract dialog construction, sizer hierarchy, and event bindings following the patterns in examples.md and code-generation.md.

**Q: What version of wxWidgets is required?**
A: wxUiEditor generates code compatible with wxWidgets 3.0+. Specific widget support depends on wxWidgets version.

## Getting Help

- **Documentation:** docs/users/ and docs/contributors/
- **Examples:** Import sample projects in wxUiEditor
- **wxWidgets Docs:** https://docs.wxwidgets.org/
- **wxUiEditor GitHub:** Report issues and request features
