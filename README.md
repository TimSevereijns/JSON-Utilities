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

Not only does this work for relatively simple, contiguous containers, it also works for associative containers:

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

Deserialization is also supported!

Given that we don't (yet) have reflection in C++, there are obviously going to be some limitations, but certain use-cases can certainly be pretty straightforward.

For instance, suppose you have a simple, flat JSON object. Here's how such an object might be deserialized:

```C++
const std::string json =
R"({)"
R"(    "object1": { "subKey1": 10, "subKey2": 20, "subKey3": 30 }, )"
R"(    "object2": { "subKey1": 40, "subKey2": 50, "subKey3": 60 }  )"
R"(})";

const auto map = json_utils::deserialize_from_json<std::map<std::string, int>>(json);
```

The derialization logc will use the provided template parameters as a guide for what the JSON object should look like at runtime. If the template suggests that a JSON object should be next, and a different type is presented at runtime, then a `std::invalid_argument` exception will be thrown.

## Customization and Handling of Custom Types

Since you'll probably want to serialize something other than plain-old datatypes (PODs), homogeneous arrays, or homogenous objects, you can use argument dependent lookup (ADL) to provide handling for custom types.

```C++
class heterogeneous_widget
{
    public:
    heterogeneous_widget()
    {
        m_data = std::vector<std::string>{ "Test String One", "Test String Two",
                                            "Test String Three" };
    }

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

    friend bool
    operator==(const heterogeneous_widget& lhs, const heterogeneous_widget& rhs) noexcept
    {
        return lhs.m_timestamp == rhs.m_timestamp && lhs.m_data == rhs.m_data;
    }

    private:
    std::string m_timestamp = "2019/05/29";
    std::vector<std::string> m_data;
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
    if (timestamp_iterator == document.MemberEnd()) {
        return;
    }

    widget.set_timestamp(timestamp_iterator->value.GetString());

    const auto vector_iterator = document.FindMember("Data");
    if (vector_iterator == document.MemberEnd()) {
        return;
    }

    using json_utils::deserializer::from_json;

    std::vector<std::string> data;
    from_json(vector_iterator->value, data);
    
    widget.set_data(std::move(data));
}
```
