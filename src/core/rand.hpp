#pragma once
#include "node.hpp"
#include <random>

namespace ScarfSparrow
{
  /** Node outputs zero samples (i.e. silence). */
  class RandomNode : public Node
  {
  private:
    std::minstd_rand prng;
    std::uniform_int_distribution<unsigned short> filter;
  public:
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
  protected:
    virtual unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  };
}

