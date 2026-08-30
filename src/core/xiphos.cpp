#include "xiphos.hpp"
#include "port.hpp"
#include <istream>
#include <sstream>
#include <unordered_map>
#include <list>
#include <string>
#include <array>
#include <stdexcept>
#include <charconv>
#include <utility>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <cmath>

namespace ScarfSparrow
{
  static constexpr double xiphos_to_ticks(SampleInfo const& info, float secs)
  {
    return static_cast<double>(info.hz) * secs;
  }
  using XiphosLine = std::array<std::string,3>;
  using XiphosLineList = std::list<XiphosLine>;
  using XiphosTextMap = std::unordered_map<std::string, XiphosLineList>;
  using XiphosNameMap = std::unordered_map<std::string, std::size_t>;

  static std::array<std::string_view,12> const xiphos_order = {{
      "c","cx","d","dx","e","f","fx","g","gx","a","ax","b"
    }};
  static std::array<PortInfo,2> const xiphos_ports = {{
    PortInfo{ PortType::MIDI, 1},
    PortInfo{ PortType::MIDIArg2, 1}
  }};


#pragma region "ScarfSparrow/XiphosNode / static"
  static std::string xiphos_or_default(std::string_view x, std::string_view dflt)
  {
    return x.empty() ? std::string(dflt) : std::string(x);
  }
  static unsigned char xiphos_make_note(std::string_view nm)
  {
    constexpr int self_octave = 3;
    // split name and octave
    std::size_t i = 0;
    for (; i < nm.size(); ++i)
    {
      if (std::isdigit(nm[i] & 255u) || std::memchr("-+", nm[i], 2))
        break;
    }
    auto name = std::string(nm.substr(0,i));
    for (char& ch : name)
      ch = std::tolower(ch & 255u);
    int octave = 0;
    std::from_chars(nm.data()+i, nm.data()+nm.size(), octave);
    auto const it = std::find(xiphos_order.begin(), xiphos_order.end(), name);
    if (it == xiphos_order.end())
      throw std::invalid_argument("ScarfSparrow::Xiphos: invalid tone name");
    return (it-xiphos_order.begin())+12*(octave+5-self_octave);
  }

  static XiphosLine xiphos_split(std::string const& x)
  {
    XiphosLine total = {};
    std::string next;
    std::size_t i = 0;
    for (char ch : x)
    {
      if (std::isspace(ch))
      {
        total[i] = std::move(next);
        i += 1;
        if (i >= total.size())
          return total;
        next.clear();
        continue;
      }
      next.push_back(ch);
    }
    if (!next.empty())
      total[i] = std::move(next);
    return total;
  }

  static bool xiphos_read_text_list(XiphosTextMap& map, std::istream& sword)
  {
    std::string current;
    while (sword)
    {
      std::string line;
      std::getline(sword, line);
      auto cut = xiphos_split(line);
      if (cut.front().empty() || cut.front().front() == '#')
        continue;
      else if (cut.front() == "def")
      {
        if (cut[1].empty() || !current.empty())
          return false;
        map[cut[1]].clear();
        current = cut[1];
      }
      else if (cut.front() == "end")
      {
        if (current.empty())
          return true;
        current = {};
      }
      else if (cut.front() == "done")
        return true;
      else
      {
        map[current].push_back(std::move(cut));
      }
    }
    return true;
  }

  bool xiphos_compile(XiphosCommandBuffer& out, XiphosLineList const& texts, XiphosNameMap const& names)
  {
    for (auto& line : texts)
    {
      // Compile.
      auto const& cmd = line.front();
      if (cmd == "on")
      {
        out.push_back({XiphosOpCode::On, xiphos_make_note(line[1]),
          0, std::stof(xiphos_or_default(line[2],"1.0"))});
      }
      else if (cmd == "off")
        out.push_back({XiphosOpCode::Off, xiphos_make_note(line[1])});
      else if (cmd == "select")
        out.push_back({XiphosOpCode::Select});
      else if (cmd == "notes")
        out.push_back({XiphosOpCode::Dump});
      else if (cmd == "call")
      {
        if (line[1].empty())
          return false;
        auto const it = names.find(line[1]);
        if (it == names.end())
          return false;
        out.push_back({XiphosOpCode::Call, 0, static_cast<unsigned short>(it->second)});
      }
      else if (cmd == "synth")
      {
        out.push_back({XiphosOpCode::Synth,
          static_cast<unsigned char>(xiphos_or_default(line[2],"b").front()),
          0, std::stof(xiphos_or_default(line[1],"1.0"))});
      }
      else if (cmd == "tempo")
      {
        out.push_back({XiphosOpCode::Tempo, 0,
          static_cast<unsigned short>(std::stoi(line[2])), std::stof(line[1])});
      }
      else if (cmd == "add")
      {
        /* do nothing */;
      }
      else
        return false;
    }
    return true;
  }
  static long xiphos_add_timeframe(SampleInfo const& info, XiphosCommand const& cmd,
    float beat_period, float measure_period, double& residual) noexcept
  {
    auto const here_units = cmd.arg8;
    auto const count = cmd.arg32;
    double xcount = 0.0;
    switch (here_units)
    {
    case 'b':
      xcount = xiphos_to_ticks(info, count*beat_period);
      break;
    case 'm':
      xcount = xiphos_to_ticks(info, count*measure_period);
      break;
    case 's':
      xcount = xiphos_to_ticks(info, count);
      break;
    default:
      xcount = count;
      break;
    }
    double n_count = 0.0;
    residual = std::modf(xcount + residual, &n_count);
    return static_cast<long>(n_count);
  }

