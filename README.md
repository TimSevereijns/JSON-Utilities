# JSON Utilities

This repository provides a series of functions that wrap the `rapidjson` library to enable easier serialization and deserialization.

## Serialization

For instance, suppose you have a simple `std::vector<int>` that you want to serialize. All you have to do is call the `serialize_to_json(...)` function, passing in the container:

```C++
const std::vector<int> container = { 1, 2, 3, 4, 5 };
const auto json = json_utils::serialize_to_json(container);
```

The output, of course, is rather simple:

```JSON
[1,2,3,4,5]
```

Not only does this work for relatively simple, continuous containers, it also works for associative containers:

```C++
const std::unordered_map<std::string, int> container = { { "key_one", 1 },
                                                         { "key_two", 2 },
                                                         { "key_three", 3 } };

const auto json = json_utils::serialize_to_json(container);
```

The snippet above would result in the following:

```JSON
{"key_three":3,"key_two":2,"key_one":1}
```

Thanks to some template voodoo, you can even serialize messier types:

```C++
const std::map<std::string, std::map<std::string, double>> container = {
    { "Key One", std::map<std::string, double>{ { "Subkey One", 16.0 },
                                                { "Subkey Two", 32.0 },
                                                { "Subkey Three", 64.0 } } },
    { "Key Two", std::map<std::string, double>{ { "Subkey One", 128.0 },
                                                { "Subkey Two", 256.0 },
                                                { "Subkey Three", 512.0 } } }
};

const auto json = json_utils::serialize_to_pretty_json(container);
```

```JSON
{
    "Key One": {
        "Subkey One": 16.0,
        "Subkey Three": 64.0,
        "Subkey Two": 32.0
    },
    "Key Two": {
        "Subkey One": 128.0,
        "Subkey Three": 512.0,
        "Subkey Two": 256.0
    }
}
```

Note, that the ordering of a `std::map<...>` likely isn't going to remain stable from insertion to serialization. JSON makes no guarantees about ordering either, though.

## Deserialization

Coming soon...