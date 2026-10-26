
#include "subgraph.hpp"
#include "factory.hpp"
#include "port.hpp"
#include "../../deps/json11-master/json11.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstring>

namespace ScarfSparrow
{
  struct SubPort
  {
    std::string name;
    int port;
  };
  class SubgraphNode::Spanner : public Node
  {
  public:
    std::vector<Sample> stash;
    PortInfo info = {};
    bool output = false;

    void clear() noexcept
    {
      std::fill(stash.begin(), stash.end(), 0);
    }
    unsigned countInputs() const noexcept override;
    PortInfo describeInput(unsigned i) const noexcept override;
    unsigned countOutputs() const noexcept override;
    PortInfo describeOutput(unsigned i) const noexcept override;
    unsigned do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
      SampleInfo const& info) noexcept override;
  };

  GraphPortDesc subport_parse(std::string const& v)
  {
    std::size_t pos = v.find(':');
    GraphPortDesc out = {};
    out.name = std::string_view(v).substr(0, pos);
    if (pos < v.size())
    {
      auto const pre_port = std::stoi(v.substr(pos+1));
      if (pre_port < 0)
        throw std::out_of_range("Negative port not admissible");
      out.port = static_cast<unsigned>(pre_port);
    }
    return out;
  }

#pragma region "ScarfSparrow/SubgraphNode/Spanner / implementation(Node)"
  unsigned SubgraphNode::Spanner::countInputs() const noexcept
  {
    return output ? 0 : 1;
  }
  PortInfo SubgraphNode::Spanner::describeInput(unsigned i) const noexcept
  {
    return info;
  }
  unsigned SubgraphNode::Spanner::countOutputs() const noexcept
  {
    return output ? 1 : 0;
  }
  PortInfo SubgraphNode::Spanner::describeOutput(unsigned i) const noexcept
  {
    return info;
  }

  unsigned SubgraphNode::Spanner::do_run(std::vector<PortInput> const& inputs,
    std::vector<PortOutput> const& outputs, SampleInfo const& info) noexcept
  {
    if (!inputs.empty())
    {
      auto& bucket = inputs.front();
      std::copy(bucket.begin(), bucket.end(), stash.begin());
    }
    if (!outputs.empty())
    {
      auto& bucket = outputs.front();
      std::copy(stash.begin(), stash.end(), bucket.begin());
    }
    return static_cast<unsigned>(stash.size());
  }
#pragma endregion

