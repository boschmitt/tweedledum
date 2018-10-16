/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "esop_post_optimization.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stack>
#include <unordered_map>
#include <vector>

#include <boost/graph/connected_components.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/pending/integer_log2.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/utils/costs.hpp>
#include <reversible/variable.hpp>

namespace cirkit
{

using edge_properties = boost::property<boost::edge_weight_t, unsigned>;
using mygraph = graph_t<boost::no_property, edge_properties>;

struct SimpleHash
{
  size_t operator()( const std::pair<unsigned, unsigned>& p ) const
  {
    return p.first ^ p.second;
  }
};

struct feature
{
  uint16_t control = 0u;
  uint16_t polarity = 0u;

  feature( gate& g );
  feature();
};

struct pair
{
  gate f;
  gate s;
  feature a;
  feature b;
  circuit const* c = nullptr;
  circuit equivalent;

  pair( const gate& f, const gate& s, const circuit& c, feature& a, feature& b );
  pair();

  bool computequivalence( unsigned& c1, unsigned& c2, double* t1, double* t2 );

  unsigned long long get_cost();

private:
  std::pair<gate, unsigned> single_control{f, 0u};
};

class optimization_graph
{
public:
  /* you can give the ESOP circuit to be optimized or directly the graph */
  optimization_graph( circuit& new_c );
  optimization_graph( const mygraph& opt_g_given );

  std::vector<edge_t<graph_t<>>> greedy_matching( bool describe );
  std::vector<edge_t<graph_t<>>> exact_matching( bool describe );
  std::vector<edge_t<graph_t<>>> realexact_matching( bool describe );

  void order_edges();

  /* this function automatically optimize for the greeding matching */
  circuit optimize_esop();

private:
  circuit c;
  mygraph opt_g;

  std::unordered_map<std::pair<unsigned, unsigned>, pair, SimpleHash> edge_to_pair;

  std::unordered_map<unsigned, std::vector<edge_t<graph_t<>>>> pos_to_match;

  std::vector<edge_t<graph_t<>>> ordered_edges;
  std::vector<edge_t<graph_t<>>> matching;
  unsigned match_w = 0;

