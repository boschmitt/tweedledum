#include <kitty/kitty.hpp>
#include <fstream>
#include <filesystem>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <tweedledum/algorithms/synthesis/qsp_tt_dependencies.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/algorithms/generic/rewrite.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/qasm.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>

auto benches_no_dep = 0;
auto all_bench2 = 0;
auto benches_I_cant_find_dep=0;

uint32_t andxor_num = 0;

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

  uint64_t count_ones(  ) const
  {
    return kitty::count_ones(_bits);
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
bool check_not_exist_dependencies(std::vector<partial_truth_table> minterms, uint32_t target)
{
    uint32_t const minterm_length = minterms[0u].num_bits();
    uint32_t const num_minterms = minterms.size();

    //std::cout<<"target: "<<target<<std::endl;

    for(auto i=0u ; i<minterms.size() ; i++)
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
        
        if(check.count_ones()==1)
            return true;  
    }
    
    return false;
}


int main()
{
  /* read minterms for dependency analysis from a file */
  //auto const minterms = read_minterms_from_file( "minterms.txt" );
  std::ofstream out_file;
  out_file.open("../input_allfunc_with4var.txt"); 
  std::string inpath = "../input_allfunc_with4var/";
  
  DIR * dir;
  struct dirent *entry;
  if( dir=opendir(inpath.c_str()) )
  {
    while(entry = readdir(dir))
    {
      if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".DS_Store") != 0 )
      {
        
        std::string tt_str;
        std::ifstream infile;
        std::string infile_name = entry->d_name;
        infile.open(inpath+infile_name);
        infile>>tt_str;
        //std::cout<<tt_str<<std::endl;
        //std::cout<<tt_str.size()<<std::endl;
        
        out_file<<infile_name<<std::endl;
        
        std::reverse(tt_str.begin(), tt_str.end());
        auto tt_vars = int(log2(tt_str.size()));
        
        kitty::dynamic_truth_table tt(tt_vars);
        kitty::create_from_binary_string(tt, tt_str);
        

        // bool skip = false;
        // for ( auto i = 0; i < tt.num_vars(); ++i )
        // {
        //     if ( kitty::count_ones( kitty::cofactor0( tt, i ) ) == 0 ||  kitty::count_ones( kitty::cofactor1( tt, i ) ) == 0)
        //     {
        //       skip = true;
        //       break;
        //     }
        // }

        // if ( skip )
        // {
        //   std::cout << "skip a function because independent variable detected" << std::endl;
        //   continue;
        // }



        std::vector<partial_truth_table> minterms = on_set(tt);

        

        // if (infile_name[0] != '8')
        //     continue;
        // if (minterms.size() < tt_vars)
        //     continue;

        std::cout << entry->d_name << "\n";
        std::cout<<"tt vars: "<<tt_vars<<std::endl;
        std::cout << "[i] #minterms = " << minterms.size() << std::endl;

        // for ( const auto& m : minterms )
        // {
        //   print_binary( m );
        //   std::cout << std::endl;
        // }

        /* convert minterms to column vectors */
        uint32_t const minterm_length = minterms[0u].num_bits();
        //std::cout<<"minterm len: "<<minterm_length<<std::endl;
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

        // kitty::dynamic_truth_table tt0(log2(num_minterms));
        // std::string str0="";
        // for(auto i=0;i<num_minterms;i++)
        //   str0 += "0";

        // kitty::create_from_binary_string(tt0, str0);

        // bool sig=0;
        // for ( const auto& c : columns )
        // {
        //   //print_binary( c );
        //   //std::cout << std::endl;
        //   if (c._bits == tt0){
        //     sig = 1;
        //     break;
        //   }
        // }
        // if(sig)
        //   continue;
        
        /* resubstitution-style dependency analysis */
        std::map<uint32_t , std::pair<std::string, std::vector<uint32_t>>> dependencies;
        //std::cout<<"columns size: "<<columns.size()<<std::endl;
        auto not_exist_dep_sig = 0;
        for ( auto i = int32_t( columns.size() )-1; i >= 0; --i )
        {
            //std::cout<<"i: "<<i<<std::endl;
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
            if(i<minterm_length-2)
            {
                bool not_exist_dep = check_not_exist_dependencies(minterms,i);
                if(not_exist_dep)
                    not_exist_dep_sig++;
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
                if ( ( columns.at( i ) == ( (columns.at( j ) & columns.at( k )) ^ columns.at(l)) ) |
                ( columns.at( i ) == ( (columns.at( l ) & columns.at( k )) ^ columns.at(j)) ) |
                ( columns.at( i ) == ( (columns.at( j ) & columns.at( l )) ^ columns.at(k)) ) )
                {
                    std::cout<<"yes and xor\n";
                    andxor_num ++;
                    found = true;
                    dependencies[i] = std::pair{ std::string{"and_xor"}, std::vector<uint32_t>{ j , k  , l}  };
                    break;
                    
                }
                
              }
            }
            if(found)
            break;
          }


          // if ( found )
          //   continue;
        }

        //std::cout<<"debug: after dependency\n";
        
        /* print dependencies */
        //std::map<uint32_t , std::pair<std::string, std::vector<uint32_t>>> dependencies;
        std::cout << "[i] dependencies:" << std::endl;
        std::cout<<"dependencies size: "<<dependencies.size()<<std::endl;
        for ( const auto& d : dependencies )
        {
          std::cout<<d.first<<"  ";
          std::cout << d.second.first << ' ';
          out_file<<d.first<<"  ";
          out_file << d.second.first << ' ';
          for ( const auto& c : d.second.second )
          {
            std::cout << c << ' ';
            out_file << c << ' ';
          }
          std::cout << std::endl;
          out_file << std::endl;
        }  

        using namespace tweedledum;
        netlist<mcst_gate> net;

        if( (not_exist_dep_sig == (minterm_length-2)) && (dependencies.size()==0) )
        {
            benches_no_dep ++;
        }
        else if((not_exist_dep_sig != (minterm_length-2)) && (dependencies.size()==0))
        {
            benches_I_cant_find_dep ++;
        }
            
        all_bench2++;
        
        qsp_tt_dependencies(net, tt_str , dependencies , out_file);
        infile.close();
      }
    }
    closedir(dir);
  }

  std::cout<<"benches no dep: "<<benches_no_dep<<std::endl;
  std::cout<<"benches I cant find dep: "<<benches_I_cant_find_dep<<std::endl;
  std::cout<<"all bench2: "<<all_bench2<<std::endl;
  
  return 0;
}
