This directory is primarily used to test wxPython code generation, though it can be used to test C++ code generation as well.

## pytest

The current requirements for running the `pytest.py` app is Python version 3.10 and wxPython 4.2.x. If they are installed then you can run the app via:

```
    python pytest.py
```

## cpptest

Before you can build cpptest.exe for the first time, you must run:

```
    cmake -G "Ninja Multi-Config" . -B build
```

After the initial configuration, you can build using:

```
    cmake --build build
```
