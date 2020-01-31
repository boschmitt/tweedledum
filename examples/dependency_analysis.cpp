#include <tweedledum/algorithms/synthesis/qsp_tt_dependencies.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/algorithms/generic/rewrite.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/qasm.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>

#include <kitty/kitty.hpp>
#include <percy/percy.hpp>
#include <fmt/format.h>
#include <fstream>

#include <dirent.h>
// #include <stdlib.h>
// #include <sys/types.h>

constexpr std::uint32_t ilog2( std::uint32_t const n )
{
  return ( n > 1 ) ?  1+log2( n >> 1 ) : 0;
}

std::uint32_t next_pow2( std::uint32_t n )
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
    return {_bits,_mask};
  }
public:
  uint32_t _num_bits;
  kitty::dynamic_truth_table _bits;
  kitty::dynamic_truth_table _mask;
}; /* partial_truth_table */

void print_binary( partial_truth_table const& tt, std::ostream& os = std::cout )
{
  kitty::print_binary( tt._bits, os );
  std::cout << ':';
  std::cout << tt._num_bits;
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

  //std::cout << tt.num_vars() << std::endl;
  do
  {
    if ( minterm._bits[0] > ( 1u << tt.num_vars() ) )
      break;

    if ( kitty::get_bit( tt, minterm._bits[0] ))
    {
      rows.emplace_back( partial_truth_table( minterm, tt.num_vars() ) );
    }

    kitty::next_inplace( minterm );
  } while ( !kitty::is_const0( minterm ) );
  return rows;
}

/*---chech that there isn't any dependency ----*/
bool check_not_exist_dependencies( std::vector<partial_truth_table> minterms, uint32_t target )
{
    uint32_t const num_minterms = minterms.size();

    //std::cout<<"target: "<<target<<std::endl;

    for ( auto i = 0 ; i < int32_t( minterms.size() ) ; i++ )
    {
        for(int32_t j =target-1 ; j>=0 ; j--)
        {
            //std::cout<<"j clear: "<<j<<std::endl;
            minterms[i].clear_bit(j);
        }
    }

    for(auto k = 0u ; k<num_minterms-1 ; k++)
    {
        auto row1 = minterms[k];
        auto row2 = minterms[k+1];

        // std::cout<<"row 1: ";
        // print_binary(row1);
        // std::cout<<std::endl;

        // std::cout<<"row 2: ";
        // print_binary(row2);
        // std::cout<<std::endl;

        auto check = row1^row2;

        // std::cout<<"check: ";
        // print_binary(check);
        // std::cout<<std::endl;

        if ( check.count_ones()==1 && check.get_bit( target ) )
        {
          return true;
        }
    }

    return false;
}


using dependencies_t = std::map<uint32_t , std::vector<std::pair<std::string, std::vector<uint32_t>>>>;

struct functional_dependency_stats
{
  uint32_t num_analysis_calls{0};
  uint32_t has_no_dependencies{0};
  uint32_t no_dependencies_computed{0};
  uint32_t has_dependencies{0};
  uint32_t funcdep_bench_useful{0};
  uint32_t funcdep_bench_notuseful{0};
  
  double total_time{0};

  uint32_t total_cnots{0};
  uint32_t total_rys{0};
};

