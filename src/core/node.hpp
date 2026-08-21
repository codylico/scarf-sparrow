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

  /** Audio graph node. */
  class Node
  {
  public:
    virtual ~Node() = default;
    virtual unsigned countInputs() const noexcept = 0;
    virtual PortInfo describeInput(unsigned i) noexcept = 0;
    virtual unsigned countOutputs() const noexcept = 0;
    virtual PortInfo describeOutput(unsigned i) noexcept = 0;
    virtual bool pendingOutput() const noexcept = 0;
    bool run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs) noexcept;
  protected:
    virtual bool do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs) noexcept = 0;
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
