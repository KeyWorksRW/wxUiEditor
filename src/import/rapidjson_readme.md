# rapidjson

The rapidjson directory contains is from https://github.com/Tencent/rapidjson

Only the files from the `include/rapidjson` directory of that repository are in the rapidjson directory. All the files except for the directory are include except for the `msinttypes` directory (only used for really old MSVC compilers).

## Implementation notes in WxCrafter class

The rapidjson examples suggest first using `Value::.HasMember(...)` to determine if a key is there, and then you can use the `Value::[...]` operator if the key exists. If you attempt to use the `[...]` operator to access a key that doesn't exist, then you get an assertion error in Debug builds, and in release builds it allocates memory for an empty Value which is never freed.

This approach makes sense if you are iterating over all the keys, but if you need random access to a specific key and you don't know if it actually exists, then the `HasMember` approach means the key lookup must be performed twice -- once to see if the key exists, and a second time to access it if it does.

The `import_wxcrafter.cpp` module contains the following:

```c++
namespace rapidjson
{
    const Value& FindValue(const Value& object, const char* key);

    static const Value empty_value;
}
```

Using `FindValue(...)` to get the Value for a key means the lookup is only performed once, and you can call `IsNull()` or any of the other `Is<T>` functions to see if you retrieved the Value you need. The single non-allocated `empty_value` Value is returned any time a key doesn't exist. For example, the following can be used when you don't know ahead of time what type the Value will be:

```c++
    if (auto& prop_value = FindValue(value, "m_value"); !prop_value.IsNull())
    {
        ...
    }
```

If a specific Value type is required, then a check for just that type can be performed as in the following:

```c++
        if (auto& windows = FindValue(document, "windows"); windows.IsArray())
        {
            for (auto& iter: windows.GetArray())
```
