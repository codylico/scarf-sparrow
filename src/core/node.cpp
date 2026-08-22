
#include "node.hpp"

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/Node / interface"
  bool Node::run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs) noexcept
  {
    if (inputs.size() < countInputs() || outputs.size() < countOutputs())
      return false;
    return do_run(inputs, outputs);
  }

  NodeEndMark Node::endOfOutput() const noexcept
  {
    return NodeEndMark::Unknown;
  }
#pragma endregion

#pragma region "ScarfSparrow/Node / namespace-local"
  PortType portTypeFromString(std::string_view name) noexcept
  {
    if (name == "text")
      return PortType::Text;
    else if (name == "pcm")
      return PortType::PCM;
    else if (name == "midi")
      return PortType::MIDI;
    return {};
  }

  std::string_view portTypeToString(PortType t) noexcept
  {
    switch (t)
    {
    case PortType::Text:
      return "text";
    case PortType::PCM:
      return "pcm";
    case PortType::MIDI:
      return "midi";
    default:
      return {};
    }
  }
#pragma endregion
}
