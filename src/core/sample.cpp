#include "sample.hpp"
#include <array>
#include <cmath>

namespace ScarfSparrow
{
  static constexpr std::array<double,12> sample_pitches = {{
      0.5946035575013605, 0.6299605249474366,
      0.6674199270850172, 0.7071067811865476,
      0.7491535384383408,
      0.7937005259840998, 0.8408964152537145,
      0.8908987181403393, 0.9438743126816935,
      1.0, 1.0594630943592953,
      1.122462048309373
    }};
  static constexpr float sample_AZero = (440.f / 32.f);

#pragma region "ScarfSparrow/Sample / namespace-local"
  float sampleToFrequency(Sample s) noexcept
  {
    auto const note = (s&255u);
    return std::ldexp(sample_pitches[note%12] * sample_AZero, note/12);
  }
#pragma endregion
}
