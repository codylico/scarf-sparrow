#pragma once
#include "sample.hpp"
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
    t at_or_zero(std::size_t i) const noexcept { return i < len ? p[i] : 0; }
  };
  using PortOutput = PortSpan<Sample>;
  using PortInput = PortSpan<Sample const>;

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