  static void xiphos_recalc_periods(float& tempo, int note_resolve, float& beat_period, float& measure_period) noexcept
  {
    if (tempo < 1.f)
        tempo = 1.f;
    beat_period = 60/tempo;
    measure_period = beat_period*note_resolve;
  }
#pragma endregion

#pragma region "ScarfSparrow/XiphosNode / implementation(Node)"
  bool XiphosNode::parseText(std::istream& sword)
  {
    // Parse to string tuple lists.
    std::unordered_map<std::string, std::list<XiphosLine>> text_lists;
    text_lists.try_emplace("");
    if (!xiphos_read_text_list(text_lists, sword))
      return false;
    // Compile the strings.
    XiphosNameMap names;
    try {
      defs.resize(text_lists.size());
      // Put the main command list at position zero.
      names[""] = 0;
      std::size_t counter = 1;
      for (auto& [k,v] : text_lists)
      {
        if (k.empty())
          continue;
        names[k] = counter;
        counter += 1;
      }
      for (auto& [k,v] : names)
      {
        if (!xiphos_compile(defs[v], text_lists[k], names))
          return false;
      }
    } catch (std::invalid_argument const&) {
      return false;
    }
    // Set up the stack.
    std::fill(stack.begin(), stack.end(), Counter{});
    stack_pos = 0;
    pc = {};
    return true;
  }
#pragma endregion

#pragma region "ScarfSparrow/XiphosNode / implementation(Node)"
  unsigned XiphosNode::countInputs() const noexcept
  {
    return 0;
  }
  unsigned XiphosNode::countOutputs() const noexcept
  {
    return static_cast<unsigned>(xiphos_ports.size());
  }
  PortInfo XiphosNode::describeInput(unsigned) const noexcept
  {
    return {};
  }
  PortInfo XiphosNode::describeOutput(unsigned int i) const noexcept
  {
    return i >= xiphos_ports.size() ? PortInfo{} : xiphos_ports[i];
  }
  NodeEndMark XiphosNode::endOfOutput() const noexcept
  {
    return done ? NodeEndMark::Done : NodeEndMark::More;
  }
  unsigned XiphosNode::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, const SampleInfo &info) noexcept
  {
    auto& front = outputs.front();
    auto& vol = outputs[1];
    for (std::size_t i = 0; i < front.size(); ++i)
    {
      if (ticks_remaining > 0)
      {
        front[i] = 0;
        vol[i] = 0;
        ticks_remaining -= 1;
        continue;
      }
      auto const sig = runCommands(info);
      front[i] = sig.cmd_note;
      vol[i] = (sig.cmd_note ? (256 | sig.vol) : 0);
    }
    return static_cast<unsigned>(front.size());
  }
#pragma endregion

#pragma region "ScarfSparrow/XiphosNode / private methods"
  XiphosNode::Signal XiphosNode::runCommands(SampleInfo const& info)
  {
    if (done)
      return {};
    constexpr unsigned MaxIterations = 2048;
    for (unsigned iteration = 0; iteration < MaxIterations; ++iteration)
    {
      // Finish functions.
      for (std::size_t j = 0; j <= stack.size(); ++j)
      {
        if (pc.offset < defs[pc.func_id].size() || done)
          break;
        else if (stack_pos == 0)
        {
          done = true;
          return {};
        }
        stack_pos -= 1;
        pc = stack[stack_pos];
      }
      auto const& fn = defs[pc.func_id];
      auto const& cmd = fn[pc.offset];
      pc.offset += 1;
      switch (cmd.op)
      {
      case XiphosOpCode::Call:
        if (stack_pos < stack.size() && cmd.arg16 < defs.size())
        {
          stack[stack_pos] = pc;
          pc.func_id = cmd.arg16;
          pc.offset = 0;
          stack_pos += 1;
          continue;
        }
        // Let the counter increment.
        break;
      case XiphosOpCode::Synth:
        // Compute the timeframe.
        ticks_remaining += xiphos_add_timeframe(info, cmd, beat_period, measure_period, tick_residual);
        if (ticks_remaining > 0)
          return {};
        break;
      case XiphosOpCode::Tempo:
        // Compute new tempo.
        tempo = cmd.arg32;
        note_resolve = cmd.arg16;
        xiphos_recalc_periods(tempo, note_resolve, beat_period, measure_period);
        break;
      case XiphosOpCode::On:
        ticks_remaining -= 1;
        return {static_cast<Sample>(0x9000 | cmd.arg8), static_cast<Sample>(256.f+cmd.arg32*127.f)};
      case XiphosOpCode::Off:
        ticks_remaining -= 1;
        return {static_cast<Sample>(0x8000 | cmd.arg8), 0x100};
      default:
        break;
      }
    }
    return {};
  }
#pragma endregion
}
