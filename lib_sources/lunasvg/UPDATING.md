# General Information

The `lunasvg_2/` folder is a static copy of the lunasvg 2.4.1 source code modified to work with wxWidgets. It can be compiled with C++11.

The `lunasvg_3/` folder contains subrepo copies of the lunasvg 3.x and plutovg repositories. These copies are modified so that they can be built into wxWidgets. C++17 is required to compile these sources.

If you are just building wxUiEditor, you don't need to do anything -- unlike submodules, subrepos don't require any updating before you can build the current sources.

# Maintainers

The following sections are for maintainers who need to merge changes from the lunasvg and plutovg repositories into the wxUiEditor code base.

To update, you need to have subrepo, which is currently an external program that calls git to perform it's functionality. To use subrepo, you should first clone https://github.com/ingydotnet/git-subrepo -- you will be running subrepo from this clone. See the instructions on the URL for Unix/Max systems (it's much easier to do then on Windows)

For Windows, follow these instructions:

- set GIT_SUBREPO_ROOT=%clone location%
- add to PATH: %clone location%;%clone location%/lib

Start a new cmd prompt with the above environment changes and run `git subrepo` -- if subrepo is working, it will list all the command line options. Note that you need to use '/' for paths on Windows.

## Updating lunasvg and plutovg

Ensure that there are no pending changes -- stash them if there are.

From the root of the wxUiEditor repository, run the following two commands:

    git subrepo pull lib_sources/lunasvg/lunasvg_3
    git subrepo pull lib_sources/lunasvg/lunasvg_3/plutovg

These two commands will merge changes in the upstream repository into the current subfolders -- each pull will create a single commit in history.

The `CMakeLists.txt` in lib_sources/lunasvg can be used to verify that wxlunasvg 3 can be built. Note that it includes it's list of source files -- you may need to modify this to pickup files added to either lunasvg or plutovg (see CMakeLists.txt in `lunasvg_3/` and `lunasvg_3/plutovg`.

## After updating

Updates are a merge, so it's likely that everything will just work. However, if something goes wrong, the following changes are what were initially made and can be used as a guideline.



In lunasvg_3/source/graphics.h add the following:

```C++
// Disable warnings about possible loss of data when converting int to float
// Disable warnings about possible loss of data when converting size_t to int
#ifdef _MSC_VER
    #pragma warning(disable: 4244)
    #pragma warning(disable: 4267)
#elif defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wconversion"
    #pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
```

In lunasvg_3/plutovg/plutovg-private.h add:

```C
// Disable warnings about possible loss of data when converting int to float
// Disable warnings about possible loss of data when converting size_t to int
#ifdef _MSC_VER
    #pragma warning(disable: 4244)
    #pragma warning(disable: 4267)
#elif defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wconversion"
    #pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
```

In lunasvg_3/plutovg/plutovg-ft-raster.c add:

```C
// Disable warnings about possible loss of data when converting size_t to long
#ifdef _MSC_VER
    #pragma warning(disable: 4267)
#else
    #pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
```

In lunasvg_3/plutovg/plutovg-ft-stroker.c add:

```C
// Disable warnings about signed/unsigned mismatch
#ifdef _MSC_VER
    #pragma warning(disable: 4018)
#else
    #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
```

In lunasvg_3/plutovg/plutovg.h replace

```C
#ifdef PLUTOVG_BUILD
#define PLUTOVG_API PLUTOVG_EXPORT
#else
#define PLUTOVG_API PLUTOVG_IMPORT
#endif
```

with

```C
#define PLUTOVG_API
```

In lunasvg_3/include/lunasvg.h insert the following code immediately after the #include statements:

```C++
#ifndef WXMAKINGDLL
    #define LUNASVG_BUILD_STATIC
#endif

#ifdef WXBUILDING
    #define LUNASVG_BUILD
#endif
```

Replace all occurences of `namespace lunasvg` with `namespace wxlunasvg`.

In lunasvg_3/source/lunasvg.cpp replace all occurences of `lunasvg::` with `wxlunasvg:`.

At this point you should be able to build the library using C++17.
