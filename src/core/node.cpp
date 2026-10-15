
#include "node.hpp"
#include "port.hpp"
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

  bool Node::reserveSamples(unsigned samples_per_frame)
  {
    return true;
  }
#pragma endregion
}
