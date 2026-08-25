#pragma once

namespace ScarfSparrow
{
  using Sample = unsigned short;
  using SampleU32 = decltype(0xFFffFFffu);

  struct SampleInfo
  {
    unsigned hz;
  };

  constexpr float sampleToUnit(Sample s)
  {
    return static_cast<short>(s) / 32767.f;
  }
  constexpr Sample sampleFromUnitChecked(float x)
  {
    return static_cast<Sample>(x > 1.0f ? 32767.f : (x < -1.0f ? -32767.f : x * 32767.f));
  }
  constexpr Sample sampleFromUnit(float x)
  {
    return static_cast<Sample>(x * 32767.f);
  }
  constexpr float sampleToVolume(Sample x)
  {
    return (x&127u) / 127.f;
  }
  float sampleToFrequency(Sample s) noexcept;
}
