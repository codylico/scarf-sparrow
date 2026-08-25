
#include "midipad.hpp"
#include "../midi/pad.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro.h>
#include <array>
#include <iostream>

namespace ScarfSparrow
{
  static std::array<PortInfo,2> const midipad_ports = {{
    PortInfo{ PortType::MIDI, 1},
    PortInfo{ PortType::MIDIArg2, 1}
  }};

#pragma region "ScarfSparrow/MidiPadNode / methods"
  void MidiPadNode::Deleter::operator()(SSMIDI_PAD *ptr) const noexcept
  {
    if (ptr)
      ssmidi_destroy_pad(ptr);
  }
#pragma endregion

#pragma region "ScarfSparrow/MidiPadNode / methods"
  MidiPadNode::MidiPadNode(std::string const& addr, unsigned frag_samples)
  : ptr(ssmidi_create_pad(addr.c_str())),
    batch_size(frag_samples)
  {
  }

  unsigned MidiPadNode::timestampToSample(SampleInfo const& info, unsigned x) noexcept
  {
    return static_cast<unsigned>(
      ((x - pad_anchor) * static_cast<unsigned long long>(info.hz)) / 65536);
  }
#pragma endregion

#pragma region "ScarfSparrow/MidiPadNode / implementation(Node)"
  unsigned MidiPadNode::countInputs() const noexcept
  {
    return 0;
  }
  PortInfo MidiPadNode::describeInput(unsigned i) const noexcept
  {
    return {};
  }
  unsigned MidiPadNode::countOutputs() const noexcept
  {
    return static_cast<unsigned>(midipad_ports.size());
  }
  PortInfo MidiPadNode::describeOutput(unsigned i) const noexcept
  {
    return i >= midipad_ports.size() ? PortInfo{} : midipad_ports[i];
  }
  NodeEndMark MidiPadNode::endOfOutput() const noexcept
  {
    return ptr ? NodeEndMark::Unknown : NodeEndMark::Done;
  }

  unsigned MidiPadNode::do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept
  {
    if (!ptr)
      return false;
    SSMIDI_PAD_NOTE next_note = {};
    auto& primary = outputs.front();
    auto& secondary = outputs[1];
    unsigned bump = 0;
    auto const primary_size = primary.size();
    while (ssmidi_pad_get_note(ptr.get(), &next_note))
    {
      if (!started)
      {
        sample_scope = 0;
        pad_anchor = next_note.timestamp;
        started = true;
      }
      auto const pos = timestampToSample(info, next_note.timestamp);
      auto biased_pos = pos - sample_scope;
      if (sample_scope + bump - pos < primary_size)
      {
        biased_pos = bump;
      }
      else if (biased_pos >= primary.size())
      {
        // Dropped note. Too bad.
        dropped_notes += 1;
        continue;
      }
      primary[biased_pos] = (next_note.cmd << 8) | next_note.args[0];
      if (biased_pos < secondary.size())
        secondary[biased_pos] = ((next_note.cmd << 4) & 0xF00) | next_note.args[1];
      bump = biased_pos + 1;
    }
    sample_scope += primary.size();
    return static_cast<unsigned>(primary.size());
  }
#pragma endregion
}
