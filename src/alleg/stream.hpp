#pragma once
#include "../core/node.hpp"
#include <memory>
#include <cstddef>

struct ALLEGRO_AUDIO_STREAM;

namespace ScarfSparrow
{
  class AllegroStreamNode : public Node
  {
  private:
    struct Deleter
    {
      void operator()(ALLEGRO_AUDIO_STREAM*) const noexcept;
    };
    std::unique_ptr<ALLEGRO_AUDIO_STREAM, Deleter> ptr;
    unsigned batch_size = 0;
  public:
    AllegroStreamNode(std::size_t fragment_count, unsigned int batch_size, unsigned freq = 44100);
    ALLEGRO_AUDIO_STREAM* get() const noexcept { return ptr.get(); }
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
    NodeEndMark endOfOutput() const noexcept override;
  protected:
    unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  };
}
