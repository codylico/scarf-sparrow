#pragma once
#include "node.hpp"
#include "sample.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

namespace ScarfSparrow
{
  class GraphRing
  {
  private:
    std::vector<Sample> samples;
    static constexpr unsigned Factor = 2;
    unsigned read_pos = 0;
    unsigned write_pos = 0;
  public:
    GraphRing(unsigned batch_size);
    PortOutput viewNextOutput(unsigned max_batch) noexcept;
    PortInput getLastInput(unsigned max_batch) const noexcept;
    bool empty() const noexcept { return read_pos >= write_pos; }
  private:
    unsigned clampBatch(unsigned x) const noexcept;
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
      std::vecotr<GraphRing> output_banks;
      std::string name;
    };
    struct Link
    {
      GraphRing input_bank;
      unsigned read_pos = 0;
      unsigned write_pos = 0;
    };
    std::vector<unsigned> depth_order;
    std::vector<Box> boxes;
    std::vector<Link> links;
    std::unordered_map<std::string, unsigned> index_map;
  public:
    Graph() = default;
    unsigned addNode(std::unique_ptr<Node>&& node, std::string_view name);
  };
}
