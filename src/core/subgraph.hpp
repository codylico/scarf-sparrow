#pragma once
#include "node.hpp"
#include "graph.hpp"

namespace ScarfSparrow
{
  class SubgraphNode : public Node
  {
  private:
    class Spanner;
    std::unique_ptr<Graph> graph;
    std::vector<std::unique_ptr<Spanner>> in_spans;
    std::vector<std::unique_ptr<Spanner>> out_spans;
  public:
    SubgraphNode() = default;
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
    bool reserveSamples(unsigned samples_per_frame) override;
    NodeEndMark endOfOutput() const noexcept override;
  protected:
    unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept;
  };
}
