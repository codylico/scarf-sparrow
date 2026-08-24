#include "sample.hpp"
#include <array>
#include <cmath>

namespace ScarfSparrow
{
  static constexpr std::array<float,12> sample_pitches = {{
      0.5946035f, 0.6299605f,
      0.6674199f, 0.7071067f,
      0.7491535f,
      0.7937005f, 0.8408964f,
      0.8908987f, 0.9438743f,
      1.0, 1.0594631f,
      1.1224620f
    }};
  static constexpr float sample_AZero = (440.f / 32.f);

#pragma region "ScarfSparrow/Sample / namespace-local"
  float sampleToFrequency(Sample s) noexcept
  {
    auto const note = (s&255u);
    return std::ldexp(sample_pitches[note%12] * sample_AZero, static_cast<int>(note)/12);
  }
#pragma endregion
}
