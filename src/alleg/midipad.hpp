#pragma once
#include "../core/node.hpp"
#include <string>
#include <memory>
#include <cstddef>

struct SSMIDI_PAD;

namespace ScarfSparrow
{
  class MidiPadNode : public Node
  {
  private:
    struct Deleter
    {
      void operator()(SSMIDI_PAD*) const noexcept;
    };
    std::unique_ptr<SSMIDI_PAD, Deleter> ptr;
    unsigned batch_size = 0;
    bool started = false;
    unsigned sample_scope = 0;
    unsigned pad_anchor = 0;
    std::size_t dropped_notes = 0;
  public:
    MidiPadNode(std::string const& addr, unsigned int batch_size);
    SSMIDI_PAD* get() const noexcept { return ptr.get(); }
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
    NodeEndMark endOfOutput() const noexcept override;
    std::size_t countDroppedNotes() const noexcept { return dropped_notes; }
  protected:
    unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  private:
    unsigned timestampToSample(SampleInfo const& info, unsigned x) noexcept;
  };
}
