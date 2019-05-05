#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include "rapid-json-utils.h"

namespace sample
{
    struct Foo
    {
        explicit Foo(std::string keyName) : m_key{ std::move(keyName) }
        {
        }

        std::string get_key() const noexcept
        {
            return m_key;
        }

      private:
        std::string m_key;
    };
} // namespace sample

namespace sample
{
    template <typename Writer> void to_json(Writer& writer, const sample::Foo& foo)
    {
        writer.StartObject();
        writer.Key("Key");
        writer.String(foo.get_key().c_str());
        writer.EndObject();
    }

    std::string to_narrow_json_key(const sample::Foo& foo) noexcept
    {
        return foo.get_key();
    }
} // namespace sample

int main()
{
    // clang-format off
  const std::vector<std::pair<sample::Foo, std::list<std::shared_ptr<std::string>>>> container =
  {
    {
      sample::Foo{ "Test" },
      {
        std::make_unique<std::string>("1"),
        std::make_unique<std::string>("2"),
        std::make_unique<std::string>("3"),
        std::make_unique<std::string>("4"),
        std::make_unique<std::string>("5"),
        nullptr
      }
    },
    {
      sample::Foo{ "Test" },
      {
        nullptr,
        std::make_unique<std::string>("5"),
        std::make_unique<std::string>("6"),
        std::make_unique<std::string>("7"),
        std::make_unique<std::string>("8"),
        std::make_unique<std::string>("9")
      }
    }
  };
    // clang-format on

    const auto json1 = json_utils::serialize_to_pretty_json(container);
    std::cout << json1 << std::endl;

    // const auto foo = fds::Foo("This is a test");
    // const auto json2 = json_utils::serialize_to_pretty_json(foo);
    // std::cout << json2 << std::endl;

    std::vector<std::pair<std::string, double>> vector = { { "1", 1.0 }, { "4", 4.0 } };
    const auto json3 = json_utils::serialize_to_pretty_json(vector);
    const auto deserialized = json_utils::deserialize_from_json<decltype(vector)>(json3);

    // std::map<std::string, std::vector<std::pair<std::string, double>>> map;
    // map.emplace("Key1", std::vector<std::pair<std::string, double>>{ { "1", 1.0 } });
    // map.emplace("Key2", std::vector<std::pair<std::string, double>>{ { "4", 4.0 } });

    // const auto json4 = json_utils::serialize_to_pretty_json(map);
    // const auto deserialized =
    //   json_utils::deserialize_from_json<decltype(map)>(json3);

    // for (auto& vector : deserialized) {
    //  for (auto& item : vector) { std::cout << item << std::endl; }
    //}

    return 0;
}
