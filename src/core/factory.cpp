
#include "factory.hpp"
#include "node.hpp"
#include <unordered_map>
#include <string>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/Factory / methods"
  void Factory::add(std::string_view key, value_type f)
  {
    store[std::string(key)] = f;
  }

  std::unique_ptr<Node> Factory::make(std::string_view key) const
  {
    auto const it = store.find(std::string(key));
    if (it == store.end() || !it->second)
      return {};
    return std::unique_ptr<Node>(it->second());
  }
#pragma endregion
}
