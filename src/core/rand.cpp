#include "rand.hpp"
#include <cstring>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/RandomNode / implementation(Node)"
  unsigned RandomNode::countInputs() const noexcept
  {
    return 0;
  }
  unsigned RandomNode::countOutputs() const noexcept
  {
    return 1;
  }
  PortInfo RandomNode::describeInput(unsigned) const noexcept
  {
    return {};
  }
  PortInfo RandomNode::describeOutput(unsigned int i) const noexcept
  {
    return i == 0 ? PortInfo{ PortType::PCM, 1 } : PortInfo{};
  }
  unsigned RandomNode::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, const SampleInfo &info) noexcept
  {
    auto& front = outputs.front();
    for (std::size_t i = 0; i < front.size(); ++i)
      front[i] = filter(prng);
    return static_cast<unsigned>(front.size());
  }
#pragma endregion
}
