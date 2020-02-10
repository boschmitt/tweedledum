#pragma once

#include <kitty/bit_operations.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operations.hpp>
#include <kitty/operators.hpp>
#include <kitty/print.hpp>
#include <iostream>
#include <fstream>

namespace tweedledum
{

inline constexpr std::uint32_t ilog2( std::uint32_t const n )
{
  return ( n > 1 ) ?  1+log2( n >> 1 ) : 0;
}

inline std::uint32_t next_pow2( std::uint32_t n )
{
  --n;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  ++n;
  return n;
}

/*! \brief A truth table interface to represent bit-vectors with less than 2^n elements. */
class partial_truth_table
{
public:
  friend void print_binary( partial_truth_table const& tt, std::ostream& os );

public:
  static partial_truth_table create_from_binary_string( std::string const& binary )
  {
    assert( binary.size() > 0u );

    partial_truth_table tt( binary.size() );

    for ( uint32_t index = 0u; index < binary.size(); ++index )
    {
      if ( binary.at( index ) == '0' )
      {
        tt.clear_bit( index );
      }
      else if ( binary.at( index ) == '1' )
      {
        tt.set_bit( index );
      }
      else
      {
        assert( false && "unknown symbol in binary string" );
      }
    }

    return tt;
  }

public:
  explicit partial_truth_table( uint32_t const num_bits )
    : _num_bits( num_bits )
    , _bits( kitty::dynamic_truth_table( ilog2( next_pow2( num_bits ) ) ) )
    , _mask( kitty::dynamic_truth_table( ilog2( next_pow2( num_bits ) ) ) )
  {
    /* initialize the mask */
    for ( auto i = _num_bits; i < _mask.num_bits(); ++i )
      kitty::set_bit( _mask, i );
    _mask = ~_mask;
  }

  explicit partial_truth_table( kitty::dynamic_truth_table const& tt, uint32_t num_bits )
    : _num_bits( num_bits )
    , _bits( tt )
    , _mask( tt )
  {
    assert( 1 << tt.num_vars() >= num_bits );
    for ( uint32_t index = 0u; index < _num_bits; ++index )
    {
      kitty::set_bit( _mask, index );
    }
  }

  void set_bit( uint32_t index )
  {
    assert( index < _num_bits );
    kitty::set_bit( _bits, index );
  }

  void clear_bit( uint32_t index )
  {
    assert( index < _num_bits );
    kitty::clear_bit( _bits, index );
  }

  bool get_bit( uint32_t index ) const
  {
    assert( index < _num_bits );
    return kitty::get_bit( _bits, index );
  }

  uint64_t count_ones() const
  {
    return kitty::count_ones( _bits );
  }

  partial_truth_table operator&( partial_truth_table const& tt ) const
  {
    assert( _num_bits == tt.num_bits() );
    partial_truth_table r( _num_bits );
    r._bits = kitty::binary_and( _bits, tt._bits );
    r._mask = _mask;
    return r;
  }

  partial_truth_table operator|( partial_truth_table const& tt ) const
  {
    assert( _num_bits == tt.num_bits() );
    partial_truth_table r( _num_bits );
    r._bits = kitty::binary_or( _bits, tt._bits );
    r._mask = _mask;
    return r;
  }

  partial_truth_table operator^( partial_truth_table const& tt ) const
  {
    assert( _num_bits == tt.num_bits() );
    partial_truth_table r( _num_bits );
    r._bits = kitty::binary_xor( _bits, tt._bits );
    r._mask = _mask;
    return r;
  }

  partial_truth_table operator~() const
  {
    partial_truth_table r( _num_bits );
    r._bits = kitty::binary_and( ~_bits, _mask );
    r._mask = _mask;
    return r;
  }

  bool operator==( partial_truth_table const& tt ) const
  {
    assert( _num_bits == tt.num_bits() );
    return ( _bits == tt._bits );
  }

  uint32_t num_bits() const
  {
    return _num_bits;
  }

  std::pair<kitty::dynamic_truth_table,kitty::dynamic_truth_table> to_isop() const
  {
    return {_bits, _mask};
  }

public:
  uint32_t _num_bits;
  kitty::dynamic_truth_table _bits;
  kitty::dynamic_truth_table _mask;
}; /* partial_truth_table */

inline void print_binary( partial_truth_table const& tt, std::ostream& os = std::cout )
{
  kitty::print_binary( tt._bits, os );
  os << ':';
  os << tt._num_bits;
}

std::vector<partial_truth_table> read_minterms_from_file( std::string const& filename )
{
  std::vector<partial_truth_table> minterms;

  std::ifstream ifs( filename, std::ios::in );
  std::string line;
  while ( std::getline( ifs, line ) )
  {
    minterms.emplace_back( partial_truth_table::create_from_binary_string( line ) );
  }
  ifs.close();

  return minterms;
}

std::vector<partial_truth_table> on_set( kitty::dynamic_truth_table const& tt )
{
  // TODO: integer_dual_logarithm( get_next_power_of_two( tt.num_vars ) ), e.g., tt.num_vars() == 3u: integer_dual_logarithm( 4 ) == 2u
  std::vector<partial_truth_table> rows;
  kitty::dynamic_truth_table minterm( tt.num_vars() );
  for ( auto i = 0; i < ( 1 << tt.num_vars() ); ++i )
  {
    if ( kitty::get_bit( tt, i ) )
    {
      rows.emplace_back( partial_truth_table( minterm, tt.num_vars() ) );
    }
    kitty::next_inplace( minterm );
  }
  return rows;
}

} // namespace tweedledum
