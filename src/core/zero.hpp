#pragma once
#include "node.hpp"

namespace ScarfSparrow
{
  /** Node outputs zero samples (i.e. silence). */
  class ZeroNode : public Node
  {
  public:
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
  protected:
    virtual bool do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  };
}
