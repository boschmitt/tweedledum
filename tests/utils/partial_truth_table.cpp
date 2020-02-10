/* author: Fereshte */
#include <catch.hpp>
#include <tweedledum/utils/partial_truth_table.hpp>
#include <kitty/constructors.hpp>
#include <sstream>

using namespace tweedledum;

TEST_CASE("Create partial truth table", "[partial_truth_table]")
{
  partial_truth_table ptt( 10u );
  CHECK( ptt.num_bits() == 10u );
  CHECK( ptt.count_ones() == 0u );
}

TEST_CASE("Create from dynamic truth table", "[partial_truth_table]")
{
  kitty::dynamic_truth_table mt0( 2 ); /* num_vars = 2 --> size = 2^2 = 4 */
  kitty::create_from_binary_string( mt0, "0000" );
  partial_truth_table ptt_mt0( mt0, 3 ); /* num_bits = 3 --> size = 3 */
  CHECK( ptt_mt0.num_bits() == 3u );

  kitty::dynamic_truth_table mt1( 2 );
  kitty::create_from_binary_string( mt1, "0111" );
  partial_truth_table ptt_mt1( mt1, 3 );
  CHECK( ptt_mt1.num_bits() == 3u );
}

// TEST_CASE("Create from too small truth table", "[partial_truth_table]")
// {
//   kitty::dynamic_truth_table tt( 2 );
//   partial_truth_table ptt( tt, 5u );
//   CHECK( ptt.num_bits() == 5u ); /* should fail */
// }

TEST_CASE("Create from string", "[partial_truth_table]")
{
  auto const mt0 = partial_truth_table::create_from_binary_string( "000" );
  CHECK( mt0.num_bits() == 3u );
  
  auto const mt1 = partial_truth_table::create_from_binary_string( "111" );
  CHECK( mt1.num_bits() == 3u );
}

TEST_CASE("Binary operations", "[partial_truth_table]")
{
  auto const a = partial_truth_table::create_from_binary_string( "0001" );
  auto const b = partial_truth_table::create_from_binary_string( "1011" );
  CHECK( a.num_bits() == 4u );
  CHECK( b.num_bits() == 4u );

  CHECK( a.count_ones() == 1u );
  CHECK( b.count_ones() == 3u );
  
  CHECK( ( a & b ) == partial_truth_table::create_from_binary_string( "0001" ) );
  CHECK( ( a | b ) == partial_truth_table::create_from_binary_string( "1011" ) );
  CHECK( ( a ^ b ) == partial_truth_table::create_from_binary_string( "1010" ) );
  CHECK( ( ~a ) == partial_truth_table::create_from_binary_string( "1110" ) );
  CHECK( ( ~b ) == partial_truth_table::create_from_binary_string( "0100" ) );
}

TEST_CASE("Bit operations", "[partial_truth_table]")
{
  auto a = partial_truth_table::create_from_binary_string( "1100" );
  a.clear_bit( 0 );
  CHECK( ( a ) == partial_truth_table::create_from_binary_string( "0100" ) );

  a.clear_bit( 1 );  
  CHECK( ( a ) == partial_truth_table::create_from_binary_string( "0000" ) );

  a.set_bit( 2 );  
  CHECK( ( a ) == partial_truth_table::create_from_binary_string( "0010" ) );  

  a.set_bit( 3 );  
  CHECK( ( a ) == partial_truth_table::create_from_binary_string( "0011" ) );    

  CHECK( a.get_bit( 0 ) == false ); 
  CHECK( a.get_bit( 1 ) == false ); 
  CHECK( a.get_bit( 2 ) == true ); 
  CHECK( a.get_bit( 3 ) == true ); 
}

TEST_CASE("Print binary", "[partial_truth_table]")
{
  auto const a = partial_truth_table::create_from_binary_string( "110" );

  std::stringstream ss;
  print_binary( a, ss );

  CHECK( ss.str() == "0011:3" ); /* TODO: should be "011" */
}

TEST_CASE( "Compute on_set", "[partial_truth_table]" )
{
  kitty::dynamic_truth_table tt( 6 );
  kitty::create_from_binary_string( tt, "0101010111010101010100001101000000000101010001010000000001000000" );

  auto const minterms = on_set( tt );
  CHECK( minterms.size() == kitty::count_ones( tt ) );
}
