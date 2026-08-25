#pragma once
#include "node.hpp"
#include "sample.hpp"

namespace ScarfSparrow
{
  /** Node controls a note's presence or absence. */
  class DimmerNode : public Node
  {
  private:
    float volume_scale = 1.0f;
    float volume_shift = 0.0f;
    Sample volume_last = {};
  public:
    explicit DimmerNode(float scale = 1.0f, float shift = 0.f);
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
  protected:
    virtual bool do_configure(std::string_view json) override;
    virtual unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  };
}

