# ttwx Tests

This directory contains Catch2 unit tests for the ttwx namespace functions.

### Configure CMake

Catch2 is fetched via FetchContent.

- BUILD_TESTING option must be set to ON (default is off) when configuring CMake

## Build the Test

`cmake --build build --config Debug --target wxUiEditor`

Because BUILD_TESTING is on, both wxUiEditor.exe and test_ttwx.exe will be built.

## Run the Test

### Direct Execution
```cmd
# Windows
build\bin\Debug\test_ttwx.exe

# Linux/Mac
build/bin/test_ttwx
```

## Useful Test Commands

### List all available tests
```bash
test_ttwx --list-tests
```

### Run a specific test case
```bash
test_ttwx "ttwx::atoi"
```

### Run tests with a specific tag
```bash
test_ttwx [ttwx]
```

### Show successful tests too
```bash
test_ttwx -s
```

### Get help
```bash
test_ttwx --help
```

## Expected Results
If all tests pass, you should see output like:
```
===============================================================================
All tests passed (XXX assertions in YY test cases)
```

If any test fails, Catch2 will show:
- The test case name
- The file and line number
- The expression that failed
- The actual vs expected values

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
