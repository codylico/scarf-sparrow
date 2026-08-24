
#include "graph.hpp"
#include "node.hpp"
#include <unordered_map>
#include <vector>
#include <new>
#include <utility>
#include <algorithm>
#include <limits>
#include <cstring>
#include <cassert>

namespace ScarfSparrow
{
  static constexpr unsigned GraphMaxSize = std::numeric_limits<unsigned short>::max()/2;

#pragma region "ScarfSparrow/Graph / methods"
  unsigned Graph::addNode(std::unique_ptr<Node>&& node, std::string_view name)
  {
    if (nameToIndex(name) < boxes.size() || !node || boxes.size() >= GraphMaxSize)
      return std::numeric_limits<unsigned>::max();
    auto const index = static_cast<unsigned>(boxes.size());
    boxes.emplace_back();
    auto const in_count = node->countInputs();
    auto const out_count = node->countOutputs();
    auto& back = boxes.back();
    back.node = std::move(node);
    back.name = name;
    back.output_banks.resize(out_count);
    back.input_links.resize(in_count);
    index_map[std::string(name)] = index;
    return index;
  }
#pragma endregion

#pragma region "ScarfSparrow/Graph / methods"
  bool Graph::connect(GraphPortDesc source, GraphPortDesc sink)
  {
    auto const source_index = nameToIndex(source.name);
    auto const sink_index = nameToIndex(sink.name);
    if (source_index >= boxes.size() || sink_index >= boxes.size()
    ||  sink.port >= boxes[sink_index].input_links.size()
    ||  source.port >= boxes[source_index].output_banks.size())
    {
      return false;
    }
    // Check type compatibility.
    auto& sink_box = boxes[sink_index];
    auto const sink_type = sink_box.node->describeInput(sink.port);
    auto const source_type = boxes[source_index].node->describeOutput(source.port);
    if (sink_type.typ != source_type.typ)
    {
      // Incompatible.
      return false;
    }
    auto& desc = sink_box.input_links[sink.port];
    desc.src_id = source_index;
    desc.src_port = source.port;
    return true;
  }

  unsigned Graph::nameToIndex(std::string_view name)
  {
    auto const name_str = std::string(name);
    if (auto const it = index_map.find(name_str); it != index_map.end())
      return it->second;
    return std::numeric_limits<unsigned>::max();
  }

  bool Graph::start(unsigned samples_per_frame)
  {
    auto actual = std::min<unsigned short>(samples_per_frame,
        std::numeric_limits<unsigned short>::max());
    // Normalize batch size.
    for (auto const& box : boxes)
    {
      for (std::size_t i = 0; i < box.input_links.size(); ++i)
      {
        auto const want_batch = box.node->describeInput(static_cast<unsigned>(i));
        if (want_batch.batch_size > actual)
          actual = want_batch.batch_size;
      }
      for (std::size_t i = 0; i < box.output_banks.size(); ++i)
      {
        auto const want_batch = box.node->describeOutput(static_cast<unsigned>(i));
        if (want_batch.batch_size > actual)
          actual = want_batch.batch_size;
      }
    }
    if (actual == 0)
      actual = 1;
    unsigned depth_index = 0;
    depth_order.clear();
    depth_order.reserve(boxes.size());
    // Allocate sample rings.
    for (std::size_t i = 0; i < boxes.size(); ++i)
    {
      auto& box = boxes[i];
      for (auto& ring : box.output_banks)
        ring = Ring(actual);
      box.finished = false;
      box.visited = false;
      box.visit_depth = 0;
      if (box.output_banks.empty())
      {
        // mark as ending point
        depth_order.push_back(static_cast<unsigned>(i));
        box.visited = true;
      }
    }
    // Determine node ordering.
    for (std::size_t i = 0; i < boxes.size(); ++i)
    {
      if (i >= depth_order.size())
      {
        // Some nodes unreachable!
        break;
      }
      auto const current = depth_order[i];
      auto& box = boxes[current];
      auto const next_depth = static_cast<unsigned short>(box.visit_depth+1);
      for (auto j : box.input_links)
      {
        if (j.src_id >= boxes.size())
        {
          // Unconnected input.
          continue;
        }
        auto& source = boxes[j.src_id];
        if (source.visit_depth < next_depth)
          source.visit_depth = next_depth;
        if (!std::exchange(source.visited, true))
          depth_order.push_back(j.src_id);
      }
    }
    std::sort(depth_order.begin(), depth_order.end(), [&boxes = boxes](unsigned a, unsigned b)
      {
        return boxes[a].visit_depth > boxes[b].visit_depth;
      });
    batch_size = actual;
    full_end = false;
    zeroes.resize(actual);
    std::fill(zeroes.begin(), zeroes.end(), 0);
    return true;
  }

  bool Graph::step()
  {
    if (full_end)
      return true;
    std::vector<PortInput> inputs;
    std::vector<PortOutput> outputs;
    bool pending_end = true;
    bool any_end = false;
    for (auto index : depth_order)
    {
      auto& box = boxes[index];
      // Grab the inputs.
      inputs.clear();
      inputs.reserve(box.input_links.size());
      for (auto const& link : box.input_links)
      {
        if (link.src_id >= boxes.size())
          inputs.emplace_back();
        else
          inputs.push_back(boxes[link.src_id].output_banks[link.src_port].getLastInput());
      }
      // Grab the outputs.
      outputs.clear();
      outputs.reserve(box.output_banks.size());
      for (auto& bank : box.output_banks)
        outputs.push_back(bank.viewNextOutput());
      // Run the node.
      if (box.finished)
      {
        any_end = true;
        continue;
      }
      box.node->run(inputs, outputs, info);
      switch (box.node->endOfOutput())
      {
      case NodeEndMark::More:
        pending_end = false;
        break;
      case NodeEndMark::Done:
        any_end = true;
        box.finished = true;
        break;
      default:
        break;
      }
    }
    if (any_end && pending_end)
      full_end = true;
    return full_end;
  }
#pragma endregion
}
