#pragma once
#include "node.hpp"
#include "sample.hpp"

namespace ScarfSparrow
{
  /** Node outputs a square wave. */
  class SquareNode : public Node
  {
  private:
    float freq_scale = 0.0f;
    float freq_shift = 440.f;
    Sample freq_last = {};
    float pos = {};
  public:
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
  protected:
    virtual bool do_configure(std::string_view json, Factory const& factory) override;
    virtual unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  };
}

