# **JSON Utilities**

[![codecov](https://codecov.io/gh/TimSevereijns/JSON-Utilities/branch/master/graph/badge.svg)](https://codecov.io/gh/TimSevereijns/JSON-Utilities)

A toy project aimed at providing a series of utility functions built around the popular `rapidjson` library to make serialization and deserialization a __little__ easier.

The primary motivation for this project is simply a desire to experiment with a bit of template meta-programming. 

# Serialization

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

The resultant JSON string is as follows:

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

Note, that the ordering of a `std::unordered_map<...>` likely isn't going to remain stable from insertion to serialization; JSON makes no guarantees about ordering either.

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

Generally speaking, any container type whose `value_type` is a `std::pair<..., ...>` will be converted to a JSON object.

# Deserialization

Deserialization is also supported. In fact, deserialization can be achieved in two distinct ways, either by using a DOM or by using a SAX parser.

Let's start with the DOM workflow.

## Deserialization via DOM

Suppose we have a simple JSON object that we'd like to deserialize:

```JSON
{"key_one":1,"key_two":2,"key_three":3}
```

Deserialization is as simple as specifying the type you want to deserialize the JSON string into. The following are all valid options, given our simple example:

```C++
const auto foo = json_utils::deserialize_via_dom<std::map<std::string, int>>(json);
const auto bar = json_utils::deserialize_via_dom<std::list<std::pair<std::string, int>>>(json);
const auto baz = json_utils::deserialize_via_dom<std::vector<std::pair<std::string, int>>>(json);
```

Because we're attempting to deserialize a JSON object, we'll need to specify a C++ container type whose `value_type` is a `std::pair<std::string, int>`.

The derialization logic will use the provided template parameters as a guide for what the JSON document should look like at runtime. If the deserialization target type doesn't match the runtime input, an exception will be thrown.

## Deserialization via SAX

Since we may want to avoid the creation of an intermediate object model (i.e., a DOM), we can also deserialize from JSON directly to a target container type. Given the same example code as in the DOM example above, we can achieve deserialization as follows:

```C++
const auto foo = json_utils::deserialize_via_sax<std::map<std::string, int>>(json);
const auto bar = json_utils::deserialize_via_sax<std::list<std::pair<std::string, int>>>(json);
const auto baz = json_utils::deserialize_via_sax<std::vector<std::pair<std::string, int>>>(json);
```
Here's an example that serializes and deserializes a container without ever building a DOM:

```C++
using container_type =
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>;

const container_type source_container = {
    { "objectOne", { { "1", "A" }, { "2", "B" }, { "3", "C" }, { "4", "D" } } },
    { "objectTwo", { { "4", "D" }, { "3", "C" }, { "2", "B" }, { "1", "A" } } }
};

const auto json = json_utils::serialize_to_json(source_container);
const auto resultant_container = json_utils::deserialize_via_sax<container_type>(json);
```

Note that SAX deserialization requires the use of C++17. 

## Customization and Handling of Custom Types

Since you'll probably want to serialize and deserialize custom, non-STL types, you can overload the `to_json(...)` and `from_json(...)` functions to achieve your needs.

Note that the current API only allows for custom types to be deserialized via the DOM workflow. Support for custom types in the SAX model is a work in progress.

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
    json_utils::serializer::to_json(writer, widget.get_data());

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

    std::vector<std::string> data;
    json_utils::dom_deserializer::from_json(data_iterator->value, data);
    
    widget.set_data(std::move(data));
}
}
```

Note that in order for argument dependent lookup (ADL) to find the correct overload, the `to_json(...)` and `from_json(...)` functions will need to be in the same namespace as the custom type that is to be serialized. With regard for the example shown above, that would be the `sample` namespace.

If you'd prefer to keep some of your class's internals private, you may opt to befriend the appropriate overload of either `to_json(...)` and `from_json(...)` so that only these functions can access your private members and functions.

## Handling Nulls

If you have a JSON object or array that might contain null values, you have a few deserialization choices. If you don't have access to C++17, you may opt to serialize from, and deserialize into, a container of smart pointers.

Here's an example of serializing a container of `std::shared_ptr<std::string>` objects:

```C++
const auto container =
    std::vector<std::shared_ptr<std::string>>{ std::make_shared<std::string>("Hello"),
                                               std::make_shared<std::string>("World"),
                                               nullptr };

const auto json = json_utils::serialize_to_json(container);
```

The resultant JSON will preserve a `nullptr` as a `null` JSON value: `["Hello","World",null]`.

Deserialization is simply the reverse:

```C++
const auto container =
   json_utils::deserialize_via_sax<std::vector<std::shared_ptr<std::string>>>(json);
```

This works for both `std::shared_ptr<...>` and `std::unique_ptr<...>`.

If you have access to C++17, you may wish to use a `std::optional<...>` instead (avoiding heap allocation):

```C++
using container_type = std::vector<std::optional<int>>;

const container_type source_container = { std::optional<int>{ 101 },
                                          std::optional<int>{ 202 },
                                          std::optional<int>{ 303 },
                                          std::nullopt,
                                          std::optional<int>{ 505 } };

const auto json = json_utils::serialize_to_json(source_container);
const auto deserialization = json_utils::deserialize_via_sax<container_type>(json);
```

## Building Instructions

After checking out the source, run the following commands from the base directory to build in `debug`:

```
mkdir debug && cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make [cpp14|cpp17]
```

Building in `release` is very similar:

```
mkdir release && cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make [cpp14|cpp17]
```
