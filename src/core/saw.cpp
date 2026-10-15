#include "saw.hpp"
#include "sample.hpp"
#include <cstring>
#include <cmath>

namespace ScarfSparrow
{
  static std::array<PortInfo,2> const saw_ports = {{
    PortInfo{ PortType::MIDI, 1},
    PortInfo{ PortType::PCM, 1}
  }};

#pragma region "ScarfSparrow/SawNode / implementation(Node)"
  unsigned SawNode::countInputs() const noexcept
  {
    return static_cast<unsigned>(saw_ports.size());
  }
  unsigned SawNode::countOutputs() const noexcept
  {
    return 1;
  }
  PortInfo SawNode::describeInput(unsigned int i) const noexcept
  {
    return i >= saw_ports.size() ? PortInfo{} : saw_ports[i];
  }
  PortInfo SawNode::describeOutput(unsigned int i) const noexcept
  {
    return i == 0 ? PortInfo{ PortType::PCM, 1 } : PortInfo{};
  }
  unsigned SawNode::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, const SampleInfo &info) noexcept
  {
    auto& front = outputs.front();
    auto const& in_data = inputs[1];
    auto const& notes_data = inputs[0];
    std::size_t i = {};
    auto const first_gap = std::min<std::size_t>(front.size(), in_data.size());
    float freq = sampleToUnit(freq_last) * freq_scale + freq_shift;
    for (i = 0; i < first_gap; ++i)
    {
      front[i] = sampleFromUnit(pos);
      freq_last = in_data[i];
      if (auto const note = notes_data.at_or_zero(i))
        freq_shift = sampleToFrequency(note);
      freq = sampleToUnit(freq_last) * freq_scale + freq_shift;
      pos = std::fmod(pos + freq/std::fmax(info.hz,1.0f), 1.0f);
    }
    for (; i < front.size(); ++i)
    {
      front[i] = sampleFromUnit(pos);
      if (auto const note = notes_data.at_or_zero(i))
      {
        freq_shift = sampleToFrequency(note);
        freq = sampleToUnit(freq_last) * freq_scale + freq_shift;
      }
      pos = std::fmod(pos + freq/std::fmax(info.hz,1.0f), 1.0f);
    }
    return static_cast<unsigned>(front.size());
  }
  bool SawNode::do_configure(std::string_view json, Factory const& factory)
  {
    return true;
  }
#pragma endregion
}

