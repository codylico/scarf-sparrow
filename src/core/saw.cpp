#include "saw.hpp"
#include "sample.hpp"
#include <cstring>
#include <cmath>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/SawNode / implementation(Node)"
  unsigned SawNode::countInputs() const noexcept
  {
    return 1;
  }
  unsigned SawNode::countOutputs() const noexcept
  {
    return 1;
  }
  PortInfo SawNode::describeInput(unsigned int i) const noexcept
  {
    return i == 0 ? PortInfo{ PortType::PCM, 1 } : PortInfo{};
  }
  PortInfo SawNode::describeOutput(unsigned int i) const noexcept
  {
    return i == 0 ? PortInfo{ PortType::PCM, 1 } : PortInfo{};
  }
  unsigned SawNode::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, const SampleInfo &info) noexcept
  {
    auto& front = outputs.front();
    auto const& in_data = inputs.front();
    std::size_t i = {};
    auto const first_gap = std::min<std::size_t>(front.size(), in_data.size());
    for (i = 0; i < first_gap; ++i)
    {
      front[i] = sampleFromUnit(pos);
      freq_last = in_data[i];
      float const freq = sampleToUnit(freq_last) * freq_scale + freq_shift;
      pos = std::fmod(pos + freq/std::fmax(info.hz,1.0f), 1.0f);
    }
    float const finish_freq = sampleToUnit(freq_last) * freq_scale + freq_shift;
    for (; i < front.size(); ++i)
    {
      front[i] = sampleFromUnit(pos);
      pos = std::fmod(pos + finish_freq/std::fmax(info.hz,1.0f), 1.0f);
    }
    return static_cast<unsigned>(front.size());
  }
  bool SawNode::do_configure(std::string_view json)
  {
    return true;
  }
#pragma endregion
}

