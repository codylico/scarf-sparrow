#include "ring.hpp"

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/Ring / rule-of-zero"
  Ring::Ring(unsigned batch_size)
    : samples(batch_size*Factor)
  {
  }
#pragma endregion

#pragma region "ScarfSparrow/Ring / methods"
  PortOutput Ring::viewNextOutput(unsigned max_batch) noexcept
  {
    auto const batch = clampBatch(max_batch);
    auto const space_left = samples.size()-write_pos;
    if (batch > space_left)
    {
      // Flush down.
      auto const shift_size = std::max<unsigned>(batch, read_pos);
      std::memmove(samples.data(), samples.data()+shift_size, samples.size()-shift_size);
      read_pos = 0;
      write_pos -= shift_size;
    }
    auto const next_write = write_pos;
    write_pos += batch;
    return PortOutput{samples.data()+next_write, batch};
  }
  PortInput Ring::getLastInput(unsigned max_batch) noexcept
  {
    auto const next_read = read_pos;
    auto const batch = std::min<unsigned>(clampBatch(max_batch), write_pos - read_pos);
    read_pos += batch;
    return PortInput{samples.data()+next_read, batch};
  }
#pragma endregion

#pragma region "ScarfSparrow/Ring / private methods"
  unsigned Ring::clampBatch(unsigned x) const noexcept
  {
    return x > samples.size()/Factor ? static_cast<unsigned>(samples.size()/Factor) : x;
  }
#pragma endregion
\
}
