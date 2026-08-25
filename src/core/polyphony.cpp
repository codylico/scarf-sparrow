#include "polyphony.hpp"
#include "sample.hpp"
#include <vector>
#include <algorithm>
#include <cstring>

namespace ScarfSparrow
{
  static std::array<PortInfo,2> const polyphony_ports = {{
    PortInfo{ PortType::MIDI, 1},
    PortInfo{ PortType::MIDIArg2, 1},
  }};

#pragma region "ScarfSparrow/PolyphonyNode / method"
  PolyphonyNode::PolyphonyNode(unsigned concurrency)
    : active(concurrency < 1 ? 1 : std::min<unsigned>(concurrency, 16))
  {
  }

  void PolyphonyNode::postOff(Sample cmd, Sample arg2,
    std::size_t offset, std::vector<PortOutput> const& outputs) noexcept
  {
    // Recover index of output node processing this note.
    Sample on_cmd = cmd | 0x1000;
    for (std::size_t i = 0; i < active.size(); ++i)
    {
      if (active[i] != on_cmd)
        continue;
      active[i] = 0;
      auto& output = outputs[i*2];
      auto& arg2_output = outputs[i*2+1];
      if (offset < output.size())
        output[offset] = cmd;
      if (offset < arg2_output.size())
        arg2_output[offset] = arg2;
    }
  }

  void PolyphonyNode::postOn(Sample cmd, Sample arg2,
    std::size_t offset, std::vector<PortOutput> const& outputs) noexcept
  {
    // Allocate a spot for this note.
    for (std::size_t unadj_i = 0; unadj_i < active.size(); ++unadj_i)
    {
      auto const adj_i = (unadj_i + next) % active.size();
      if (active[adj_i] != 0)
        continue;
      next = adj_i;
      active[adj_i] = cmd;
      auto& output = outputs[adj_i*2];
      auto& arg2_output = outputs[adj_i*2+1];
      if (offset < output.size())
        output[offset] = cmd;
      if (offset < arg2_output.size())
        arg2_output[offset] = arg2;
      return;
    }
    dropped_notes += 1;
  }

  void PolyphonyNode::broadcast(Sample cmd, Sample arg2,
    std::size_t offset, std::vector<PortOutput> const& outputs) noexcept
  {
    for (std::size_t i = 0; i < outputs.size(); ++i)
    {
      auto& output = outputs[i];
      if (offset >= output.size())
        continue;
      output[offset] = (i % 2 ? arg2 : cmd);
    }
  }
#pragma endregion

#pragma region "ScarfSparrow/PolyphonyNode / implementation(Node)"
  unsigned PolyphonyNode::countInputs() const noexcept
  {
    return static_cast<unsigned>(polyphony_ports.size());
  }
  unsigned PolyphonyNode::countOutputs() const noexcept
  {
    return static_cast<unsigned>(polyphony_ports.size() * active.size());
  }
  PortInfo PolyphonyNode::describeInput(unsigned int i) const noexcept
  {
    return i >= polyphony_ports.size() ? PortInfo{} : polyphony_ports[i];
  }
  PortInfo PolyphonyNode::describeOutput(unsigned int i) const noexcept
  {
    return i/2 < active.size() ? polyphony_ports[i%2] : PortInfo{};
  }
  unsigned PolyphonyNode::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, const SampleInfo &info) noexcept
  {
    auto const& in_data = inputs.front();
    for (std::size_t i = 0; i < in_data.size(); ++i)
    {
      auto const cmd = in_data[i];
      if (!cmd)
        continue;
      switch (cmd >> 12)
      {
      case 8: // off
        postOff(cmd, inputs[1].at_or_zero(i), i, outputs);
        break;
      case 9: // on
        postOn(cmd, inputs[1].at_or_zero(i), i, outputs);
        break;
      case 12: // preset change
        broadcast(cmd, inputs[1].at_or_zero(i), i, outputs);
        break;
      }
    }
    return static_cast<unsigned>(in_data.size());
  }
  bool PolyphonyNode::do_configure(std::string_view json)
  {
    return true;
  }
#pragma endregion
}

