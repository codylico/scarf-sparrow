
#include "graph.hpp"
#include <unordered_map>
#include <vector>
#include <new>
#include <limits>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/GraphRing / rule-of-zero"
  GraphRing::GraphRing(unsigned batch_size)
    : samples(batch_size*Factor)
  {
  }
#pragma endregion

#pragma region "ScarfSparrow/GraphRing / methods"
  PortOutput GraphRing::viewNextOutput(unsigned max_batch) noexcept
  {
    auto const batch = clampBatch(max_batch);
    auto const space_left = samples.size()-write_pos;
    if (batch > space_left)
    {
      // Flush down.
      auto const shift_size = std::max<unsigned>(batch, read_pos);
      std::memmove(samples.data(), samples.data()+shift_size, samples.size()-shift_size);
      read_pos = 0;
      write_pos -= shift_size;
    }
    auto const next_write = write_pos;
    write_pos += batch;
    return PortOutput{samples.data()+next_write, batch};
  }
  PortInput GraphRing::getLastInput(unsigned max_batch) const noexcept
  {
    auto const next_read = read_pos;
    auto const batch = std::min<unsigned>(clampBatch(max_batch), write_pos - read_pos);
    read_pos += batch;
    return PortInput{samples.data()+next_read, batch};
  }
#pragma endregion

#pragma region "ScarfSparrow/GraphRing / private methods"
  unsigned GraphRing::clampBatch(unsigned x) const noexcept
  {
    return x > samples.size()/Factor ? samples.size()/Factor : x;
  }
#pragma endregion

#pragma region "ScarfSparrow/Graph / methods"
  unsigned GraphRing::addNode(std::unique_ptr<Node>&& node, std::string_view name)
  {
    auto const name_str = std::string(name);
    if (index_map.find(name_str) != index_map.end() || !node)
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
      back.output_banks[i] = GraphRing(desc.batch_size);
    }
    return index;
  }
#pragma endregion
}
