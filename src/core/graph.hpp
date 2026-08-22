#pragma once
#include "node.hpp"
#include "ring.hpp"
#include "sample.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

namespace ScarfSparrow
{

  /** Node name and port index. */
  struct GraphPortDesc
  {
    std::string_view name;
    unsigned port;
  };

  /** Audio graph. */
  class Graph
  {
  private:
    struct InputLink
    {
      unsigned short src_id = static_cast<unsigned short>(-1);
      unsigned short src_port = 0;
    };
    struct Box
    {
      std::unique_ptr<Node> node;
      unsigned visit_index = 0;
      bool visited = false;
      bool finished = false;
      std::vector<Ring> output_banks;
      std::vector<InputLink> input_links;
      std::string name;
    };
    std::vector<unsigned> depth_order;
    std::vector<Box> boxes;
    std::unordered_map<std::string, unsigned> index_map;
    unsigned batch_size = 0;
    std::vector<Sample> zeroes;
    bool full_end = false;

  public:
    Graph() = default;
    unsigned addNode(std::unique_ptr<Node>&& node, std::string_view name);
    bool connect(GraphPortDesc source, GraphPortDesc sink_port);
    unsigned nameToIndex(std::string_view name);
    bool start(unsigned samples_per_frame);
    bool done() const noexcept { return full_end; }
    /** @return whether the audio graph finished. */
    bool step();
  };
}
