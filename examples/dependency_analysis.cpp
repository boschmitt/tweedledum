#include <tweedledum/algorithms/synthesis/qsp_tt_dependencies.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/algorithms/generic/rewrite.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/qasm.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>

#include <kitty/kitty.hpp>
#include <fmt/format.h>
#include <fstream>

// #include <dirent.h>
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






using dependencies_t = std::map<uint32_t , std::pair<std::string, std::vector<uint32_t>>>;

struct functional_dependency_stats
{
  uint32_t num_analysis_calls{0};
  uint32_t has_no_dependencies{0};
  uint32_t no_dependencies_computed{0};

  uint32_t total_reduction{0};
  double total_time{0};

  uint32_t total_cnots{0};
  uint32_t total_rys{0};
};

dependencies_t functional_dependency_analysis( kitty::dynamic_truth_table const& tt, functional_dependency_stats& stats )
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

  /* resubstitution-style dependency analysis */
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
        dependencies[i] = std::pair{ std::string{"eq"}, std::vector<uint32_t>{ j } };
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
        dependencies[i] = std::pair{ std::string{"not"}, std::vector<uint32_t>{ j }  };
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
        if ( columns.at( i ) == ( columns.at( j ) ^ columns.at( k ) ) )
        {
          found = true;
          dependencies[i] = std::pair{ std::string{"xor"}, std::vector<uint32_t>{ j , k }  };
          break;
        }
        else if ( columns.at( i ) == ( ~ (columns.at( j ) ^ columns.at( k ) ) ) )
        {
          found = true;
          dependencies[i] = std::pair{ std::string{"xnor"}, std::vector<uint32_t>{ j , k  }  };
          break;
        }

        //-----3rd input
        for(auto l = j-2 ; l>i ; --l)
        {
          if ( columns.at( i ) == ( columns.at( j ) ^ columns.at( k ) ^ columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::pair{ std::string{"xor"}, std::vector<uint32_t>{ j , k  , l}  };
            break;
          }
          else if ( columns.at( i ) == ( ~(columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ) ) )
          {
            found = true;
            dependencies[i] = std::pair{ std::string{"xnor"}, std::vector<uint32_t>{ j , k , l }  };
            break;
          }
          //---- 4th input
          for(auto m = j-3 ; m>i ; --m)
          {
            if ( columns.at( i ) == ( columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ^ columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::pair{ std::string{"xor"}, std::vector<uint32_t>{ j , k  , l , m}  };
              break;
            }
            else if ( columns.at( i ) == ( ~(columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ^ columns.at(m) ) ) )
            {
              found = true;
              dependencies[i] = std::pair{ std::string{"xnor"}, std::vector<uint32_t>{ j , k , l , m }  };
              break;
            }

            //---- 5th input
            for(auto i5 = j-4 ; i5>i ; --i5)
            {
              if ( columns.at( i ) == ( columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ^ columns.at(m) ^ columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::pair{ std::string{"xor"}, std::vector<uint32_t>{ j , k  , l , m , i5}  };
                break;
              }
              else if ( columns.at( i ) == ( ~(columns.at( j ) ^ columns.at( k ) ^ columns.at(l) ^ columns.at(m) ^ columns.at(i5) ) ) )
              {
                found = true;
                dependencies[i] = std::pair{ std::string{"xnor"}, std::vector<uint32_t>{ j , k , l , m , i5}  };
                break;
              }
            }  
          }
        }
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
          dependencies[i] = std::pair{ std::string{"and"}, std::vector<uint32_t>{ j , k }  };
          break;
        }
        else if ( columns.at( i ) == ( ~ (columns.at( j ) & columns.at( k )) ) )
        {
          found = true;
          dependencies[i] = std::pair{ std::string{"nand"}, std::vector<uint32_t>{ j , k  }  };
          break;
        }

        //----3rd input
        for(auto l = j-2 ; l>i ; --l)
        {
          if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::pair{ std::string{"and"}, std::vector<uint32_t>{ j , k  , l}  };
            break;
          }
          else if ( columns.at( i ) == ( ~(columns.at( j ) & columns.at( k ) & columns.at(l) ) )  )
          {
            found = true;
            dependencies[i] = std::pair{ std::string{"nand"}, std::vector<uint32_t>{ j , k , l }  };
            break;
          }
          //---- 4th input
          for(auto m = j-3 ; m>i ; --m)
          {
            if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::pair{ std::string{"and"}, std::vector<uint32_t>{ j , k  , l , m}  };
              break;
            }
            else if ( columns.at( i ) == ( ~(columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) ) ) )
            {
              found = true;
              dependencies[i] = std::pair{ std::string{"nand"}, std::vector<uint32_t>{ j , k , l , m }  };
              break;
            }

            //---- 5th input
            for(auto i5 = j-4 ; i5>i ; --i5)
            {
              if ( columns.at( i ) == ( columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) & columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::pair{ std::string{"and"}, std::vector<uint32_t>{ j , k  , l , m , i5}  };
                break;
              }
              else if ( columns.at( i ) == ( ~(columns.at( j ) & columns.at( k ) & columns.at(l) & columns.at(m) & columns.at(i5) ) ) )
              {
                found = true;
                dependencies[i] = std::pair{ std::string{"nand"}, std::vector<uint32_t>{ j , k , l , m , i5}  };
                break;
              }
            }
          }
        }
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
          dependencies[i] = std::pair{ std::string{"or"}, std::vector<uint32_t>{ j , k }  };
          break;
        }
        else if ( columns.at( i ) == ( ~ (columns.at( j ) | columns.at( k )) ) )
        {
          found = true;
          dependencies[i] = std::pair{ std::string{"nor"}, std::vector<uint32_t>{ j , k  }  };
          break;
        }

        //------3rd input
        for(auto l = j-2 ; l>i ; --l)
        {
          if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l)) )
          {
            found = true;
            dependencies[i] = std::pair{ std::string{"or"}, std::vector<uint32_t>{ j , k  , l}  };
            break;
          }
          else if ( columns.at( i ) == ( ~(columns.at( j ) | columns.at( k ) | columns.at(l) ) )  )
          {
            found = true;
            dependencies[i] = std::pair{ std::string{"nor"}, std::vector<uint32_t>{ j , k , l }  };
            break;
          }

          //---- 4th input
          for(auto m = j-3 ; m>i ; --m)
          {
            if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m)) )
            {
              found = true;
              dependencies[i] = std::pair{ std::string{"or"}, std::vector<uint32_t>{ j , k  , l , m}  };
              break;
            }
            else if ( columns.at( i ) == ( ~(columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) ) ) )
            {
              found = true;
              dependencies[i] = std::pair{ std::string{"nor"}, std::vector<uint32_t>{ j , k , l , m }  };
              break;
            }
            
            //---- 5th input
            for(auto i5 = j-4 ; i5>i ; --i5)
            {
              if ( columns.at( i ) == ( columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) | columns.at(i5)) )
              {
                found = true;
                dependencies[i] = std::pair{ std::string{"or"}, std::vector<uint32_t>{ j , k  , l , m , i5}  };
                break;
              }
              else if ( columns.at( i ) == ( ~(columns.at( j ) | columns.at( k ) | columns.at(l) | columns.at(m) | columns.at(i5) ) ) )
              {
                found = true;
                dependencies[i] = std::pair{ std::string{"nor"}, std::vector<uint32_t>{ j , k , l , m , i5}  };
                break;
              }
            }  
          }        
        }
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
          if ( ( columns.at( i ) == ( (columns.at( j ) & columns.at( k ) ) ^ columns.at( l )) ) |
               ( columns.at( i ) == ( (columns.at( l ) & columns.at( k ) ) ^ columns.at( j )) ) |
               ( columns.at( i ) == ( (columns.at( j ) & columns.at( l ) ) ^ columns.at( k )) ) )
          {
            found = true;
            dependencies[i] = std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ j , k  , l}  };
            break;    
          }       
        }
      }
      if(found)
        break;
    }
  }

  if ( has_no_dependencies == num_minterms - 2u )
  {
    ++stats.has_no_dependencies; 
  }
  else if ( dependencies.size() == 0u )
  {
    ++stats.no_dependencies_computed;
  }

  return dependencies;
}

