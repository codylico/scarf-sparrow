#pragma once
#include "node.hpp"
#include "graph.hpp"

namespace json11
{
  class Json;
}
namespace ScarfSparrow
{
  class SubgraphNode : public Node
  {
  private:
    class Spanner;
    std::unique_ptr<Graph> graph;
    std::vector<Spanner*> in_spans;
    std::vector<Spanner*> out_spans;
  public:
    SubgraphNode() = default;
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
    bool reserveSamples(unsigned samples_per_frame) override;
    NodeEndMark endOfOutput() const noexcept override;
  protected:
    bool do_configure(std::string_view json, Factory const& toolbox) override;
    unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
    bool pre_add(json11::Json const& info, bool dir, Graph& g, std::vector<Spanner*>& stash);
  };
}
