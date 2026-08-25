
#include "node.hpp"
#include <utility>

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/Port / namespace-local"
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
