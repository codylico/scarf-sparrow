#pragma once
#include <string_view>

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
    /** Number of samples accepted at once, or zero for "give me everything now". */
    unsigned short batch_size;
  };
  class Node
  {
  public:
    virtual ~Node() = default;
    virtual unsigned countInputs() const noexcept = 0;
    virtual PortInfo describeInput(unsigned i) noexcept = 0;
    virtual unsigned countOutputs() const noexcept = 0;
    virtual PortInfo describeOutput(unsigned i) noexcept = 0;
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
