
#include "node.hpp"
#include <utility>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/Node / interface"
  bool Node::configure(std::string_view json)
  {
    if (!std::exchange(once, true))
      once = do_configure(json);
    return once;
  }
  unsigned Node::run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
    SampleInfo const& info) noexcept
  {
    if (inputs.size() < countInputs() || outputs.size() < countOutputs())
      return 0;
    return do_run(inputs, outputs, info);
  }

  NodeEndMark Node::endOfOutput() const noexcept
  {
    return NodeEndMark::Unknown;
  }

  bool Node::do_configure(std::string_view json)
  {
    return true;
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