void print_dependencies( dependencies_t const& dependencies, std::ostream& os = std::cout )
{
  os << "[i] dependencies:" << std::endl;
  os << "dependencies size: " << dependencies.size() << std::endl;
  for ( const auto& d : dependencies )
  {
    os << d.first<<"  ";
    os << d.second.first << ' ';
    for ( const auto& c : d.second.second )
    {
      os << c << ' ';
    }
    os << std::endl;
  }
}

void prepare_quantum_state( kitty::dynamic_truth_table const& tt, dependencies_t const& dependencies, functional_dependency_stats& stats )
{
  tweedledum::netlist<tweedledum::mcst_gate> ntk;
  auto const binary_tt = kitty::to_binary( tt );

  qsp_tt_statistics qsp_stats;

  // dependencies_t no_deps;
  // qsp_tt_dependencies( ntk, binary_tt, no_deps, qsp_stats );

  qsp_tt_dependencies( ntk, binary_tt, dependencies, qsp_stats );
  stats.total_time += qsp_stats.time;
  stats.total_reduction += qsp_stats.reduction;
  stats.total_cnots += qsp_stats.total_cnots;
  stats.total_rys += qsp_stats.total_rys;
}

void write_report( functional_dependency_stats const& stats, std::ostream& os )
{
  os << "[i] number of analyzed benchmarks = " << stats.num_analysis_calls << std::endl;
  os << fmt::format( "[i] total = no deps exist + no deps found + found deps ::: {} = {} + {} + {}\n",
                     stats.num_analysis_calls, stats.has_no_dependencies, stats.no_dependencies_computed,
                     ( stats.num_analysis_calls - stats.has_no_dependencies - stats.no_dependencies_computed ) );
  os << fmt::format( "[i] total synthesis time (considering dependencies) = {:8.2f}s\n",
                     stats.total_time );
  os << fmt::format( "[i] total reduction = {}\n",
                     stats.total_reduction );
  os << fmt::format( "[i] synthesis result: CNOTs / RYs = {} / {}\n",
                     stats.total_cnots, stats.total_rys );
}

