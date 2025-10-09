# ttwx Tests

This directory contains Catch2 unit tests for the ttwx namespace functions.

## Test File

- `test_ttwx.cpp` - Comprehensive tests for all functions in `ttwx.cpp`

## Functions Tested

The following functions from the ttwx namespace are tested:

1. **String Search Functions**
   - `find_oneof()` - Find any one of the characters in a group
   - `find_nonspace()` - Find the next non-whitespace character
   - `find_extension()` - Extract file extension from a path

2. **Path Manipulation Functions**
   - `replace_extension()` - Replace or add file extension
   - `find_filename()` - Extract filename from a path
   - `append_filename()` - Append filename to a path
   - `append_folder_name()` - Append folder name to a path

3. **String Extraction**
   - `extract_substring()` - Extract a quoted or bracketed substring
   - `create_substring()` - Helper that returns the extracted substring

4. **String Conversion**
   - `MakeLower()` - Convert string to lowercase (non-UTF-8)
   - `atoi()` - Convert string to integer (supports hex with 0x prefix)
   - `itoa()` - Convert numeric value to string

5. **Character Classification**
   - `is_alnum()`, `is_alpha()`, `is_digit()`, `is_whitespace()`, etc.
   - `is_found()` - Check if a position was found (not -1)

6. **Path Utilities**
   - `back_slashesto_forward()` - Convert backslashes to forward slashes
   - `forward_slashesto_back()` - Convert forward slashes to backslashes
   - `add_trailing_slash()` - Ensure path ends with a slash

## Building and Running Tests

### Configure CMake
The tests are automatically included when CMake is configured. Catch2 is fetched via FetchContent.

```bash
# From the build directory
ninja -f build-Debug.ninja
```

### Build the Tests
```bash
# Build the test executable
cmake --build . --target test_ttwx
```

### Run the Tests

**Using CTest:**
```
cd build
ctest -C Debug -R test_ttwx --output-on-failure
```

**Direct Execution:**
```bash
# Windows
bin\Debug\test_ttwx.exe

# Unix/Linux/Mac
bin/test_ttwx.exe
```

**Run Specific Tests:**
```bash
# Run tests matching a tag
bin\Debug\test_ttwx.exe [ttwx]

# Run specific test case
bin\Debug\test_ttwx.exe "ttwx::find_oneof"
```

**List All Tests:**
```bash
bin\Debug\test_ttwx.exe --list-tests
```

## Test Coverage

Each function has multiple test sections covering:
- Normal operation cases
- Edge cases (empty strings, invalid inputs)
- Platform-specific behavior (Windows vs Unix paths)
- Error conditions

## Adding New Tests

To add tests for new ttwx functions:

1. Add test cases to `test_ttwx.cpp` following the Catch2 pattern:
```cpp
TEST_CASE("ttwx::new_function", "[ttwx]")
{
    SECTION("Description of test case")
    {
        // Arrange
        // Act
        // Assert with REQUIRE() or CHECK()
    }
}
```

2. Rebuild and run the tests to verify.

## References

- [Catch2 Documentation](https://github.com/catchorg/Catch2)
- [wxWidgets Documentation](https://docs.wxwidgets.org/)
