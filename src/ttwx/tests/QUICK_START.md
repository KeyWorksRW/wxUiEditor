# Quick Start: Running ttwx Tests

## Prerequisites
- CMake has already fetched Catch2 via FetchContent
- Build environment is configured

## Build the Test

### Windows (from build directory)
```cmd
ninja test_ttwx
```
or
```cmd
cmake --build . --target test_ttwx
```

### Linux/Mac (from build directory)
```bash
make test_ttwx
```
or
```bash
cmake --build . --target test_ttwx
```

## Run the Test

### Using CTest (Recommended)

```
cd build
ctest -C Debug -R test_ttwx --output-on-failure
```

### Direct Execution
```cmd
# Windows
bin\Debug\test_ttwx.exe

# Linux/Mac
bin/test_ttwx
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
