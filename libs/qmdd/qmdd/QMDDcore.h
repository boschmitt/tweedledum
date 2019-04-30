/*
DD-based simulator by JKU Linz, Austria

Developer: Alwin Zulehner, Robert Wille

With code from the QMDD implementation provided by Michael Miller (University of Victoria, Canada)
and Philipp Niemann (University of Bremen, Germany).

For more information, please visit http://iic.jku.at/eda/research/quantum_simulation

If you have any questions feel free to contact us using
alwin.zulehner@jku.at or robert.wille@jku.at

If you use the quantum simulator for your research, we would be thankful if you referred to it
by citing the following publication:

@article{zulehner2018simulation,
    title={Advanced Simulation of Quantum Computations},
    author={Zulehner, Alwin and Wille, Robert},
    journal={IEEE Transactions on Computer Aided Design of Integrated Circuits and Systems (TCAD)},
    year={2018},
    eprint = {arXiv:1707.00865}
}
*/


#include <sstream>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
//#include "qcost.c"
#include "qcost.h"
//#include "timing.c"
#include "timing.h"
//#include "textFileUtilities.c"  	// basic text handling routines
#include "textFileUtilities.h"  	// basic text handling routines
#include "QMDDpackage.h"		// constants, type definitions and globals
#include "QMDDcomplex.h"
//#ifdef __HADAMARD__
//#include "QMDDcomplexH.c"		// complex Hadamard number package
//#else
//#include "QMDDcomplexD.c"		// complex number package
//#endif
//#include "QMDDpackage.c"    		// QMDD package
//#ifdef __SignatureSynthesis__
//#include "QMDDsigstuff.c"  		// include signature stuff
//#endif
#include "QMDDreorder.h"  		// sifting
#include "QMDDcircuit.h"		// procedures for building a QMDD from a circuit file

