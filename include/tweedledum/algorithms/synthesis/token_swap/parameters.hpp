/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

namespace tweedledum {

/*! \brief Parameters for `swap_network`. */
struct swap_network_params {
	/*! \brief Variants of swap network synthesis. */
	enum class methods {
		/*! \brief Use A* search with admissible heuristic. (**default**) */
		admissible,
		/*! \brief A* search with non admissible heuristic. */
		non_admissible,
		/*! \brief Use SAT solver. */
		sat,
	};

	/*! \brief Optimization goal while synthesizing. */
	enum class opt_goals {
		/*! \brief Number of swap gates. (**default**) */
		num_swaps,
		/*! \brief Number of levels. */
		num_levels,
	};

	/*! \brief Synthesis method. */
	methods method = methods::admissible;

	/*! \brief Synthesis optimization goal. */
	opt_goals opt_goal = opt_goals::num_swaps;

	/*! \brief Be verbose. */
	bool verbose = false;
};

} // namespace tweedledum
