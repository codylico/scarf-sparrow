#include "dimmer.hpp"
#include "sample.hpp"
#include <cstring>
#include <cmath>

namespace ScarfSparrow
{
  static std::array<PortInfo,2> const dimmer_ports = {{
    PortInfo{ PortType::PCM, 1},
    PortInfo{ PortType::MIDIArg2, 1},
  }};

#pragma region "ScarfSparrow/DimmerNode / implementation(Node)"
  DimmerNode::DimmerNode(float scale, float shift)
    : volume_scale(scale), volume_shift(shift)
  {
  }
#pragma endregion
#pragma region "ScarfSparrow/DimmerNode / implementation(Node)"
  unsigned DimmerNode::countInputs() const noexcept
  {
    return static_cast<unsigned>(dimmer_ports.size());
  }
  unsigned DimmerNode::countOutputs() const noexcept
  {
    return 1;
  }
  PortInfo DimmerNode::describeInput(unsigned int i) const noexcept
  {
    return i >= dimmer_ports.size() ? PortInfo{} : dimmer_ports[i];
  }
  PortInfo DimmerNode::describeOutput(unsigned int i) const noexcept
  {
    return i == 0 ? PortInfo{ PortType::PCM, 1 } : PortInfo{};
  }
  unsigned DimmerNode::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, const SampleInfo &info) noexcept
  {
    auto& front = outputs.front();
    auto const& in_data = inputs.front();
    auto const& notes_data = inputs[1];
    std::size_t i = {};
    auto const first_gap = std::min<std::size_t>(front.size(), in_data.size());
    float volume = sampleToVolume(volume_last) * volume_scale + volume_shift;
    for (i = 0; i < first_gap; ++i)
    {
      if (auto const next_vol = notes_data.at_or_zero(i))
      {
        volume_last = next_vol;
        volume = sampleToVolume(volume_last) * volume_scale + volume_shift;
      }
      front[i] = sampleFromUnitChecked(volume * sampleToUnit(in_data[i]));
    }
    for (; i < front.size(); ++i)
    {
      front[i] = 0;
      if (auto const next_vol = notes_data.at_or_zero(i))
        volume_last = next_vol;
    }
    volume = sampleToVolume(volume_last) * volume_scale + volume_shift;
    return static_cast<unsigned>(front.size());
  }
  bool DimmerNode::do_configure(std::string_view json)
  {
    return true;
  }
#pragma endregion
}

