
#include "subgraph.hpp"
#include "port.hpp"
#include <vector>
#include <algorithm>
#include <cstring>

namespace ScarfSparrow
{
  class SubgraphNode::Spanner : public Node
  {
  public:
    std::vector<Sample> stash;
    PortInfo info = {};
    bool output = false;

    void clear() noexcept
    {
      std::fill(stash.begin(), stash.end(), 0);
    }
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
    unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  };

#pragma region "ScarfSparrow/SubgraphNode/Spanner / implementation(Node)"
  unsigned SubgraphNode::Spanner::countInputs() const noexcept
  {
    return output ? 0 : 1;
  }
  PortInfo SubgraphNode::Spanner::describeInput(unsigned i) const noexcept
  {
    return info;
  }
  unsigned SubgraphNode::Spanner::countOutputs() const noexcept
  {
    return output ? 1 : 0;
  }
  PortInfo SubgraphNode::Spanner::describeOutput(unsigned i) const noexcept
  {
    return info;
  }

  unsigned SubgraphNode::Spanner::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, SampleInfo const& info) noexcept
  {
    if (!inputs.empty())
    {
      auto& bucket = inputs.front();
      std::copy(bucket.begin(), bucket.end(), stash.begin());
    }
    if (!outputs.empty())
    {
      auto& bucket = outputs.front();
      std::copy(stash.begin(), stash.end(), bucket.begin());
    }
    return static_cast<unsigned>(stash.size());
  }
#pragma endregion

#pragma region "ScarfSparrow/SubgraphNode / implementation(Node)"
  unsigned SubgraphNode::countInputs() const noexcept
  {
    return static_cast<unsigned>(in_spans.size());
  }
  PortInfo SubgraphNode::describeInput(unsigned i) const noexcept
  {
    return i >= in_spans.size() ? PortInfo{} : in_spans[i];
  }
  unsigned SubgraphNode::countOutputs() const noexcept
  {
    return static_cast<unsigned>(out_spans.size());
  }
  PortInfo SubgraphNode::describeOutput(unsigned i) const noexcept
  {
    return i >= out_spans.size() ? PortInfo{} : out_spans[i];
  }
  bool SubgraphNode::reserveSamples(unsigned samples_per_frame)
  {
    for (auto const& node : in_spans)
      node->stash.resize(samples_per_frame);
    for (auto const& node : out_spans)
      node->stash.resize(samples_per_frame);
    return graph->start(samples_per_frame);
  }

  unsigned SubgraphNode::do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
    SampleInfo const& info) noexcept
  {
    // Prepare and fill spans.
    for (std::size_t i = 0; i < in_spans.size(); ++i)
    {
      auto& node = *(in_spans[i]);
      if (i >= inputs.size())
        node->clear();
      else
        std::copy(inputs[i].begin(), inputs[i].end(), node->stash.begin());
    }
    for (std::size_t i = 0; i < out_spans.size(); ++i)
      out_spans[i]->clear();

    // Run the graph.
    if (graph)
      graph->step();

    // Scatter the results.
    unsigned count = 0;
    for (std::size_t i = 0; i < outputs.size(); ++i)
    {
      auto& bucket = outputs[i];
      if (i >= out_spans.size())
      {
        std::fill(bucket.begin(), bucket.end(), 0);
        continue;
      }
      auto& node = *(out_spans[i]);
      std::copy(node.stash.begin(), node.stash.end(), bucket.begin());
      count = static_cast<unsigned>(node.stash);
    }
    return count;
  }

  NodeEndMark SubgraphNode::endOfOutput() const noexcept
  {
    return graph && graph->done() ? NodeEndMark::Done : NodeEndMark::More;
  }
#pragma endregion
}
