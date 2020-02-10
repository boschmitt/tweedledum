#include <tweedledum/algorithms/synthesis/qsp_tt_dependencies.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/algorithms/generic/rewrite.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/qasm.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/partial_truth_table.hpp>
#include <tweedledum/utils/dependency_analysis.hpp>
#include <kitty/kitty.hpp>
#include <percy/percy.hpp>
#include <fmt/format.h>
#include <fstream>
#include <dirent.h>
// #include <stdlib.h>
// #include <sys/types.h>

using namespace tweedledum;

void prepare_quantum_state( kitty::dynamic_truth_table const& tt, dependencies_t const& dependencies, 
functional_dependency_stats& stats, std::vector<uint32_t> orders )
{
  tweedledum::netlist<tweedledum::mcst_gate> ntk;
  qsp_tt_deps_statistics qsp_stats;
  std::reverse(orders.begin(),orders.end());
  
  qsp_tt_dependencies( ntk, tt, dependencies, orders, qsp_stats);

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

    tt_str = "1001101010101010100110101010101010011010101010100000000000000000";

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
    print_dependencies(deps1);
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
  auto counter=0;
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
          if(counter==50)
            break;
          counter++;
        /* functional deps analysis */
        std::vector <uint32_t> orders_init;
        for(int32_t i = tt_vars-1 ; i>=0 ; i--)
            orders_init.emplace_back(i);

        /* without ordering */   
        auto const deps2 = functional_dependency_analysis( tt, stats2 , orders_init);
        std::cout<<"tt_str:"<<tt_str<<std::endl;
        kitty::print_binary(tt);
        std::cout<<std::endl;
        print_dependencies(deps2);
        std::cout<<"next iteration\n";
        prepare_quantum_state( tt, deps2, stats2 , orders_init );

        /* with ordering */
        //auto const deps1 = functional_dependency_analysis( tt, stats , orders_init);
        //print_dependencies(deps1);
        //auto orders = varaible_ordering_regarding_deps(deps1 , tt_vars);
        // for(auto i=0; i<orders.size() ; i++)
        //     std::cout<<orders[i]<<"  ";
        // std::cout<<std::endl;
        
        //auto const deps2 = functional_dependency_analysis( tt, stats2 , orders);
        //std::cout<<"deps2:\n";
        //print_dependencies(deps2);
        //std::cout<<"end iteration\n";
        //prepare_quantum_state( tt, deps2, stats2 , orders );
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
