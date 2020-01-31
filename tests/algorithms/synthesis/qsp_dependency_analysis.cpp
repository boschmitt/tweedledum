/* author: Fereshte */
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/qsp_dependency_analysis.hpp>
#include <kitty/constructors.hpp>

using namespace tweedledum;

TEST_CASE("Create order", "[qsp_dependency_analysis]")
{
  CHECK( create_order( 0u ) == order_t{} );
  CHECK( create_order( 1u ) == order_t{ 0u } );
  CHECK( create_order( 2u ) == order_t{ 1u, 0u } );
  CHECK( create_order( 3u ) == order_t{ 2u, 1u, 0u } );
  CHECK( create_order( 4u ) == order_t{ 3u, 2u, 1u, 0u } );
}

TEST_CASE("Compute dependencies", "[qsp_dependency_analysis]")
{
  kitty::dynamic_truth_table tt(3);
  kitty::create_from_binary_string(tt, "10000001");
 
  order_t order = create_order( 3u );

  qsp_dependency_analysis_stats stats;
  auto const& deps = qsp_dependency_analysis( tt, order, stats );
  CHECK( deps == dependencies_t{} );
}
