#pragma once
#include "node.hpp"
#include <unordered_map>
#include <list>
#include <vector>
#include <array>
#include <iosfwd>

namespace ScarfSparrow
{
  enum class XiphosOpCode : unsigned char
  {
    NoOp = 0,
    Call = 1,
    Tempo = 2,
    Select = 3,
    Off = 4,
    On = 5,
    Dump = 6,
    Synth = 7,
  };
  struct XiphosCommand
  {
    XiphosOpCode op;
    unsigned char arg8;
    unsigned short arg16;
    float arg32;
  };
  using XiphosCommandBuffer = std::vector<XiphosCommand>;

  /**
  * Node parses a Xiphos Rhema file into MIDI notes.
  * @todo Support configuration by JSON.
  */
  class XiphosNode : public Node
  {
  private:
    struct Counter
    {
      unsigned short func_id;
      unsigned short offset;
    };
    struct Signal
    {
      unsigned short cmd_note;
      unsigned short vol;
    };
    std::vector<XiphosCommandBuffer> defs;
    std::array<Counter, 8> stack = {};
    unsigned short stack_pos = 0;
    bool done = false;
    Counter pc = {};
    float tempo = 60;
    int note_resolve = 1;
    long ticks_remaining = 0;
    float beat_period = 0.5f;
    float measure_period = 0.5f;
    std::array<unsigned short,2> next_signal = {};

  public:
    bool parseText(std::istream& sword);
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
  protected:
    virtual unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  private:
    Signal runCommands(SampleInfo const& info);
  };
}

