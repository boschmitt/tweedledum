Change Log
==========

v1.0-beta [`Not yet released <https://github.com/boschmitt/tweedledum/tree/master>`_] 
-------------------------------------------------------------------------------------

* Initial network and gate interfaces 
* Gate implementations:
* Network implementations:
    - Gate graph (`gg_network`) `#17 <https://github.com/boschmitt/tweedledum/pull/17>`_
* Algorithms:
    - Transformation based synthesis (`tbs`) `#6 <https://github.com/boschmitt/tweedledum/pull/6>`_
    - ESOP-phase synthesis (`esop_phase_synth`) `#7 <https://github.com/boschmitt/tweedledum/pull/7>`_
    - Single targe gate synthesis (`stg_from_exact_esop`) `#11 <https://github.com/boschmitt/tweedledum/pull/11>`_
* I/O
    - Write to ProjectQ (`write_projectq`) `#13 <https://github.com/boschmitt/tweedledum/pull/13>`_
    - Write to Quirk json format (`write_quirk`) `#12 <https://github.com/boschmitt/tweedledum/pull/12>`_
    - Writo to GraphViz DOT format (`write_dot`) `#17 <https://github.com/boschmitt/tweedledum/pull/17>`_
* Utility data structures:
* Views:
    - Disable structural modifications to network (`immutable_view`) `#17 <https://github.com/boschmitt/tweedledum/pull/17>`_
    - View compute depth and node levels (`depth_view`) `#17 <https://github.com/boschmitt/tweedledum/pull/17>`_
    - View compute nodes path sums (`pathsum_view`) `#36 <https://github.com/boschmitt/tweedledum/pull/36>`_

v1.0-alpha [`December 17, 2018 <https://github.com/boschmitt/tweedledum/tree/v1.0-alpha>`_]
-------------------------------------------------------------------------------------------

Implements DATE'19 paper: **Compiling Permutations for Superconducting QPUs**

* Initial network and gate interfaces 
* Gate implementations:
    - Multiple Control Multiple Target quantum gate (`mcmt_gate`)
    - Multiple Control Single Target quantum gate (`mcst_gate`)
* Network implementations:
    - Netlist (`netlist`)
* Algorithms:
    - CNOT Patel (`cnot_patel`)
    - Decompostion based synthesis (`dbs`)
    - Gray synthesis (`gray_synth`)
    - Linear synthesis (`linear_synth`)
    - Single targe gate synthesis (`stg_from_pkrm`, `stg_from_pprm`, `stg_from_spectrum`)
* I/O
    - Open QASM 2.0 (`write_qasm`)
    - Quil (`write_quil`)
    - Write to qpic (`write_qpic`)
    - Write to unicode (`write_unicode`) 
* Utility data structures:
    - Angle (`angle`)
    - Bit matrix (Column-Major) (`bit_matrix_cm`)
    - Bit matrix (Row-Major) (`bit_matrix_rm`)
    - Dynamic bitset (`dynamic_bitset`)
