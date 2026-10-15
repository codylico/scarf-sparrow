#include "sum.hpp"
#include "sample.hpp"
#include <vector>
#include <algorithm>
#include <cstring>

namespace ScarfSparrow
{
  static std::array<PortInfo,2> const polyphony_ports = {{
    PortInfo{ PortType::PCM, 1},
    PortInfo{ PortType::MIDIArg2, 1},
  }};

#pragma region "ScarfSparrow/SumNode / method"
  SumNode::SumNode(unsigned concurrency)
    : concurrency_(concurrency)
  {
  }
#pragma endregion

#pragma region "ScarfSparrow/SumNode / implementation(Node)"
  unsigned SumNode::countInputs() const noexcept
  {
    return concurrency_;
  }
  unsigned SumNode::countOutputs() const noexcept
  {
    return 1;
  }
  PortInfo SumNode::describeInput(unsigned int i) const noexcept
  {
    return i >= concurrency_ ? PortInfo{} : PortInfo{PortType::PCM, 1};
  }
  PortInfo SumNode::describeOutput(unsigned int i) const noexcept
  {
    return i == 0 ? PortInfo{PortType::PCM, 1} : PortInfo{};
  }
  unsigned SumNode::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, const SampleInfo &info) noexcept
  {
    auto const& front = outputs.front();
    for (std::size_t i = 0; i < front.size(); ++i)
    {
      unsigned short total = 0;
      for (auto& input : inputs)
        total += input.at_or_zero(i);
      front[i] = total;
    }
    return static_cast<unsigned>(front.size());
  }
  bool SumNode::do_configure(std::string_view json, Factory const& factory)
  {
    return true;
  }
#pragma endregion
}

