#pragma once
#include "sample.hpp"
#include <vector>
#include <string_view>
#include <cstddef>

namespace ScarfSparrow
{
  enum class PortType : unsigned char
  {
    Unknown = 0,
    PCM = 1,
    MIDI = 2,
    Text = 3,
  };
  struct PortInfo
  {
    PortType typ;
    /** Number of samples accepted at once */
    unsigned short batch_size;
  };

  /** `std::span<unsigned short>` shim. */
  template <typename t>
  class PortSpan
  {
  private:
    t* p = nullptr;
    std::size_t len = 0;
  public:
    using value_type = t;
    constexpr PortSpan() = default;
    constexpr PortSpan(t* dta, std::size_t sz) : p(dta), len(sz) {}
    t* data() const noexcept { return p; }
    std::size_t size() const noexcept { return len; }
    t& operator[](std::size_t i) const noexcept { return p[i]; }
  };
  using PortOutput = PortSpan<Sample>;
  using PortInput = PortSpan<Sample const>;

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

  /**
  * @param name type name
  * @return corresponding type constant, or Unknown on failure
  */
  PortType portTypeFromString(std::string_view name) noexcept;
  /**
  * @param t port type code
  * @return string describing port type, or empty string on failure
  */
  std::string_view portTypeToString(PortType t) noexcept;
}
