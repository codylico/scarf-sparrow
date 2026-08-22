#pragma once
#include "node.hpp"
#include "sample.hpp"

namespace ScarfSparrow
{
  class Ring
  {
  private:
    std::vector<Sample> samples;
  public:
    explicit Ring(unsigned batch_size = 0);
    PortOutput viewNextOutput() noexcept;
    PortInput getLastInput() const noexcept;
    bool empty() const noexcept;
  private:
    unsigned clampBatch(unsigned x) const noexcept;
  };

}
