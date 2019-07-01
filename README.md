# JSON Utilities

> A series of functions that wrap the `rapidjson` library to enable easier serialization and deserialization.

## Serialization

For instance, suppose you have a simple `std::vector<int>` that you want to serialize. All you have to do is call the `serialize_to_json(...)` function, passing in the container:

```C++
const std::vector<int> container = { 1, 2, 3, 4, 5 };
const auto json = json_utils::serialize_to_json(container);
```

The library will treat a `std::vector<...>` as an array, and produce the following output:

```JSON
[1,2,3,4,5]
```

Not only does this work for relatively simple, contiguous containers, it also works for associative containers:

```C++
const std::unordered_map<std::string, int> container = { { "key_one", 1 },
                                                         { "key_two", 2 },
                                                         { "key_three", 3 } };

const auto json = json_utils::serialize_to_json(container);
```

The snippet above would result in the following JSON serialization:

```JSON
{"key_three":3,"key_two":2,"key_one":1}
```

Thanks to some template voodoo, you can even serialize messier, nested types:

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

The resultant JSON string looks as follows:

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

Note, that the ordering of a `std::map<...>` likely isn't going to remain stable from insertion to serialization; JSON makes no guarantees about ordering either.

If you'd rather store your data in a contiguous container, the example above can also be expressed as a `std::vector<std::pair<...>>`:

```C++
const std::vector<std::pair<std::string, std::map<std::string, double>>> container = {
    { "Key One", std::map<std::string, double>{ { "Subkey One", 16.0 },
                                                { "Subkey Two", 32.0 },
                                                { "Subkey Three", 64.0 } } },
    { "Key Two", std::map<std::string, double>{ { "Subkey One", 128.0 },
                                                { "Subkey Two", 256.0 },
                                                { "Subkey Three", 512.0 } } }
};

const auto json = json_utils::serialize_to_pretty_json(container);
```

Generally speaking, associative containers (like `std::map<...>` and `std::unordered_map<..>`) will be treated as JSON object, while non-associative containers (like `std::vector<...>`, `std::array<...>`, `std::set<...>`, `std::list<...>`, et cetera) will be treated as JSON arrays.

An exception to this is a `std::vector<std::pair<...>>`, which is also treated as an object (since it's very similar to a `std::[unordered_]map`).

## Deserialization

Deserialization is also supported!

Suppose we want to take the JSON object from the snippet above and transform it back into a C++ container. It's as simple as specifying the STL type you want to deserialize it into:

```C++
const auto map = json_utils::deserialize_from_json<std::map<std::string, int>>(json);
```

Or, alternatively:

```C++
const auto vector =
    json_utils::deserialize_from_json<std::vector<std::pair<std::string, int>>>(json);
```

That's it.

The derialization logic will use the provided template parameters as a guide for what the JSON object should look like at runtime. So, if the template suggests that, say, a JSON object should be present, and a different type is presented at runtime, then a `std::invalid_argument` exception will be thrown.

## Customization and Handling of Custom Types

Since you'll probably want to serialize something other than plain-old datatypes (PODs), homogeneous arrays, or homogenous objects, you can use argument dependent lookup (ADL) to provide handling for custom types.

```C++
namespace sample
{
class heterogeneous_widget
{
public:
    const std::vector<std::string>& get_data() const
    {
        return m_data;
    }

    void set_data(std::vector<std::string>&& data)
    {
        m_data = std::move(data);
    }

    const std::string& get_timestamp() const
    {
        return m_timestamp;
    }

    void set_timestamp(std::string timestamp)
    {
        m_timestamp = std::move(timestamp);
    }

private:
    std::string m_timestamp = "2019/05/29";
    std::vector<std::string> m_data = { "Test String One", "Test String Two", "Test String Three" };
};

template <typename Writer>
void to_json(Writer& writer, const sample::heterogeneous_widget& widget)
{
    writer.StartObject();

    writer.Key("Timestamp");
    writer.String(widget.get_timestamp().c_str());

    writer.Key("Data");
    using json_utils::serializer::to_json;
    to_json(writer, widget.get_data());

    writer.EndObject();
}

void from_json(const rapidjson::Document& document, sample::heterogeneous_widget& widget)
{
    if (!document.IsObject()) {
        return;
    }

    const auto timestamp_iterator = document.FindMember("Timestamp");
    if (timestamp_iterator == document.MemberEnd() || !timestamp_iterator->value.IsString()) {
        return;
    }

    widget.set_timestamp(timestamp_iterator->value.GetString());

    const auto data_iterator = document.FindMember("Data");
    if (data_iterator == document.MemberEnd() || !data_iterator->value.IsObject()) {
        return;
    }

    using json_utils::deserializer::from_json;

    std::vector<std::string> data;
    from_json(data_iterator->value, data);
    
    widget.set_data(std::move(data));
}
}
```

Note that in order for ADL to find the correct overload, the `to_json(...)` and `from_json(...)` functions will need to be in the same namespace as the custom type that is to be serialized. With regard for the example shown above, that would be the `sample` namespace.

## Compatibility

This project aims to target C++11 and up. Where sensible, support for C++17 features (like `std::filesystem::path` and `std::optional<...>`) has been provided.

## Building Instructions

After checking out the source, run the following commands from the base directory to build in `debug`:

```
mkdir debug && cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

Building in `release` is very similar:

```
mkdir release && cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
