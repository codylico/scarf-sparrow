#pragma once
#include "node.hpp"
#include "sample.hpp"
#include <vector>

namespace ScarfSparrow
{
  /** Node adds signal together. */
  class SumNode : public Node
  {
  private:
    unsigned concurrency_ = 1;
  public:
    SumNode(unsigned concurrency = 1);
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