#pragma region "ScarfSparrow/SubgraphNode / implementation(Node)"
  bool SubgraphNode::do_configure(std::string_view json, Factory const& toolbox)
  {
    std::string err;
    auto cfg = json11::Json::parse(std::string(json), err);
    if (!err.empty())
      return false;
    auto new_graph = std::make_unique<Graph>();
    std::map<std::string, std::string> configs;
    std::vector<Spanner*> new_in_spans;
    std::vector<Spanner*> new_out_spans;
    /*
    format:
      {
        "inputs": [],
        "outputs": [
          {"name":"
        ],
        "nodes": [
          {"name": "a_name",
          "class": "saw",
          "config": []}
        ],
        "edges": [
          { "source": ":0", "sink": "a_name:0" },
          { "source": "a_name:0", "sink": ":0" },
        ]
      }
    */
    if (!(cfg.is_object()))
      return false;
    auto items = cfg.object_items();
    auto const node_list_it = items.find("nodes");
    auto const edge_list_it = items.find("edges");
    if (node_list_it == items.end() || edge_list_it == items.end())
      return false;
    for (auto const& item : node_list_it->second.array_items())
    {
      // Grab the type.
      auto& node_object = item.object_items();
      auto const type_it = node_object.find("class");
      auto const name_it = node_object.find("name");
      if (type_it == node_object.end() || name_it == node_object.end())
        return false;
      std::string type = type_it->second.string_value();
      std::string name = name_it->second.string_value();
      if (type.empty() || name.empty() || name.find(':') < name.size())
        return false;
      auto node = toolbox.make(type);
      if (!node)
        continue;
      // Grab the data.
      auto const sub_cfg_it = node_object.find("config");
      if (sub_cfg_it != node_object.end())
      {
        auto const sub_cfg_text = sub_cfg_it->second.dump();
        if (!node->configure(sub_cfg_text, toolbox))
          return false;
      }
      // Grab the name.
      new_graph->addNode(std::move(node), name);
    }

    auto const input_list_it = items.find("inputs");
    auto const output_list_it = items.find("outputs");
    if (input_list_it != items.end() && !pre_add(input_list_it->second, true, *new_graph, new_in_spans))
      return false;
    if (output_list_it != items.end() && !pre_add(output_list_it->second, false, *new_graph, new_out_spans))
      return false;

    // Iterate over edges.
    for (auto const& link : edge_list_it->second.array_items())
    {
      auto& edge_object = link.object_items();
      auto const source_it = edge_object.find("source");
      auto const sink_it = edge_object.find("sink");
      if (source_it == edge_object.end() || sink_it == edge_object.end())
        return false;
      try {
        auto const source = subport_parse(source_it->second.string_value());
        auto const sink = subport_parse(sink_it->second.string_value());
        if (source.name.empty() || sink.name.empty())
          throw std::invalid_argument("Bad edge name");
        if (!new_graph->connect(source, sink))
          return false;
      } catch (std::invalid_argument const&) {
        continue;
      } catch (std::out_of_range const&) {
        continue;
      }
    }

    // Post to storage
    graph = std::move(new_graph);
    in_spans = std::move(new_in_spans);
    out_spans = std::move(new_out_spans);
    return true;
  }
  unsigned SubgraphNode::countInputs() const noexcept
  {
    return static_cast<unsigned>(in_spans.size());
  }
  PortInfo SubgraphNode::describeInput(unsigned i) const noexcept
  {
    return i >= in_spans.size() ? PortInfo{} : in_spans[i]->info;
  }
  unsigned SubgraphNode::countOutputs() const noexcept
  {
    return static_cast<unsigned>(out_spans.size());
  }
  PortInfo SubgraphNode::describeOutput(unsigned i) const noexcept
  {
    return i >= out_spans.size() ? PortInfo{} : out_spans[i]->info;
  }
  bool SubgraphNode::reserveSamples(unsigned samples_per_frame)
  {
    for (auto const& node : in_spans)
      node->stash.resize(samples_per_frame);
    for (auto const& node : out_spans)
      node->stash.resize(samples_per_frame);
    return graph->start(samples_per_frame);
  }

  unsigned SubgraphNode::do_run(std::vector<PortInput> const& inputs, std::vector<PortOutput> const& outputs,
    SampleInfo const& info) noexcept
  {
    // Prepare and fill spans.
    for (std::size_t i = 0; i < in_spans.size(); ++i)
    {
      auto& node = *(in_spans[i]);
      if (i >= inputs.size())
        node.clear();
      else
        std::copy(inputs[i].begin(), inputs[i].end(), node.stash.begin());
    }
    for (std::size_t i = 0; i < out_spans.size(); ++i)
      out_spans[i]->clear();

    // Run the graph.
    if (graph)
      graph->step();

    // Scatter the results.
    unsigned count = 0;
    for (std::size_t i = 0; i < outputs.size(); ++i)
    {
      auto& bucket = outputs[i];
      if (i >= out_spans.size())
      {
        std::fill(bucket.begin(), bucket.end(), 0);
        continue;
      }
      auto& node = *(out_spans[i]);
      std::copy(node.stash.begin(), node.stash.end(), bucket.begin());
      count = static_cast<unsigned>(node.stash.size());
    }
    return count;
  }

  NodeEndMark SubgraphNode::endOfOutput() const noexcept
  {
    return graph && graph->done() ? NodeEndMark::Done : NodeEndMark::More;
  }
#pragma endregion

#pragma region "ScarfSparrow/SubgraphNode / private"
  bool SubgraphNode::pre_add(json11::Json const& info, bool dir, Graph &g,
    std::vector<Spanner*>& stash)
  {
    for (auto const& item : info.array_items())
    {
      // Grab the type.
      auto& node_object = item.object_items();
      auto const type_it = node_object.find("type");
      auto const name_it = node_object.find("name");
      if (type_it == node_object.end() || name_it == node_object.end())
        return false;
      std::string type = type_it->second.string_value();
      std::string name = name_it->second.string_value();
      if (type.empty() || name.empty() || name.find(':') < name.size())
        return false;
      auto node = std::make_unique<Spanner>();
      node->info.typ = portTypeFromString(type);
      node->info.batch_size = 1;
      node->output = dir;
      node->stash.resize(1, 0);
      auto const ptr = node.get();
      // Grab the name.
      g.addNode(std::move(node), name);
      stash.push_back(ptr);
    }
    return true;
  }
#pragma endregion
}
