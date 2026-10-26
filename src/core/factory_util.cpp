
#include "factory.hpp"
#include "dimmer.hpp"
#include "polyphony.hpp"
#include "rand.hpp"
#include "saw.hpp"
#include "square.hpp"
#include "subgraph.hpp"
#include "sum.hpp"
#include "xiphos.hpp"
#include "zero.hpp"

namespace ScarfSparrow
{
#pragma region "ScarfSparrow/Factory / namespace-local"
  void factory_add_core(Factory& toolbox)
  {
    toolbox.add("ScarfSparrow::DimmerNode", []() { return new DimmerNode; });
    toolbox.add("ScarfSparrow::PolyphonyNode", []() { return new PolyphonyNode; });
    toolbox.add("ScarfSparrow::RandomNode", []() { return new RandomNode; });
    toolbox.add("ScarfSparrow::SawNode", []() { return new SawNode; });
    toolbox.add("ScarfSparrow::SquareNode", []() { return new SquareNode; });
    toolbox.add("ScarfSparrow::SubgraphNode", []() { return new SubgraphNode; });
    toolbox.add("ScarfSparrow::SumNode", []() { return new SumNode; });
    toolbox.add("ScarfSparrow::XiphosNode", []() { return new XiphosNode; });
    toolbox.add("ScarfSparrow::ZeroNode", []() { return new ZeroNode; });
  }
#pragma endregion
}
