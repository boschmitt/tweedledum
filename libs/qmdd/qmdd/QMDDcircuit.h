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


#ifndef QMDDcircuit_H
#define QMDDcircuit_H

#include <stdio.h>

#include "QMDDpackage.h"
#include "QMDDcomplex.h"
#include "qcost.h"
#include "textFileUtilities.h"
#include <string.h>
#include <iostream>

/*****************************************************************

    Routines            
*****************************************************************/

int getlabel(char*,QMDDrevlibDescription,int*);

QMDDedge QMDDreadGateFromString(char*, QMDDrevlibDescription*);
QMDDedge QMDDreadGate(FILE*,QMDDrevlibDescription*);
QMDDrevlibDescription QMDDrevlibHeader(FILE*);
QMDDrevlibDescription QMDDcircuitRevlib(char *fname,QMDDrevlibDescription firstCirc,int match);
// reads a circuit in Revlib format: http://www.revlib.org/documentation.php 


/*******************************************************************************/
#endif
