
#include "graph.hpp"
#include <unordered_map>
#include <vector>
#include <new>
#include <limits>

namespace ScarfSparrow
{

#pragma region "ScarfSparrow/Graph / methods"
  unsigned Graph::addNode(std::unique_ptr<Node>&& node, std::string_view name)
  {
    if (nameToIndex(name) < boxes.size() || !node)
      return std::numeric_limits<unsigned>::max();
    auto const index = static_cast<unsigned>(boxes.size());
    boxes.emplace_back();
    auto& back = boxes.back();
    back.node = std::move(node);
    back.name = name;
    auto const out_count = node->countOutputs();
    back.output_banks.resize(out_count);
    for (unsigned i = 0; i < out_count; ++i)
    {
      auto const desc = node->describeOutput(i);
      back.output_banks[i] = Ring(desc.batch_size);
    }
    return index;
  }
#pragma endregion

#pragma region "ScarfSparrow/Graph / methods"
  bool Graph::connect(GraphPortDesc source, GraphPortDesc sink)
  {
    auto const source_index = nameToIndex(source.name);
    auto const sink_index = nameToIndex(sink.name);
    if (source_index >= boxes.size() || sink_index >= boxes.size())
      return false;
    \
  }

  unsigned Graph::nameToIndex(std::string_view name)
  {
    auto const name_str = std::string(name);
    if (auto const it = index_map.find(name_str); it != index_map.end())
      return it->second;
    return std::numeric_limits<unsigned>::max();
  }
#pragma endregion
}
