#pragma once
#include "port.hpp"
#include "sample.hpp"
#include <vector>
#include <string_view>
#include <cstddef>

namespace ScarfSparrow
{
  /** End-of-file signal from a node. */
  enum class NodeEndMark : signed char
  {
    /** Node can't say whether it finished yet. */
    Unknown = -1,
    /** i.e. wants more samples */
    More = 0,
    /** End of stream for this node. */
    Done = 1
  };

  /** Audio graph node. */
  class Node
  {
  private:
    bool once = false;
  public:
    virtual ~Node() = default;
    bool configure(std::string_view json);
    virtual unsigned countInputs() const noexcept = 0;
    virtual PortInfo describeInput(unsigned i) const noexcept = 0;
    virtual unsigned countOutputs() const noexcept = 0;
    virtual PortInfo describeOutput(unsigned i) const noexcept = 0;
    virtual NodeEndMark endOfOutput() const noexcept;
    /**
    * @param input input buffers to process
    * @param[out] outputs buffers to fill
    * @param info content information
    * @return maximum number of samples generated in any output buffer
    */
    unsigned run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept;
  protected:
    virtual bool do_configure(std::string_view json);
    virtual unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept = 0;
  };
}
