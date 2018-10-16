/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Giulia Meuli
*------------------------------------------------------------------------------------------------*/

#include <vector>
#include <kitty/cube.hpp>

namespace tweedledum
{

    class optimization_graph
    {
        using kcube = kitty::cube;

        struct node
        {
            uint index;
            bool matched;
        };

        struct edge
        {
            node from;
            node to;
            uint weight = 0;
            bool matched;
        };

        std::vector<node> nodes;
        std::vector<edge> edges;

        int check_first_property_first(kcube f, kcube s)
        {
            if ( ( ( f._mask & s._mask ) & ( f._bits ^ s._bits ) ) == 0 ) && ( ( f._mask & s._mask ) != 0 ) )
            {
                auto a_notb = f._mask & ( ~s._mask );
                auto b_nota = ( ~f._mask ) & s._mask;
                var cnot_controls_count = 0;

                if ( count( a_notb ) == 1 )
                {
                    while (b_nota != 0)
                    {
                        if (b_nota & 1) cnot_controls_count ++;
                        b_nota = bnota >> 1;
                    }

                    return cost;
                }

                if ( count( b_nota ) == 1 )
                {
                    while (a_notb != 0)
                    {
                        if (a_notb & 1) cnot_controls_count ++;
                        a_notb = anotb >> 1;
                    }

                    return cost;
                }
                
            }
        }

        int check_second_property(kcube f, kcube s)
        {
            auto f_contr = f._mask;
            auto f_pol = f._bits;
            auto s_contr = s._mask;
            auto s_pol = s._bits;


        }

        optimization_graph(std::vector<kcube> esop)
        {
            nodes.resize(esop.size());

            for_each(auto cube : esop)
            {
                foreach(auto paired : esop)
                {
                    if paired != cube
                    {
                        check_first_property(cube, paired);
                        check_second_property(cube, paired);
                    }
                }
                auto contr = cube._mask;
                auto pol = cube._bits;



            }

            //foreach cube
            //check its pairs
                //not repeat
            // if verifies one condition 
                //compute cost
                //add edge
        }
    };
}