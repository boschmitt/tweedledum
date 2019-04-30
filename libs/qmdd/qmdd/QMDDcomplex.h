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


#ifndef QMDDcomplex_H
#define QMDDcomplex_H

#include <mpreal.h>
#include <gmp.h>
#include <mpfr.h>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>

//#include <stdint.h>



#include <ostream>
#include <math.h>


using mpfr::mpreal;

#ifndef DEFINE_COMPLEX_H_VARIABLES
#define EXTERN_C	extern
#else
#define EXTERN_C
#endif

typedef struct
{
//	long double r,i;
   mpfr_t r;
   mpfr_t i;
} complex;

#include "QMDDpackage.h"

extern mpreal Ctol;


void Cprint(uint64_t, std::ostream&);
void Cprint(uint64_t); // print a complex value to STD_OUT

/*********************************************

Complex computation tables.  These tables save
result of computations over complex values to
avoid recomputation later.

  Cta - addition
  Cts - subtraction
  Ctm - multiplication
  Ctd - division

*********************************************/

struct pair_hash
{
  std::size_t operator()(std::pair<uint64_t,uint64_t> p) const
  {
    using std::size_t;
    using std::hash;

    return hash<uint64_t>()(p.first) ^ hash<uint64_t>()(p.second);
  }
};

EXTERN_C std::unordered_map< std::pair<uint64_t, uint64_t>, uint64_t, pair_hash> cta,cts,ctm,ctd;


complex Cvalue(uint64_t x);

int Cgt(uint64_t,uint64_t); // greater than
int Clt(uint64_t, uint64_t); // analogous to Cgt

void QMDDinitCtable(void); // initialize the complex value table and complex operation tables to empty
void QMDDcomplexInit(void); // initialization


void QMDDcvalue_table_list(void); // print the complex value table entries
uint64_t Clookup(complex&); // lookup a complex value in the complex value table; if not found add it

uint64_t Conj(uint64_t); /// return complex conjugate
std::pair<mpreal,mpreal> approx(uint64_t t);

// basic operations on complex values
// meanings are self-evident from the names
// NOTE arguments are the indices to the values
// in the complex value table not the values themselves

uint64_t Cnegative(uint64_t);
uint64_t Cadd(uint64_t,uint64_t);
uint64_t Csub(uint64_t,uint64_t);
uint64_t Cmul(uint64_t,uint64_t);
uint64_t CintMul(int, uint64_t); // multiply by an integer
uint64_t Cdiv(uint64_t,uint64_t);
void QMDDmakeRootsOfUnity(void);
uint64_t CAbs(uint64_t); /// by PN: returns the absolut value of a complex number
int CUnit(uint64_t a); ///by PN: returns whether a complex number has norm 1

//void Cfree(complex&);


#define COMPLEX_ZERO 0x0ull
#define COMPLEX_ONE 0x0000000100000000ull
#define COMPLEX_M_ONE 0x8000000100000000ull

//Czero is used in QMDDpackage.cpp and in QMDDcircuit.cpp to initialize the gate matrices (in combination with Cmake)
//#define Czero 0
#define PREC 200

EXTERN_C std::unordered_map<uint32_t, __mpfr_struct> Ctable; // value
EXTERN_C std::unordered_map<uint64_t, mpreal> Cmag; //mpfr_t /*long double*/ Cmag[COMPLEXTSIZE];  // magnitude to avoid repeated computation

mpreal QMDDcos(int fac, double div);
mpreal QMDDsin(int fac, double div);
void angle(mpfr_t, int); // computes angle for polar coordinate representation
uint64_t Cmake(mpreal, mpreal); // make a complex value
mpreal Qmake(int,int,int); // returns the complex number equal to (a+b*sqrt(2))/c
// required to be compatible with quadratic irrational-based 
// complex number package


#endif