dependencies_t functional_dependency_analysis( kitty::dynamic_truth_table const& tt, functional_dependency_stats& stats, std::vector<uint32_t> orders )
{
  ++stats.num_analysis_calls;

  /* extract minterms */
  std::vector<partial_truth_table> minterms = on_set( tt );

  /* convert minterms to column vectors */
  uint32_t const minterm_length = minterms[0u].num_bits();
  uint32_t const num_minterms = minterms.size();

  std::vector<partial_truth_table> columns( minterm_length, partial_truth_table( num_minterms ) );
  for ( auto i = 0u; i < minterm_length; ++i )
  {
    for ( auto j = 0u; j < num_minterms; ++j )
    {
      if ( minterms.at( j ).get_bit( orders[i] ) )
      {
        columns[minterm_length-i-1].set_bit( j );
      }
    }
  }

  /* resubstitution-style dependency analysis */
  dependencies_t dependencies;

  uint32_t has_no_dependencies = 0u;
  for ( auto mirror_i = 0; mirror_i < int32_t( columns.size() ); ++mirror_i )
  {
    auto i = columns.size() - mirror_i - 1;

       /*---check that there isn't any dependency---*/
    if ( i < minterm_length - 2 )
    {
      if ( check_not_exist_dependencies( minterms, i ) )
      {
        ++has_no_dependencies;
        continue;
      }
    }

    bool found = false;
    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      if ( columns.at( i ) == columns.at( j ) )
      {
        found = true;
        dependencies[i] = std::vector{ std::pair{ std::string{"eq"}, std::vector<uint32_t>{ uint32_t( j*2+0 ) } } };
        break;
      }
    }

    /* go to next column if a solution has been found for this column */
    if ( found )
      continue;

    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      if ( columns.at( i ) == ~columns.at( j ) )
      {
        found = true;
        dependencies[i] = std::vector{ std::pair{ std::string{"not"}, std::vector<uint32_t>{ uint32_t( j*2+0 ) } } };
        break;
      }
    }

    /* go to next column if a solution has been found for this column */
    if ( found )
      continue;

 
    //-----xor---------
    //----first input
    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      //-----second input
      for ( auto k = j - 1; k > i; --k )
      {
        if ( columns.at( i ) == ( columns.at( j ) ^ columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"xor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( ~ (columns.at( j ) ^ columns.at( k ) ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"xnor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ) } } };
          break;
        }

        //-----3rd input
        for(auto l = j-2 ; l>i ; --l)
        {
          if ( columns.at( i ) == ( columns.at( j ) ^ columns.at( k ) ^ columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"xor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( ~(columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ) ) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"xnor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ) } } };
            break;
          }
          //---- 4th input
          for(auto m = j-3 ; m>i ; --m)
          {
            if ( columns.at( i ) == ( columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ^ columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"xor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 )} } };
              break;
            }
            else if ( columns.at( i ) == ( ~(columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ^ columns.at(m) ) ) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"xnor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ) , uint32_t( k*2+0 ) , uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }

            //---- 5th input
            for(auto i5 = j-4 ; i5>i ; --i5)
            {
              if ( columns.at( i ) == ( columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ^ columns.at(m) ^ columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"xor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~(columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ^ columns.at(m) ^ columns.at(i5) ) ) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"xnor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
            }
            if (found)
                break;
          }
          if (found)
            break;
        }
        if (found)
            break;
      }
      if ( found )
        break;
    }

    if ( found )
      continue;

    //-----and---------
    //-----first input
    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      //----second input
      for ( auto k = j - 1; k > i; --k )
      {
        if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( ~ (columns.at( j ) & columns.at( k )) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"nand"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ) } } };
          break;
        }

        //----3rd input
        for(auto l = j-2 ; l>i ; --l)
        {
          if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 )} } };
            break;
          }
          else if ( columns.at( i ) == ( ~(columns.at( j ) & columns.at( k ) & columns.at(l) ) )  )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"nand"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 )} } };
            break;
          }
          else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 )} } };
            break;
          }
          else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 )} } };
            break;
          }
          else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & ~columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 )} } };
            break;
          }
          else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & ~columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 )} } };
            break;
          }
          else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & ~columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 )} } };
            break;
          }
          else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & ~columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 )} } };
            break;
          }

          //---- 4th input
          for(auto m = j-3 ; m>i ; --m)
          {
            if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~(columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) ) ) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"nand"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & ~columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & ~columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l) & ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & columns.at(l) & ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & columns.at(l) & ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & columns.at(l) & ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & ~columns.at(l) & ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & ~columns.at(l) & ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ) } } };
              break;
            }

            //---- 5th input
            for(auto i5 = j-4 ; i5>i ; --i5)
            {
              if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~(columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) & columns.at(i5) ) ) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"nand"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & ~columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & ~columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l) & ~columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & columns.at(l) & ~columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & columns.at(l) & ~columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & columns.at(l) & ~columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & ~columns.at(l) & ~columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & ~columns.at(l) & ~columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & ~columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & ~columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
                else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & columns.at(l) & columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & columns.at(l) & columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & ~columns.at(l) & columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & ~columns.at(l) & columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l) & ~columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & columns.at(l) & ~columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & columns.at(l) & ~columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & columns.at(l) & ~columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & ~columns.at(l) & ~columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & columns.at( k ) & ~columns.at(l) & ~columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & ~columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) & ~columns.at( k ) & ~columns.at(l) & ~columns.at(m) & ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"and"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
            }
            if (found)
                break;
          }
          if (found)
                break;
        }
        if (found)
            break;
      }

      if ( found )
        break;
    }
    if ( found )
      continue;

    //-----or---------
    //-----first input
    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      //-----second input
      for ( auto k = j - 1; k > i; --k )
      {
        if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( ~ (columns.at( j ) | columns.at( k )) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"nor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ) } } };
          break;
        }
        else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ) } } };
          break;
        }

        //------3rd input
        for(auto l = j-2 ; l>i ; --l)
        {
          if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( ~(columns.at( j ) | columns.at( k ) | columns.at(l) ) )  )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"nor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | ~columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | ~columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | ~columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ) } } };
            break;
          }
          else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | ~columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ) } } };
            break;
          }

          //---- 4th input
          for(auto m = j-3 ; m>i ; --m)
          {
            if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~(columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) ) ) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"nor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }

            else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | ~columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | ~columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l) | ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | columns.at(l) | ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | columns.at(l) | ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | columns.at(l) | ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | ~columns.at(l) | ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | ~columns.at(l) | ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ) } } };
              break;
            }
            else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | ~columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ) } } };
              break;
            }

            //---- 5th input
            for(auto i5 = j-4 ; i5>i ; --i5)
            {
              if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~(columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) | columns.at(i5) ) ) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"nor"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | ~columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | ~columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l) | ~columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | columns.at(l) | ~columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | columns.at(l) | ~columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | columns.at(l) | ~columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | ~columns.at(l) | ~columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | ~columns.at(l) | ~columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | ~columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | ~columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+0 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }

              else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | columns.at(l) | columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | columns.at(l) | columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | ~columns.at(l) | columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | ~columns.at(l) | columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+0 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l) | ~columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | columns.at(l) | ~columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | columns.at(l) | ~columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | columns.at(l) | ~columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+0 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | ~columns.at(l) | ~columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | columns.at( k ) | ~columns.at(l) | ~columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+0 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | ~columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+0 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
              else if ( columns.at( i ) == ( ~columns.at( j ) | ~columns.at( k ) | ~columns.at(l) | ~columns.at(m) | ~columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::vector{ std::pair{ std::string{"or"}, std::vector<uint32_t>{ uint32_t( j*2+1 ), uint32_t( k*2+1 ), uint32_t( l*2+1 ), uint32_t( m*2+1 ), uint32_t( i5*2+1 ) } } };
                break;
              }
            }
            if (found)
                break;
          }
          if (found)
                break;
        }
        if (found)
                break;
      }

      if ( found )
        break;
    }
    if ( found )
      continue;

    //-----and xor---------
    //-----first input
    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      //-----second input
      for ( auto k = j - 1; k > i; --k )
      {
        //------3rd input
        for(auto l = j-2 ; l>i ; --l)
        {
          auto in1 = j;
          auto in2 = k;
          auto in3 = l;
          if  ( columns.at( i ) == ( (columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ~( (columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xnor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( (~columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( columns.at(in1) & ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( ~columns.at(in1) & ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }

          in1 = l;
          in2 = k;
          in3 = j;
          if  ( columns.at( i ) == ( (columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ~( (columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xnor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( (~columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( columns.at(in1) & ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( ~columns.at(in1) & ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }

          in1 = j;
          in2 = l;
          in3 = k;
          if  ( columns.at( i ) == ( (columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ~( (columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xnor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( (~columns.at( in1 ) & columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( columns.at(in1) & ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( ~columns.at(in1) & ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }
          
        }
        if (found)
                break;
      }
      if(found)
        break;
    }
    if (found)
        continue;

    //-----or xor---------
    //-----first input
    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      //-----second input
      for ( auto k = j - 1; k > i; --k )
      {
        //------3rd input
        for(auto l = j-2 ; l>i ; --l)
        {
          auto in1 = j;
          auto in2 = k;
          auto in3 = l;
          if  ( columns.at( i ) == ( (columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ~( (columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xnor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( (~columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( columns.at(in1) | ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( ~columns.at(in1) | ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }

          in1 = l;
          in2 = k;
          in3 = j;
          if  ( columns.at( i ) == ( (columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ~( (columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xnor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( (~columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( columns.at(in1) | ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( ~columns.at(in1) | ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }

          in1 = j;
          in2 = l;
          in3 = k;
          if  ( columns.at( i ) == ( (columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ~( (columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xnor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( (~columns.at( in1 ) | columns.at( in2 ) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+0 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( columns.at(in1) | ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+0 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }
          else if  ( columns.at( i ) == ( ( ~columns.at(in1) | ~columns.at(in2) )  ^ columns.at( in3 ) ) )  
          {
            found = true;
            dependencies[i] = std::vector{ std::pair{ std::string{"or_xor"}, std::vector<uint32_t>{ uint32_t( in1*2+1 ), uint32_t( in2*2+1 ), uint32_t( in3*2+0 )} } };
            break;
          }
          
        }
        if (found)
                break;
      }
      if(found)
        break;
    }
    if (found)
        continue;
  }

  if ( has_no_dependencies == (minterm_length - 2u) )
  {
    ++stats.has_no_dependencies;
  }
  else if ( dependencies.size() == 0u )
  {
    ++stats.no_dependencies_computed;
  }
  else if ( dependencies.size() > 0u )
  {
    ++stats.has_dependencies;
  }

  return dependencies;
}

dependencies_t exact_fd_analysis( kitty::dynamic_truth_table const& tt, functional_dependency_stats& stats )
{
  ++stats.num_analysis_calls;

  /* extract minterms */
  std::vector<partial_truth_table> minterms = on_set( tt );

  /* convert minterms to column vectors */
  uint32_t const minterm_length = minterms[0u].num_bits();
  uint32_t const num_minterms = minterms.size();

  std::vector<partial_truth_table> columns( minterm_length, partial_truth_table( num_minterms ) );
  for ( auto i = 0u; i < minterm_length; ++i )
  {
    for ( auto j = 0u; j < num_minterms; ++j )
    {
      if ( minterms.at( j ).get_bit( i ) )
      {
        columns[i].set_bit( j );
      }
    }
  }

  /* dependency analysis using exact synthesis*/
  dependencies_t dependencies;

  uint32_t has_no_dependencies = 0u;
  for ( auto mirror_i = 0; mirror_i < int32_t( columns.size() ); ++mirror_i )
  {
    auto i = columns.size() - mirror_i - 1;

    bool found = false;
    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      if ( columns.at( i ) == columns.at( j ) )
      {
        found = true;
        dependencies[i] = std::vector{ std::pair{ std::string{"eq"}, std::vector<uint32_t>{ uint32_t( j*2+0 ) } } };
        break;
      }
    }

    /* go to next column if a solution has been found for this column */
    if ( found )
      continue;

    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      if ( columns.at( i ) == ~columns.at( j ) )
      {
        found = true;
        dependencies[i] = std::vector{ std::pair{ std::string{"not"}, std::vector<uint32_t>{ uint32_t( j*2+0 ) } } };
        break;
      }
    }

    /* go to next column if a solution has been found for this column */
    if ( found )
      continue;

    /*---check that there isn't any dependency---*/
    if ( i < minterm_length - 2 )
    {
      if ( check_not_exist_dependencies( minterms, i ) )
      {
        ++has_no_dependencies;
        continue;
      }
    }

    //----first input
    for ( auto j = uint32_t( columns.size() )-1; j > i; --j )
    {
      //-----second input
      for ( auto k = j - 1; k > i; --k )
      {
        // std::cout << i << ' ' << j << ' ' << k << std::endl;

        auto const isop_i = columns.at( i ).to_isop();
        auto const isop_j = columns.at( j ).to_isop();
        auto const isop_k = columns.at( k ).to_isop();

        percy::chain chain;
        percy::spec spec;

        /* TODO #1: select the right primitives */
        // spec.set_primitive( percy::AIG );
        auto const fanin = 2;
        kitty::dynamic_truth_table tt( fanin );
        std::vector<kitty::dynamic_truth_table> inputs;
        for (int i = 0; i < fanin; i++)
        {
          inputs.push_back(kitty::create<kitty::dynamic_truth_table>(fanin));
          kitty::create_nth_var(inputs[i], i);
        }
        spec.add_primitive( inputs[0] & inputs[1] );
        spec.add_primitive( ~inputs[0] & inputs[1] );
        spec.add_primitive( inputs[0] & ~inputs[1] );
        spec.add_primitive( inputs[0] | inputs[1] );
        
        /* TODO: is the order correct? */
        spec[0] = isop_i.first;
        spec.set_dont_care( 0, ~isop_i.second );
        if ( spec.get_nr_in() < 2 )
          continue;
        
        std::vector<std::pair<std::string,std::vector<uint32_t>>> xs;
        xs.emplace_back( "column", std::vector<uint32_t>{ uint32_t( j*2+0 ) } );
        xs.emplace_back( "column", std::vector<uint32_t>{ uint32_t( k*2+0 ) } );
        
        auto const result = synthesize( spec, chain );
        if ( result == percy::success )
        {
          /* TODO: simulate the result */
          // assert( chain.is_aig() );
          // assert( chain.simulate()[0] == tt );

          for ( auto l = 0; l < chain.get_nr_steps(); ++l )
          {
            // std::cout << l << ' ' << chain.get_operator( l )._bits[0] << ' ' << chain.get_step( l )[0] << ' ' << chain.get_step( l )[1] << std::endl;            
            
            auto c1 = uint32_t (chain.get_step( l )[0]);
            auto c2 = uint32_t (chain.get_step( l )[1]);

            switch ( chain.get_operator( l )._bits[0] )
            {
            default:
              std::cerr << "[e] unsupported operation " << kitty::to_hex( chain.get_operator( l ) ) << "\n";
              assert( false );
              break;
            case 0x8:
              xs.emplace_back( "and", std::vector<uint32_t>{c1*2+0,c2*2+0} );
              break;
            case 0x4:
              xs.emplace_back( "and", std::vector<uint32_t>{c1*2+1,c2*2+0} );
              break;
            case 0x2:
              xs.emplace_back( "and", std::vector<uint32_t>{c1*2+0,c2*2+1} );
              break;
            case 0xe:
              xs.emplace_back( "or", std::vector<uint32_t>{c1*2+0,c2*2+0} );
              break;
            case 0x6:
              xs.emplace_back( "xor", std::vector<uint32_t>{c1*2+0,c2*2+0} );
              break;
            }
          }

          found = true;
          dependencies[i] = xs;
          break;
        }
      }

      if ( found )
        break;
    }

    if ( found )
      continue;
  }

  return dependencies;
}

std::vector<uint32_t> varaible_ordering_regarding_deps(dependencies_t deps , uint32_t num_vars)
{
    std::vector<uint32_t> orders;
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "eq")
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "not")
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "xor")
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "and" && d.second[0].second.size()<4)
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "nand" && d.second[0].second.size()<4)
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "or" && d.second[0].second.size()<4)
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "nor" && d.second[0].second.size()<4)
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "and_xor")
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "and_xnor")
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "or_xor")
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "or_xnor")
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "and" && d.second[0].second.size()<(num_vars-1) && d.second[0].second.size()>3)
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "nand" && d.second[0].second.size()<(num_vars-1) && d.second[0].second.size()>3)
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "or" && d.second[0].second.size()<(num_vars-1) && d.second[0].second.size()>3)
            orders.emplace_back(d.first);
    } 
    for ( const auto& d : deps )
    {
        if(d.second[0].first == "nor" && d.second[0].second.size()<(num_vars-1) && d.second[0].second.size()>3)
            orders.emplace_back(d.first);
    } 

    for (auto i=0u ; i<num_vars; i++)
    {
        auto it = std::find(orders.begin(), orders.end(), i);
        if(it == orders.end())
            orders.emplace_back(i);
    }
    std::reverse(orders.begin(),orders.end());
    return orders;
}

void print_dependencies( dependencies_t const& dependencies, std::ostream& os = std::cout )
{
  os << "[i] dependencies:" << std::endl;
  os << "dependencies size: " << dependencies.size() << std::endl;
  for ( const auto& d : dependencies )
  {
    os << d.first << "  ";
    for ( const auto& dd : d.second )
    {
      os << dd.first << ' ';
      for ( const auto& c : dd.second )
      {
        os << c << ' ';
      }
    }
    os << std::endl;
  }
}

void prepare_quantum_state( kitty::dynamic_truth_table const& tt, dependencies_t const& dependencies, 
functional_dependency_stats& stats, std::vector<uint32_t> orders )
{
  tweedledum::netlist<tweedledum::mcst_gate> ntk;
  //auto const binary_tt = kitty::to_binary( tt );

  qsp_tt_statistics qsp_stats;

  // dependencies_t no_deps;
  // qsp_tt_dependencies( ntk, binary_tt, no_deps, qsp_stats );

  /* TODO: need to be modified */
  std::reverse(orders.begin(),orders.end());
  qsp_tt_dependencies( ntk, tt, dependencies, qsp_stats, orders);

  stats.total_time += qsp_stats.time;
  stats.funcdep_bench_useful += qsp_stats.funcdep_bench_useful;
  stats.funcdep_bench_notuseful += qsp_stats.funcdep_bench_notuseful;
  stats.total_cnots += qsp_stats.total_cnots;
  stats.total_rys += qsp_stats.total_rys;
}

void write_report( functional_dependency_stats const& stats, std::ostream& os )
{
  os << "[i] number of analyzed benchmarks = " << stats.num_analysis_calls << std::endl;
  os << fmt::format( "[i] total = no deps exist + no deps found + found deps ::: {} = {} + {} + {}\n",
                     (stats.has_no_dependencies+stats.no_dependencies_computed+
                     stats.has_dependencies), stats.has_no_dependencies, stats.no_dependencies_computed, 
                     stats.has_dependencies );
  os << fmt::format( "[i] total deps = dep useful + dep not useful ::: {} = {} + {}\n",
                     stats.funcdep_bench_useful + stats.funcdep_bench_notuseful, stats.funcdep_bench_useful, stats.funcdep_bench_notuseful);
  os << fmt::format( "[i] total synthesis time (considering dependencies) = {:8.2f}s\n",
                     stats.total_time );

  os << fmt::format( "[i] synthesis result: CNOTs / RYs = {} / {}\n",
                     stats.total_cnots, stats.total_rys );
}

/* Experiment #1:
 *
 * Run for one Boolean function
 */
void example1()
{
    std::string const inpath = "../input6/";
    functional_dependency_stats stats;
    functional_dependency_stats stats2;
    std::string filename;

    std::string tt_str;

    // std::ifstream infile;
    // infile.open( inpath + filename );
    // infile >> tt_str;
    // infile.close();

    tt_str = "10000001";

    /* prepare truth table */
    std::reverse( tt_str.begin(), tt_str.end() );
    auto const tt_vars = int( log2( tt_str.size() ) );

    kitty::dynamic_truth_table tt( tt_vars );
    kitty::create_from_binary_string( tt, tt_str );

    if(!kitty::is_const0(tt))
    {
    /* functional deps analysis */
    std::vector <uint32_t> orders_init;
    for(int32_t i = tt_vars-1 ; i>=0 ; i--)
        orders_init.emplace_back(i);

    /* without ordering */   
    // auto const deps2 = functional_dependency_analysis( tt, stats2 , orders_init);
    // if(deps2.size()>0)
    //     prepare_quantum_state( tt, deps2, stats2 , orders_init );

    /* with ordering */
    auto const deps1 = functional_dependency_analysis( tt, stats , orders_init);
    //print_dependencies(deps1);
    auto orders = varaible_ordering_regarding_deps(deps1 , tt_vars);
    for(auto i=0; i<orders.size() ; i++)
        std::cout<<orders[i]<<"  ";
    std::cout<<std::endl;
    
    auto const deps2 = functional_dependency_analysis( tt, stats2 , orders);
    std::cout<<"deps2:\n";
    print_dependencies(deps2);
    //std::cout<<"end iteration\n";
    prepare_quantum_state( tt, deps2, stats2 , orders );
    }

    std::cout << '\n';
    write_report( stats2, std::cout );
}

/* Experiment #2:
 *
 * Run quantum circuit synthesis with and without dependency analysis
 * for all k-input functions.
 */
void example2()
{
  uint32_t const num_vars = 4u;

  kitty::dynamic_truth_table tt( num_vars );

  functional_dependency_stats stats;

  /* TODO: implementation does not work for 0 */
  kitty::next_inplace( tt );

  do
  {
    std::cout << '\r';
    kitty::print_binary( tt );

    /* functional deps analysis */
    std::vector <uint32_t> orders_init;
        for(int32_t i = num_vars-1 ; i>=0 ; i--)
            orders_init.emplace_back(i);
    auto const deps = functional_dependency_analysis( tt, stats , orders_init );
    prepare_quantum_state( tt, deps, stats , orders_init );

    /* increment truth table */
    kitty::next_inplace( tt );
  } while ( !kitty::is_const0( tt ) );

  // std::ofstream report_file;
  // report_file.open("../output.txt");
  // write_report( stats, report_file );
  // report_file.close();

  std::cout << '\n';
  write_report( stats, std::cout );
}

/* Experiment #3:
 *
 * Run quantum circuit synthesis with and without dependency analysis
 * for all representatives of the NPN-k class.
 */
void example3()
{
  auto const num_vars = 4u;

  /* truth table type in this example */
  using truth_table = kitty::dynamic_truth_table;

  /* set to store all NPN representatives (dynamic to store bits on heap) */
  kitty::dynamic_truth_table map( 1 << num_vars );

  /* invert bits: 1 means not classified yet */
  std::transform( map.cbegin(), map.cend(), map.begin(), []( auto word ) { return ~word; } );

  /* hash set to store all NPN classes */
  std::unordered_set<truth_table, kitty::hash<truth_table>> classes;

  /* start from 0 */
  int64_t index = 0;
  truth_table tt( num_vars );

  while ( index != -1 )
  {
    /* create truth table from index value */
    kitty::create_from_words( tt, &index, &index + 1 );

    /* apply NPN canonization and add resulting representative to set;
       while canonization, mark all encountered truth tables in map
     */
    const auto res = kitty::exact_npn_canonization( tt, [&map]( const auto& tt ) { kitty::clear_bit( map, *tt.cbegin() ); } );
    classes.insert( std::get<0>( res ) );

    /* find next non-classified truth table */
    index = find_first_one_bit( map );
  }

  std::cout << "[i] enumerated "
            << map.num_bits() << " functions into "
            << classes.size() << " classes." << std::endl;

  functional_dependency_stats stats;
  for ( const auto& cl : classes )
  {
    if ( kitty::is_const0( cl ) )
      continue;

    /* functional deps analysis */
    std::vector <uint32_t> orders_init;
        for(int32_t i = num_vars-1 ; i>=0 ; i--)
            orders_init.emplace_back(i);
    auto const deps = functional_dependency_analysis( cl, stats , orders_init );
    prepare_quantum_state( cl, deps, stats , orders_init );
  }

  // std::ofstream report_file;
  // report_file.open("../output.txt");
  // write_report( stats, report_file );
  // report_file.close();

  write_report( stats, std::cout );
}

/* Experiment #4:
 *
 * Run quantum circuit synthesis with and without dependency analysis
 * for all files in a directory using functional dependency analysis
 */

void example4()
{
  std::string const inpath = "../input6/";
  functional_dependency_stats stats;
  functional_dependency_stats stats2;
  DIR * dir;
  struct dirent *entry;
  std::vector<std::string> tt_strs;
  if ( ( dir = opendir( inpath.c_str() ) ) )
  {
    while ( entry = readdir(dir) )
    {
      std::string filename( entry->d_name );
      if ( filename == "." || filename == ".." || filename == ".DS_Store" || filename[0]!= '6' )
        continue;

      /* read file */
      std::string tt_str;

      std::ifstream infile;
      infile.open( inpath + filename );
      infile >> tt_str;
      infile.close();

      /* check uniqueness */
      auto it = std::find(tt_strs.begin() , tt_strs.end() , tt_str);
      if(it != tt_strs.end())
        continue;
        
      tt_strs.emplace_back(tt_str);

      /* prepare truth table */
      std::reverse( tt_str.begin(), tt_str.end() );
      auto const tt_vars = int( log2( tt_str.size() ) );

      kitty::dynamic_truth_table tt( tt_vars );
      kitty::create_from_binary_string( tt, tt_str );

      if(!kitty::is_const0(tt))
      {
        /* functional deps analysis */
        std::vector <uint32_t> orders_init;
        for(int32_t i = tt_vars-1 ; i>=0 ; i--)
            orders_init.emplace_back(i);

        /* without ordering */   
        // auto const deps2 = functional_dependency_analysis( tt, stats2 , orders_init);
        // if(deps2.size()>0)
        //     prepare_quantum_state( tt, deps2, stats2 , orders_init );

        /* with ordering */
        auto const deps1 = functional_dependency_analysis( tt, stats , orders_init);
        //print_dependencies(deps1);
        auto orders = varaible_ordering_regarding_deps(deps1 , tt_vars);
        // for(auto i=0; i<orders.size() ; i++)
        //     std::cout<<orders[i]<<"  ";
        // std::cout<<std::endl;
        
        auto const deps2 = functional_dependency_analysis( tt, stats2 , orders);
        //std::cout<<"deps2:\n";
        //print_dependencies(deps2);
        //std::cout<<"end iteration\n";
        prepare_quantum_state( tt, deps2, stats2 , orders );
      }
      
    }

    closedir( dir );
  }
  

//   std::ofstream report_file;
//   report_file.open("../output.txt");
//   write_report( stats, report_file );
//   report_file.close();
  std::cout << '\n';
  write_report( stats2, std::cout );
}


/* Experiment #5:
 *
 * Run quantum circuit synthesis with and without dependency analysis
 * for all k-input functions using exact synthesis
 */


void example5()
{
  uint32_t const num_vars = 4u;

  kitty::dynamic_truth_table tt( num_vars );

  functional_dependency_stats stats;

  /* TODO: implementation does not work for 0 */
  kitty::next_inplace( tt );

  do
  {
    // std::cout << '\r';
    //kitty::print_binary( tt ); std::cout << std::endl;

    /* functional deps analysis */
    std::vector <uint32_t> orders_init;
        for(int32_t i = num_vars-1 ; i>=0 ; i--)
            orders_init.emplace_back(i);
    auto const deps = exact_fd_analysis( tt, stats );
    prepare_quantum_state( tt, deps, stats , orders_init);

    /* increment truth table */
    kitty::next_inplace( tt );
  } while ( !kitty::is_const0( tt ) );

  // std::ofstream report_file;
  // report_file.open("../output.txt");
  // write_report( stats, report_file );
  // report_file.close();

  std::cout << '\n';
  write_report( stats, std::cout );
}


int main()
{
  example1();
  return 0;
}
