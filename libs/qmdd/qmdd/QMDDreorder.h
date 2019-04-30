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


#ifndef QMDDreorder_H
#define QMDDreorder_H

//#include <stdio.h>

#include "QMDDpackage.h"
//#include "QMDDcomplex.h"
#include "timing.h"
//#include "textFileUtilities.h"
//#include <string.h>
//#include <iostream>

/*****************************************************************

    Routines            
*****************************************************************/

int siftingCostFunction(QMDDedge a);

int checkandsetBlockProperty(QMDDedge a);
int checkBlockMatrices(QMDDedge a, int triggerValue);
void QMDDrestoreSpecialMatrices(QMDDedge a);
void QMDDmarkupSpecialMatrices(QMDDedge a);
void QMDDresetVertexWeights(QMDDedge a, uint64_t standardValue);

QMDDedge QMDDbuildIntermediate(QMDDedge a);
QMDDedge QMDDrenormalize(QMDDedge a);
void QMDDchangeNonterminal(short v,QMDDedge edge[],QMDDnodeptr p);
int QMDDcheckDontCare(QMDDnodeptr p, int v2);
void QMDDswapNode(QMDDnodeptr p,int v1,int v2, int swap);
void QMDDswap(int i);
int QMDDsift(int n, QMDDedge *root, QMDDrevlibDescription *circ, std::ostream &os);
int myQMDDsift(int n, QMDDedge *root, QMDDrevlibDescription *circ, std::ostream &os, int lowerbound, int upperbound);
int QMDDsift(int n, QMDDedge *root, QMDDrevlibDescription *circ);
int lookupLabel(char buffer[], char moveLabel[], QMDDrevlibDescription *circ);
void QMDDreorder(int order[],int n, QMDDedge *root);
void myQMDDreorder(int order[],int n, QMDDedge *root);
int QMDDmoveVariable(QMDDedge *basic, char buffer[], QMDDrevlibDescription *circ);
void SJTalgorithm(QMDDedge a, int n);
/*******************************************************************************/
#endif