void example1()
{
  std::string const filename = "../input/specs.txt";
  
  functional_dependency_stats stats;
  std::ifstream ifs;
  ifs.open( filename );

  std::string line;
  while ( !ifs.eof() )
  {
    std::getline( ifs, line );

    if ( line.empty() )
      continue;
    
    /* prepare truth table */
    std::reverse( line.begin(), line.end() );
    std::uint32_t num_vars = ilog2( line.size() );
    
    kitty::dynamic_truth_table tt( num_vars );
    kitty::create_from_binary_string( tt, line );

    /* functional deps analysis */
    auto const deps = functional_dependency_analysis( tt, stats );
    prepare_quantum_state( tt, deps, stats );
  }
  ifs.close();

  std::ofstream report_file;
  report_file.open("../output.txt"); 
  write_report( stats, report_file );
  report_file.close();
}

void example2()
{
  kitty::dynamic_truth_table tt( 4 );

  functional_dependency_stats stats;

  /* TODO: implementation does not work for 0 */
  kitty::next_inplace( tt );

  do
  {
    /* functional deps analysis */
    auto const deps = functional_dependency_analysis( tt, stats );
    prepare_quantum_state( tt, deps, stats );

    /* increment truth table */
    kitty::next_inplace( tt );
  } while ( !kitty::is_const0( tt ) );

  // std::ofstream report_file;
  // report_file.open("../output.txt"); 
  // write_report( stats, report_file );
  // report_file.close();

  write_report( stats, std::cout );
}

void example3()
{
#if 0
  std::string const inpath = "../input/";
  functional_dependency_stats stats;
  DIR * dir;
  struct dirent *entry;
  if ( ( dir = opendir( inpath.c_str() ) ) )
  {
    while ( ( entry = readdir(dir) ) )
    {
      std::string filename( entry->d_name );
      if ( filename == "." || filename == ".." || filename == ".DS_Store" )
        continue;

      /* read file */
      std::string tt_str;

      std::ifstream infile;
      infile.open( inpath + filename );
      infile >> tt_str;
      infile.close();

      /* prepare truth table */
      std::reverse( tt_str.begin(), tt_str.end() );
      auto const tt_vars = int( log2( tt_str.size() ) );
      
      kitty::dynamic_truth_table tt( tt_vars );
      kitty::create_from_binary_string( tt, tt_str );

      /* functional deps analysis */
      auto const deps = functional_dependency_analysis( tt, stats );
      prepare_quantum_state( tt, deps, stats );
    }
    
    closedir( dir );
  }

  std::ofstream report_file;
  report_file.open("../output.txt"); 
  write_report( stats, report_file );
  report_file.close();
#endif  
}

int main()
{
  example2();  
  return 0;
}
