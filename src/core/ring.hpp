#pragma once
#include "node.hpp"
#include "sample.hpp"

namespace ScarfSparrow
{
  class Ring
  {
  private:
    std::vector<Sample> samples;
    static constexpr unsigned Factor = 2;
    unsigned read_pos = 0;
    unsigned write_pos = 0;
  public:
    explicit Ring(unsigned batch_size = 0);
    PortOutput viewNextOutput(unsigned max_batch) noexcept;
    PortInput getLastInput(unsigned max_batch) noexcept;
    bool empty() const noexcept { return read_pos >= write_pos; }
  private:
    unsigned clampBatch(unsigned x) const noexcept;
  };

}
