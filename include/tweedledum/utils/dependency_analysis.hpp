#pragma once

#include <kitty/kitty.hpp>
#include <percy/percy.hpp>

namespace tweedledum
{
using dependencies_t = std::map<uint32_t , std::vector<std::pair<std::string, std::vector<uint32_t>>>>;

struct functional_dependency_stats
{
  uint32_t num_analysis_calls{0};
  uint32_t has_no_dependencies{0};
  uint32_t no_dependencies_computed{0};
  uint32_t has_dependencies{0};
  uint32_t funcdep_bench_useful{0};
  uint32_t funcdep_bench_notuseful{0};
  uint32_t total_cnots{0};
  uint32_t total_rys{0};
  double total_time{0};
};

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


}// end namespace tweedledum
