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
    struct Box
    {
      std::unique_ptr<Node> node;
      unsigned visit_index = 0;
      bool visited = false;
      std::vector<Ring> output_banks;
      std::string name;
    };
    struct Link
    {
      Ring input_bank;
    };
    std::vector<unsigned> depth_order;
    std::vector<Box> boxes;
    std::vector<Link> links;
    std::unordered_map<std::string, unsigned> index_map;
  public:
    Graph() = default;
    unsigned addNode(std::unique_ptr<Node>&& node, std::string_view name);
    bool connect(GraphPortDesc source, GraphPortDesc sink_port);
    unsigned nameToIndex(std::string_view name);
  };
}
