#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include <cstddef>

namespace ScarfSparrow
{
  class Node;

  class Factory
  {
  public:
    using value_type = std::function<Node*()>;
  private:
    std::unordered_map<std::string, value_type> store;
  public:
    void add(std::string_view key, value_type f);
    std::unique_ptr<Node> make(std::string_view key) const;
  };
}
