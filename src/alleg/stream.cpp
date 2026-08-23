
#include "stream.hpp"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro.h>
#include <array>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/AllegroStreamNode / methods"
  void AllegroStreamNode::Deleter::operator()(ALLEGRO_AUDIO_STREAM *ptr) const noexcept
  {
    if (ptr)
      al_destroy_audio_stream(ptr);
  }
#pragma endregion

#pragma region "ScarfSparrow/AllegroStreamNode / methods"
  AllegroStreamNode::AllegroStreamNode(std::size_t fragment_count, unsigned int frag_samples, unsigned freq)
    : ptr(al_create_audio_stream(fragment_count, frag_samples, freq, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2)),
    batch_size(frag_samples)
  {
  }
#pragma endregion

#pragma region "ScarfSparrow/AllegroStreamNode / implementation(Node)"
  unsigned AllegroStreamNode::countInputs() const noexcept
  {
    return 2;
  }
  PortInfo AllegroStreamNode::describeInput(unsigned i) const noexcept
  {
    if (i >= 2)
      return {};
    return { PortType::PCM, static_cast<unsigned short>(batch_size) };
  }
  unsigned AllegroStreamNode::countOutputs() const noexcept
  {
    return 0;
  }
  PortInfo AllegroStreamNode::describeOutput(unsigned i) const noexcept
  {
    return {};
  }
  NodeEndMark AllegroStreamNode::endOfOutput() const noexcept
  {
    return ptr && al_get_audio_stream_playing(ptr.get()) ? NodeEndMark::More : NodeEndMark::Done;
  }

  unsigned AllegroStreamNode::do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept
  {
    if (!ptr)
      return false;
    auto* const buffer = static_cast<Sample*>(al_get_audio_stream_fragment(ptr.get()));
    if (!buffer)
      return false;
    std::array<std::size_t,2> sizes = {{inputs[0].size(), inputs[1].size()}};
    for (int i = 0; i < batch_size; ++i)
    {
      auto* const sample = &buffer[i*2];
      for (std::size_t j = 0; j < sizes.size(); ++j)
        sample[j] = (i < sizes[j]) ? inputs[j][i] : 0;
    }
    al_set_audio_stream_fragment(ptr.get(), buffer);
    return static_cast<unsigned>(sizes[0] > sizes[1] ? sizes[0] : sizes[1]);
  }
#pragma endregion
}
