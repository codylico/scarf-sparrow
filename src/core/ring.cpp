#include "ring.hpp"
#include <cstring>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/Ring / rule-of-zero"
  Ring::Ring(unsigned batch_size)
    : samples(batch_size)
  {
  }
#pragma endregion

#pragma region "ScarfSparrow/Ring / methods"
  PortOutput Ring::viewNextOutput() noexcept
  {
    std::memset(samples.data(), 0, samples.size()*sizeof(Sample));
    return PortOutput{samples.data(), samples.size()};
  }
  PortInput Ring::getLastInput() const noexcept
  {
    return PortInput{samples.data(), samples.size()};
  }
  bool Ring::empty() const noexcept
  {
    return samples.empty();
  }
#pragma endregion
}
