#pragma once

#include <kitty/dynamic_truth_table.hpp>
#include <map>
#include <vector>

namespace tweedledum
{

using dependencies_t = std::map<uint32_t , std::vector<std::pair<std::string, std::vector<uint32_t>>>>;
using order_t = std::vector<uint32_t>;

struct qsp_dependency_analysis_stats
{
  uint32_t num_analysis_calls{0};
  uint32_t has_no_dependencies{0};
  uint32_t no_dependencies_computed{0};
  uint32_t has_dependencies{0};
  
  double total_time{0};
};

inline order_t create_order( uint32_t num_variables )
{
  order_t order( num_variables );
  for ( auto i = 0u; i < num_variables; ++i )
  {
    order[i] = num_variables - 1 - i;
  }
  return order;
}

class qsp_default_dependency_analysis_impl
{
public:
  explicit qsp_default_dependency_analysis_impl( kitty::dynamic_truth_table const& tt, order_t const& order, qsp_dependency_analysis_stats& stats )
    : tt( tt )
    , order( order )
    , stats( stats )
  {
  }

  dependencies_t operator()()
  {
    return dependencies_t{};
  }
  
protected:
  kitty::dynamic_truth_table const& tt;
  order_t const& order;
  qsp_dependency_analysis_stats& stats;
}; /* qsp_default_dependency_analysis */

class qsp_dependency_analysis_impl
{
public:
  explicit qsp_dependency_analysis_impl( kitty::dynamic_truth_table const& tt, order_t const& order, qsp_dependency_analysis_stats& stats )
    : tt( tt )
    , order( order )
    , stats( stats )
  {
  }

  dependencies_t operator()()
  {
    return dependencies_t{};
  }
  
protected:
  kitty::dynamic_truth_table const& tt;
  order_t const& order;
  qsp_dependency_analysis_stats& stats;
}; /* qsp_dependency_analysis */

template<class DependencyAnalysisEngine = qsp_default_dependency_analysis_impl>
dependencies_t qsp_dependency_analysis( kitty::dynamic_truth_table const& tt, order_t const& order, qsp_dependency_analysis_stats& stats )
{
  DependencyAnalysisEngine dep_analysis_engine( tt, order, stats );
  return dep_analysis_engine();
}

} /* namespace tweedledum */
