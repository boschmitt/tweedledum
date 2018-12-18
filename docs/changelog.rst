Change Log
==========

v1.0-alpha (`GitHub <https://github.com/boschmitt/tweedledum/tree/v1.0-alpha>`_) 
--------------------------------------------------------------------------------

Implements DATE'19 paper: **Compiling Permutations for Superconducting QPUs**

* Initial network and gate interfaces 
* Gate implementations:
    - Multiple Control Multiple Target quantum gate (`mcmt_gate`)
    - Multiple Control Single Target quantum gate (`mcst_gate`)
* Network implementations:
    - Netlist (`netlist`)
* Algorithms:
    - CNOT Patel (`netlist`)
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
