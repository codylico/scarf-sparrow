#pragma once
#include "node.hpp"
#include "sample.hpp"
#include <vector>

namespace ScarfSparrow
{
  /** Node dispatches MIDI events using round robin. */
  class PolyphonyNode : public Node
  {
  private:
    std::vector<unsigned short> active;
    std::size_t next = 0;
    std::size_t dropped_notes = 0;
  public:
    PolyphonyNode(unsigned concurrency = 1);
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
    std::size_t countDroppedNotes() const noexcept { return dropped_notes; }
  protected:
    virtual bool do_configure(std::string_view json) override;
    virtual unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  private:
    void postOff(Sample cmd, Sample arg2, std::size_t offset, std::vector<PortOutput> const& outputs) noexcept;
    void postOn(Sample cmd, Sample arg2, std::size_t offset, std::vector<PortOutput> const& outputs) noexcept;
    void broadcast(Sample cmd, Sample arg2, std::size_t offset, std::vector<PortOutput> const& outputs) noexcept;
  };
}

