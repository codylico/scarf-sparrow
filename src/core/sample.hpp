#pragma once

namespace ScarfSparrow
{
  using Sample = unsigned short;

  struct SampleInfo
  {
    unsigned hz;
  };

  constexpr float sampleToUnit(Sample s)
  {
    return static_cast<short>(s) / 32768.f;
  }
}
