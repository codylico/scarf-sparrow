#include "zero.hpp"
#include <cstring>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/ZeroNode / implementation(Node)"
  unsigned ZeroNode::countInputs() const noexcept
  {
    return 0;
  }
  unsigned ZeroNode::countOutputs() const noexcept
  {
    return 1;
  }
  PortInfo ZeroNode::describeInput(unsigned) const noexcept
  {
    return {};
  }
  PortInfo ZeroNode::describeOutput(unsigned int i) const noexcept
  {
    return i == 0 ? PortInfo{ PortType::PCM, 1 } : PortInfo{};
  }
  unsigned ZeroNode::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, const SampleInfo &info) noexcept
  {
    std::memset(outputs.front().data(), 0, outputs.front().size()*sizeof(Sample));
    return static_cast<unsigned>(outputs.front().size());
  }
#pragma endregion
}