  std::vector<vertex_t<graph_t<>>> v_saturated;
};

bool is_control( gate g, variable control )
{
  auto it = std::find( g.controls().begin(), g.controls().end(), control );
  return it != g.controls().end();
}

pair::pair( const gate& f_given, const gate& s_given, const circuit& c_given,
            feature& a_given, feature& b_given )
    : f( f_given ), s( s_given ), c( &c_given ), a( a_given ), b( b_given )
{
}

pair::pair() {}

int count( const uint16_t bits ) { return __builtin_popcount( bits ); }

int get_bit( uint16_t number, unsigned index )
{
  if ( ( ( number >> index ) & 1 ) == 1 )
    return 1;
  else
    return 0;
}

bool pair::computequivalence( unsigned& c1, unsigned& c2, double* t1,
                              double* t2 )
{
  bool combine = false;

  /* FIRST PROPERTY */
  if ( ( ( ( a.control & b.control ) & ( a.polarity ^ b.polarity ) ) == 0 ) &&
       ( ( a.control & b.control ) !=
         0 ) )
  {
    increment_timer t( t1 );
    c1++;
    auto a_notb = a.control & ( ~b.control );
    auto b_nota = ( ~a.control ) & b.control;

    if ( count( a_notb ) == 1 )
    {

      unsigned target = log2( a_notb );

      std::vector<variable> controls;
      uint16_t to_create = b_nota;
      for ( unsigned i = 0; i < 16; i++ )
      {
        if ( ( to_create & 1 ) == 1 )
        {
          if ( ( ( b.polarity & ( 1 << i ) ) >> i ) == 1 )
          {
            controls.push_back( make_var( i, true ) );
          }
          else
          {
            controls.push_back( make_var( i, false ) );
          }
        }

        to_create = to_create >> 1;
      }
      equivalent.set_lines( c->lines() );
      append_toffoli( equivalent, controls, target );
      append_toffoli( equivalent, f.controls(), f.targets().front() );
      append_toffoli( equivalent, controls, target );
      combine = true;
    }
    else if ( count( b_nota ) == 1 )
    {
      unsigned target = boost::integer_log2( b_nota );

      std::vector<variable> controls;
      uint16_t to_create = a_notb;
      for ( unsigned i = 0; i < 16; i++ )
      {
        if ( ( to_create & 1 ) == 1 )
        {
          if ( ( ( a.polarity >> i ) & 1 ) == 1 )
          {
            controls.push_back( make_var( i, true ) );
          }
          else
            controls.push_back( make_var( i, false ) );
        }
        to_create = to_create >> 1;
      }

      equivalent.set_lines( c->lines() );
      append_toffoli( equivalent, controls, target );
      append_toffoli( equivalent, s.controls(), s.targets().front() );
      append_toffoli( equivalent, controls, target );

      combine = true;
    }
  }
  /* SECOND PROPERTY */
  else if ( a.control == b.control )
  {
    increment_timer t( t2 );
    c2++;
    auto diff_pol = a.polarity ^ b.polarity;

    if ( count( diff_pol ) >= 1 )
    {
      unsigned i = 0;
      bool done = false;
      std::vector<unsigned> cnot_targets;
      unsigned cnot_control;
      std::vector<variable> common_controls;

      for ( i = 0; i < 16; ++i )
      {
        if ( ( diff_pol >> i ) & 1 )
        {
          if ( !done )
          {
            cnot_control = i;
            done = true;
            if ( ( ( a.polarity >> i ) & 1 ) == 1 )
            {
              for ( auto con : s.controls() )
              {
                if ( con.line() != i )
                {
                  common_controls.push_back( con );
                }
              }
            }
            else if ( ( ( b.polarity >> i ) & 1 ) == 1 )
            {
              for ( auto con : f.controls() )
              {
                if ( con.line() != i )
                {
                  common_controls.push_back( con );
                }
              }
            }
          }
          else
            cnot_targets.push_back( i );
        }
      }

      equivalent.set_lines( c->lines() );
      for ( auto target : cnot_targets )
      {
        append_cnot( equivalent, make_var( cnot_control, true ), target );
      }

      append_toffoli( equivalent, common_controls, s.targets().front() );

      for ( auto target : cnot_targets )
      {
        append_cnot( equivalent, make_var( cnot_control, true ), target );
      }
    }
    combine = true;
  }

  return combine;
}

unsigned long long pair::get_cost()
{
  circuit pair( c->lines() );

  append_toffoli( pair, f.controls(), f.targets().front() );
  append_toffoli( pair, s.controls(), s.targets().front() );

  return costs( pair, costs_by_gate_func( t_costs() ) );
}

bool operator==( const gate& g1, const gate& g2 )
{
  return ( g1.controls() == g2.controls() ) &&
         ( g1.targets() == g2.targets() ) && same_type( g1, g2 );
}

optimization_graph::optimization_graph( const mygraph& opt_g_given )
    : opt_g( opt_g_given )
{
}

optimization_graph::optimization_graph( circuit& new_c )
{
  double tempo = 0.0, tempo2 = 0.0, tempo3 = 0.0;
  double time_case1 = 0.0, time_case2 = 0.0;
  c = new_c;
  const auto edge_weights = boost::get( boost::edge_weight, opt_g );
  add_vertices( opt_g, new_c.num_gates() );
  std::vector<feature> stack_f;

  {
    reference_timer costa( &tempo3 );
    for ( unsigned i = 0; i < new_c.num_gates(); i++ )
    {
      feature f( new_c[i] );
      stack_f.push_back( f );
    }
  }

  auto c1 = 0u, c2 = 0u, ctr = 0u;
  /* set the loop over all the pairs of the circuit */
  {
    reference_timer costa( &tempo );
    for ( unsigned first = 0; first < new_c.num_gates() - 1; ++first )
    {
      for ( unsigned second = first + 1; second < new_c.num_gates(); ++second )
      {
        pair p( new_c[first], new_c[second], new_c, stack_f[first],
                stack_f[second] );

        bool result = false;
        ++ctr;
        result = p.computequivalence( c1, c2, &time_case1, &time_case2 );

        if ( result )
        {
          increment_timer costa2( &tempo2 );

          circuit& eq = p.equivalent;

          auto old_cost = p.get_cost();

          const auto cost = costs( eq, costs_by_gate_func( t_costs() ) );
          if ( cost < old_cost )
          {
            unsigned long long edge_gain =
                old_cost - cost; // it should be always positive

            auto e = add_edge( first, second, opt_g );
            auto edge = e.first;
            edge_weights[edge] = edge_gain;

            // map the fist source vertex with the corresponding pair
            edge_to_pair.insert( {std::make_pair( first, second ), p} );

          }
        }
      }
    }
  }
}

void optimization_graph::order_edges()
{
  auto edge_weights = boost::get( boost::edge_weight, opt_g );


  for ( auto edge : boost::make_iterator_range( boost::edges( opt_g ) ) )
  {
    ordered_edges.push_back( edge );
  }


  std::sort(
      ordered_edges.begin(), ordered_edges.end(),
      [edge_weights]( const edge_t<graph_t<>>& i, const edge_t<graph_t<>>& j ) {
        return ( edge_weights[i] > edge_weights[j] );
      } );
}

std::vector<edge_t<graph_t<>>>
optimization_graph::realexact_matching( bool describe )
{
  const auto edge_weights = boost::get( boost::edge_weight, opt_g );
  matching = {};

  std::vector<unsigned> components( boost::num_vertices( opt_g ) );
  const auto num_components =
      boost::connected_components( opt_g, &components[0] );
  if ( describe )
  {
    std::cout << "graph has" << num_components
              << "components: " << any_join( components, " " ) << std::endl;
  }

  std::vector<std::vector<unsigned>> map_connected( num_components );
  for ( unsigned i = 0; i < boost::num_vertices( opt_g ); i++ )
  {
    unsigned group = components[i];
    map_connected[group].push_back( i );
  }

  if ( describe )
  {
    unsigned conta = 0;
    for ( auto u : map_connected )
    {
      if ( describe )
      {
        std::cout << "Group " << conta << ":" << std::endl;
        for ( auto k : u )
        {
          std::cout << k << " ";
        }
        std::cout << std::endl;
      }

      conta++;
    }
  }

  // for every group >1
  for ( auto group : map_connected )
  {
    auto g_size = group.size();
    if ( g_size > 1 )
    {

      std::vector<boost::dynamic_bitset<>> match_matrix;

      std::vector<std::vector<edge_t<graph_t<>>>> matrix(
          g_size, std::vector<edge_t<graph_t<>>>( g_size ) );
      std::vector<std::vector<unsigned>> matrix_w(
          g_size, std::vector<unsigned>( g_size ) );
      std::vector<std::pair<unsigned, unsigned>>
          pair_edge; // contains all the edges in the group



      for ( auto edge : boost::make_iterator_range( boost::edges( opt_g ) ) )
      {
        auto v_source = boost::source( edge, opt_g );
        auto s_iter = std::find( group.begin(), group.end(), v_source );
        if ( s_iter != group.end() )
        {

          auto s_index = s_iter - group.begin();
          auto v_target = boost::target( edge, opt_g );
          auto t_index =
              std::find( group.begin(), group.end(), v_target ) - group.begin();
          matrix[s_index][t_index] = matrix[t_index][s_index] = edge;
          matrix_w[s_index][t_index] = matrix_w[t_index][s_index] =
              edge_weights[edge];

          pair_edge.push_back( std::make_pair( s_index, t_index ) );
        }
      }

      if ( describe )
      {
        std::cout << "matrix " << std::endl;
        for ( auto i : matrix )
        {
          for ( auto k : i )
          {
            std::cout << k << " ";
          }
          std::cout << std::endl;
        }
        std::cout << "matrix W" << std::endl;
        for ( auto i : matrix_w )
        {
          for ( auto k : i )
          {
            std::cout << k << " ";
          }
          std::cout << std::endl;
        }
        std::cout << "edges" << std::endl;
        for ( auto i : pair_edge )
        {
          std::cout << i.first << "-" << i.second << std::endl;
        }
      }

      boost::dynamic_bitset<> b( pair_edge.size() );

      unsigned wmax_combinations = 0;

      do
      {
        if ( describe )
        {
          std::cout << "combination: " << b << std::endl;
        }
        unsigned w = 0;

        std::vector<boost::dynamic_bitset<>> M(
            g_size, boost::dynamic_bitset<>( g_size ) );

        for ( unsigned bit = 0; bit < pair_edge.size(); bit++ )
        {

          if ( describe )
          {
            std::cout << "edge num: " << bit << std::endl;
          }

          unsigned source, target;
          source = pair_edge[bit].first;
          target = pair_edge[bit].second;
          if ( describe )
          {
            std::cout << source << "-->" << target << std::endl;
          }
          M[source][target] = M[target][source] = b[bit];
          if ( describe )
          {
            for ( auto row : M )
            {
              std::cout << row << std::endl;
            }
          }

          if ( b[bit] )
          {
            w += matrix_w[source][target];
          } // increment the match weigth
        }

        if ( describe )
          std::cout << "is the matrix for this combination a match?? "
                    << std::endl;
        bool match = true;
        for ( auto row : M )
        {
          if ( describe )
            std::cout << row << std::endl;
          if ( row.count() > 1 )
          {
            match = false;
          }
        }
        if ( describe )
          std::cout << match << std::endl;

        if ( match )
        {
          if ( w > wmax_combinations )
          {
            wmax_combinations = w;

            match_matrix = M;
          }
        }

        inc( b );

      } while ( b.any() );

      if ( describe )
        std::cout << "max comb weight: " << wmax_combinations << std::endl;
      if ( describe )
        std::cout << "matching matrix: ";


      unsigned count = 0u;
      for ( auto i : match_matrix )
      {
        if ( describe )
          std::cout << i << std::endl;
        for ( unsigned k = 0; k < i.size(); k++ )
        {
          if ( i[k] == 1 )
          {
            matching.push_back( matrix[count][k] );
            match_w += matrix_w[count][k];
            match_matrix[k][count] = 0;
          }
        }
        count++;
      }
    }
  }

  if ( describe )
  {
    std::cout << "the found match is: " << std::endl;
    for ( auto match_edge : matching )
    {
      std::cout << edge_weights[match_edge] << " ";
    }
    std::cout << std::endl;
    std::cout << "its weight is: " << match_w << std::endl;
  }
  return matching;
}

std::vector<edge_t<graph_t<>>>
optimization_graph::exact_matching( bool describe )
{
  const auto edge_weights = boost::get( boost::edge_weight, opt_g );

  ordered_edges = {};

  order_edges();
  if ( describe )
  {
    std::cout << "ordered edges: ";
    for ( auto edge : ordered_edges )
    {
      std::cout << edge_weights[edge] << " " << boost::source( edge, opt_g )
                << " - " << boost::target( edge, opt_g ) << ", ";
    }
    std::cout << std::endl;
  }

  unsigned max_match = 0;
  unsigned pos_match = 0;
  for ( unsigned i = 0; i < ordered_edges.size(); i++ )
  {
    matching = {};
    v_saturated = {};
    auto value_match = 0u;
    auto edge_selected = ordered_edges[i];

    if ( describe )
      std::cout << "selected is " << edge_weights[edge_selected];

    auto v_source = boost::source( edge_selected, opt_g );
    auto v_target = boost::target( edge_selected, opt_g );

    if ( describe )
      std::cout << "the vertex are " << v_source << " and " << v_target
                << std::endl;

    matching.push_back( edge_selected );
    value_match += edge_weights[edge_selected];
    // v_sat.set( v_source );
    v_saturated.push_back( v_source );
    v_saturated.push_back( v_target );

    for ( auto edge : ordered_edges )
    {
      if ( edge != edge_selected )
      {
        if ( describe )
          std::cout << "edge is " << edge_weights[edge];
        auto v_source = boost::source( edge, opt_g );
        auto v_target = boost::target( edge, opt_g );

        if ( describe )
          std::cout << "The sources are: " << v_source << "and" << v_target
                    << std::endl;

        if ( std::find( v_saturated.begin(), v_saturated.end(), v_source ) ==
                 v_saturated.end() &&
             std::find( v_saturated.begin(), v_saturated.end(), v_target ) ==
                 v_saturated.end() )
        {

          if ( describe )
            std::cout << "can be in the matching" << std::endl;

          matching.push_back( edge );
          value_match += edge_weights[edge];
          v_saturated.push_back( v_source );
          v_saturated.push_back( v_target );
        }
        else
        {
          if ( describe )
            std::cout << "cannot be in the matching" << std::endl;
        }
      }
    }

    pos_to_match[i] = matching;

    if ( value_match > max_match )
    {
      max_match = value_match;
      pos_match = i;
    }
  }


  matching = pos_to_match[pos_match];
  if ( describe )
  {
    std::cout << "the found match is: " << std::endl;
    for ( auto match_edge : matching )
    {
      std::cout << edge_weights[match_edge] << " ";
    }
    std::cout << std::endl;
  }
  return matching;
}

std::vector<edge_t<graph_t<>>>
optimization_graph::greedy_matching( bool describe )
{

  const auto edge_weights = boost::get( boost::edge_weight, opt_g );

  order_edges();


  for ( auto edge : ordered_edges )
  {

    auto v_source = boost::source( edge, opt_g );
    auto v_target = boost::target( edge, opt_g );

    if ( describe )
      std::cout << "The sources are: " << v_source << "and" << v_target
                << std::endl;

    if ( std::find( v_saturated.begin(), v_saturated.end(), v_source ) ==
             v_saturated.end() &&
         std::find( v_saturated.begin(), v_saturated.end(), v_target ) ==
             v_saturated.end() )
    {
      if ( describe )
        std::cout << "can be in the matching" << std::endl;
      matching.push_back( edge );

      v_saturated.push_back( v_source );
      v_saturated.push_back( v_target );
    }
    else
    {
      if ( describe )
        std::cout << "cant be in the matching" << std::endl;
    }
  }
  if ( describe )
  {
    std::cout << "the found match is: " << std::endl;
    for ( auto match_edge : matching )
    {
      std::cout << edge_weights[match_edge] << " ";
    }
    std::cout << std::endl;
  }

  return matching;
}

circuit optimization_graph::optimize_esop()
{
  std::vector<unsigned> untouched_pos;

  for ( unsigned pos = 0u; pos < c.num_gates(); ++pos )
  {
    untouched_pos.push_back( pos );
  }

  circuit c_optimized( c.lines() );
  for ( auto edge : matching )
  {

    auto v_source = boost::source( edge, opt_g );
    auto v_target = boost::target( edge, opt_g );
    std::pair<unsigned, unsigned> key = std::make_pair( v_source, v_target );
    pair& p_comb = edge_to_pair[key];

    for ( const auto& g : p_comb.equivalent )
    {
      c_optimized.append_gate() = g;
    }
    untouched_pos.erase(
        std::remove( untouched_pos.begin(), untouched_pos.end(), v_source ),
        untouched_pos.end() );
    untouched_pos.erase(
        std::remove( untouched_pos.begin(), untouched_pos.end(), v_target ),
        untouched_pos.end() );
  }

  for ( unsigned pos : untouched_pos )
  {
    append_toffoli( c_optimized, c[pos].controls(), c[pos].targets().front() );
  }


  return c_optimized;
}
feature::feature() {}

feature::feature( gate& g )
{

  for ( auto c : g.controls() )
  {
    control |= ( 1 << c.line() );
    if ( c.polarity() == 1 )
    {
      polarity |= ( 1 << c.line() );
    }
  }
}

circuit esop_post_optimization( const circuit& c, const properties::ptr& settings,
                                const properties::ptr& statistics )
{
  const auto verbose = get( settings, "verbose", false );
  const auto greedy = get( settings, "greedy", true );
  const auto exact = get( settings, "exact", false );

  if ( greedy && exact )
  {
    std::cout << "[w] both optimization approaches are enabled, will perform greedy matching" << std::endl;
  }

  properties_timer t( statistics );
  double* tempo;

  circuit new_c = c;

  optimization_graph g( new_c );
  std::vector<edge_t<graph_t<>>> match;

  if ( greedy )
  {
    match = g.greedy_matching( false );
  }
  else if ( exact )
  {
    match = g.realexact_matching( false );
  }

  if ( verbose )
  {
    std::cout << c << std::endl;
  }

  new_c = g.optimize_esop();

  if ( verbose )
  {
    std::cout << new_c << std::endl;
  }

  unsigned global_pre = costs( c, costs_by_gate_func( t_costs() ) );
  unsigned global_post = costs( new_c, costs_by_gate_func( t_costs() ) );

  unsigned global_cost_gain = global_pre - global_post;

  double percentage = global_cost_gain;
  percentage /= global_pre;

  double match_percentage = match.size();
  match_percentage /= c.num_gates();

  set( statistics, "% matched pairs", match_percentage );
  set( statistics, "global improvement", percentage );

  return new_c;
}
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
