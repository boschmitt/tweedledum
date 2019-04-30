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

#define DEFINE_COMPLEX_H_VARIABLES
#include "QMDDcomplex.h"



uint32_t Ctentries;					 // number of complex table entries


mpreal Ctol;
static mpreal Pi;	// Pi is defined using asin function in QMDDinit routine

bool my_compare(__mpfr_struct& x, __mpfr_struct& y);
struct my_double_cmp
{
    bool operator() ( __mpfr_struct x, __mpfr_struct y ) const {
    	return my_compare(x,y);
    }
};

std::map<__mpfr_struct, uint32_t, my_double_cmp> Ctable2; // Table for reverse lookup of complex numbers
std::unordered_map<uint64_t, mpreal> Cangle; //mpfr_t /*long double*/ Cangle[COMPLEXTSIZE];// angle to avoid repeated computation
uint64_t CTa[MAXRADIX];				 // complex table positions for roots of unity


using mpfr::mpreal;



mpfr_t tmp,tmp2;
complex tmp_c;
void QMDDpause(void);

/**************************************

    Routines
    
**************************************/

mpreal QMDDcos(int fac, double div) {
	mpreal m = cos(Pi * fac/div);
	return m;
}

mpreal QMDDsin(int fac, double div) {
	mpreal m = sin(Pi * fac/div);
	return m;
}


complex Cvalue(uint64_t ci) {
	complex c;
	uint32_t r,i;
	r = (uint32_t) (ci >> 32) & 0x7FFFFFFFu;
	i = (uint32_t) (ci & 0x0FFFFFFFFul) & 0x7FFFFFFF;
	*c.r = Ctable[r];
	*c.i = Ctable[i];
	return c;
}

bool my_compare(__mpfr_struct& x, __mpfr_struct& y) {
	mpfr_sub(tmp, &x, &y, MPFR_RNDN);

	bool sign = mpfr_signbit(tmp);
	mpfr_abs(tmp, tmp, MPFR_RNDN);

	if(mpfr_cmp(tmp, Ctol.mpfr_srcptr())<=0) {
		return 0;
	}

	//int cmp = mpfr_cmp(&x, &y);
	//return cmp <0;
	return sign;
}

void Cprint(uint64_t i, std::ostream &os)
{
	complex c = Cvalue(i);
	int sign_r = (i >> 63) & 1;
	int sign_i = ((i >> 31) & 1);

	if(i == 0ull) {
		os << "0";
		return;
	}

	bool print = false;

	if((i >> 32) != 0ull) {
		if(sign_r /*mpfr_signbit(c.r)*/) {
		   os << "-";
		}

		os << mpfr_get_d(c.r, MPFR_RNDN);
		print = true;
	}
	if((i & 0x0000FFFF) != 0ull) {
		if (!sign_i/*mpfr_signbit(c.i)*/ ) {
			//mpfr_sprintf (buf, "%.128RNf", c.i);
			os << "+" << mpfr_get_d(c.i, MPFR_RNDN) << "i";
			// os << "+" << buf << "i";
		} else {
			//mpfr_sprintf (buf, "%.128RNf", c.i);
			os << "-" << mpfr_get_d(c.i, MPFR_RNDN) << "i";
			//  os << "-" << buf << "i";
		}
		print = true;
	}

	if(!print) {
		std::cout << "ERROR in Cprint: " << i << std::endl;
			exit(1);
	}
}

void Cprint(uint64_t i)
// print a complex value
{
	std::ostringstream oss;
	Cprint(i, oss);
	std::cout << oss.str();  
}

void angle(mpfr_t res, uint64_t a)
// computes angle for polar coordinate representation of Cvalue(a)
{
  complex ca;
  ca=Cvalue(a);

  int sign_r = (a >> 63) & 1;
  int sign_i = (a >> 31) & 1;

  std::unordered_map<uint64_t, mpreal>::iterator it = Cmag.find(a);

  mpfr_div(res, ca.r, it->second.mpfr_srcptr(), MPFR_RNDN);
  if(sign_r) {
	  mpfr_neg(res,res, MPFR_RNDN);
  }
  mpfr_acos(res, res, MPFR_RNDN); //res = acos(res);

  if((sign_i) &&  mpfr_cmp(ca.i, tmp) > 0) {
	  mpfr_mul_2ui(tmp, Pi.mpfr_srcptr(), 1, MPFR_RNDN);
	  mpfr_sub(res, tmp, res, MPFR_RNDN);
  }
}

uint64_t Cdiv(uint64_t ai,uint64_t bi); /* prototype */

int Cgt(uint64_t a, uint64_t b)
{  
  //complex ca,cb;
  if(a==b) return(0);
  
  if (a == 0)
    return(1);
  if (b == 0)
    return(0);


  std::unordered_map<uint64_t, mpreal>::iterator it = Cmag.find(b);
  mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);

  it = Cmag.find(a);
  if(mpfr_cmp(it->second.mpfr_ptr(),tmp) > 0) {
	  return 1;
  }
  mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);
  it = Cmag.find(b);
  if(mpfr_cmp(it->second.mpfr_srcptr(), tmp) >0) {
	  return(0);
  }
  //CHANGED by pN 120831
  it = Cangle.find(a);

  mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);
  it = Cangle.find(b);
  int ret_val = mpfr_cmp(tmp, it->second.mpfr_srcptr()) < 0;
  return ret_val;
}

int Cgt_new(uint64_t a, uint64_t b)
{  
  //complex ca,cb;
  if(a==b) return(0);
  //ca=Cvalue(a);
  //cb=Cvalue(b);

  std::unordered_map<uint64_t, mpreal>::iterator it = Cangle.find(a);

  mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);
  it = Cangle.find(b);

  if(mpfr_cmp(tmp, it->second.mpfr_srcptr()) <0) {
	  return(1);
  }

  it = Cmag.find(b);
  mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);
  it = Cmag.find(a);

  int ret_val = (mpfr_cmp(it->second.mpfr_srcptr(), tmp) > 0);
  return ret_val;
}

int Clt(uint64_t a, uint64_t b)
// analogous to Cgt
{
  //complex ca,cb;
  if(a==b) return(0);
  //ca=Cvalue(a);
  //cb=Cvalue(b);

  std::unordered_map<uint64_t, mpreal>::iterator it = Cmag.find(b);

  mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);

  it = Cmag.find(a);
  if(mpfr_cmp(it->second.mpfr_srcptr(), tmp) < 0) {
	  return(1);
  }
  mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);
  it = Cmag.find(b);
  if(mpfr_cmp(it->second.mpfr_srcptr(), tmp) < 0) {
	  return(0);
  }
  it = Cangle.find(a);
  mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);
  it = Cangle.find(b);
  int ret_val = (mpfr_cmp(tmp,it->second.mpfr_srcptr())>0);
  return ret_val;
}

uint64_t Cmake(mpreal r,mpreal i)
// make a complex value
{
  mpfr_set(tmp_c.r, r.mpfr_ptr(), MPFR_RNDN);
  mpfr_set(tmp_c.i, i.mpfr_ptr(), MPFR_RNDN);

  return Clookup(tmp_c);
}

complex CmakeOne(void)
{
	complex c;
	mpfr_init2(c.r, PREC);
	mpfr_init2(c.i, PREC);
	mpfr_set_si(c.r, 1, MPFR_RNDN);
	mpfr_set_si(c.i, 0, MPFR_RNDN);

	return c;
}

complex CmakeZero(void)
{
	  complex c;
		mpfr_init2(c.r, PREC);
		mpfr_init2(c.i, PREC);
		mpfr_set_si(c.r, 0, MPFR_RNDN);
		mpfr_set_si(c.i, 0, MPFR_RNDN);
	  return c;
}

complex CmakeMOne(void)
{
	  complex c;
	mpfr_init2(c.r, PREC);
		mpfr_init2(c.i, PREC);
		mpfr_set_si(c.r, -1, MPFR_RNDN);
		mpfr_set_si(c.i, 0, MPFR_RNDN);
	  return c;
}

mpreal Qmake(int a, int b,int c)
// returns the complex number equal to (a+b*sqrt(2))/c
// required to be compatible with quadratic irrational-based 
// complex number package
{
	mpreal res;
	res = (a+b*sqrt(2))/c;
	mpfr_set(tmp_c.r, res.mpfr_srcptr(), MPFR_RNDN);
	return res;
}

void Cfree(complex& c) {
	mpfr_clear(c.i);
	mpfr_clear(c.r);
}


void QMDDinitCtable(void)
// initialize the complex value table and complex operation tables to empty
{
  Ctentries=0;

  if(VERBOSE) printf("\nDouble complex number package initialized\n\n");
}

void QMDDcomplexInit(void)
// initialization
{

	mpreal::set_default_prec(PREC);

	Pi = 2 * acos(mpreal(0));


	mpfr_init2(tmp, PREC);
	mpfr_init2(tmp2, PREC);
	mpfr_init2(tmp_c.i, PREC);
	mpfr_init2(tmp_c.r, PREC);

	Ctol = mpreal(1e-10); //mpreal(1e-20);

	mpreal mag1,mag2;
	mag1 = mpreal(0);
	Cmag.insert(std::pair<uint64_t, mpreal>(0x0000000000000000ull, mag1));
	mag2 = mpreal(1);
	Cmag.insert(std::pair<uint64_t, mpreal>(0x0000000100000000ull, mag2));

	QMDDinitCtable();

	complex tmp_complex = CmakeZero();
	Clookup(tmp_complex);
	Cfree(tmp_complex);

	tmp_complex = CmakeOne();
	Clookup(tmp_complex);
	Cfree(tmp_complex);
}

void QMDDcvalue_table_list(void)
// print the complex value table entries
{
  
  printf("\nComplex value table: %d entries\n",Ctentries);
  std::cout << "index value Magnitude Angle 1) radian 2) degree" << std::endl;

  std::unordered_map<uint32_t, __mpfr_struct>::iterator it = Ctable.begin();
  for(;it!= Ctable.end(); it++) {
	  std::cout << it->first << " ->";
	  //Cprint(it->second, it->first);
	  std::cout << std::endl;

  }
}


uint64_t Clookup(complex& c)
// lookup a complex value in the complex value table
// if not found add it
// this routine uses linear searching
{
  uint32_t r,i;

 // 	  std::cout << "lookup " << mpreal(c.r) << " + " << mpreal(c.i) << "i" << std::endl;
//  Cprint(c,0);
//  std::cout << std::endl;

//  std::cout << "Look up "; Cprint(c,0); std::cout<<std::endl;


  int sign_r = (mpfr_signbit(c.r) ? 1 : 0);
  int sign_i = (mpfr_signbit(c.i) ? 1 : 0);

  mpfr_abs(c.r,c.r, MPFR_RNDN);
  mpfr_abs(c.i,c.i, MPFR_RNDN);

  complex new_c;

  if(mpfr_zero_p(c.r)) {
	  sign_r = 0;
  }
  if(mpfr_zero_p(c.i)) {
	  sign_i = 0;
  }


  std::map<__mpfr_struct, uint32_t, my_double_cmp>::iterator it;

  it = Ctable2.find(*c.r);
  if(it != Ctable2.end()) {
	  r = it->second;
//	  std::cout << "found r: " << r << std::endl;
  } else {
	  r = Ctentries++;
//	  std::cout << "did not find r: set to " << r << std::endl;
	  mpfr_init2(new_c.r, PREC);

	  mpfr_set(new_c.r, c.r, MPFR_RNDN);


	  Ctable[r] = *new_c.r;
	  Ctable2[*new_c.r]=r;
  }

  if(sign_r && (r & 0x7FFFFFFFul)) {
	  r |= 0x80000000ull;
   	  mpfr_neg(c.r, c.r, MPFR_RNDN);
  }

  it = Ctable2.find(*c.i);
  if(it != Ctable2.end()) {
	  i = it->second;
//	  std::cout << "found i: " << i << std::endl;
  } else {
	  i = Ctentries++;
//	  std::cout << "did not find i: set to " << i << std::endl;
	  mpfr_init2(new_c.i, PREC);

	  mpfr_set(new_c.i, c.i, MPFR_RNDN);

	  Ctable[i] = *new_c.i;
	  Ctable2[*new_c.i]=i;
  }

  if(Ctentries > 0x7FFFFFFFu) {
	  std::cerr << "Complex mapping overflow!" << std::endl;
	  exit(0);
  }

  if(sign_i && (i & 0x7FFFFFFFul)) {
	  i |= 0x80000000ul;
   	  mpfr_neg(c.i, c.i, MPFR_RNDN);
  }

  uint64_t ret_val = (((uint64_t)r) << 32) | ((uint64_t)i);

  if(Cmag.find(ret_val & 0x7FFFFFFF7FFFFFFFull) == Cmag.end()) {
	  mpreal new_mag, new_angle;
	  new_mag = mpreal();
	  new_angle = mpreal();

	  mpfr_mul(new_mag.mpfr_ptr(), c.r, c.r, MPFR_RNDN);
	  mpfr_mul(new_angle.mpfr_ptr(), c.i, c.i, MPFR_RNDN);
	  mpfr_add(tmp, new_mag.mpfr_srcptr(), new_angle.mpfr_srcptr(), MPFR_RNDN);
	  mpfr_sqrt(new_mag.mpfr_ptr(), tmp, MPFR_RNDN);
//	  std::pair<unsigned int, mpreal> my_pair = std::make_pair(i, new_mag);

	  Cmag[ret_val & 0x7FFFFFFF7FFFFFFFull]=new_mag;
  }
  return ret_val;
}

uint64_t Conj(uint64_t a)
// return complex conjugate
{
	if((a & 0xFFFFFFFFull) == 0ull) {
		return a;
	}
	return a ^ 0x80000000ull;
}


// basic operations on complex values
// meanings are self-evident from the names
// NOTE arguments are the indices to the values 
// in the complex value table not the values themselves

uint64_t Cnegative(uint64_t a)
{
  uint64_t r, i;
  r = a >> 32;
  i = a & 0x0FFFFFFFFull;
  if(r != 0ull) {
	  r ^= 0x80000000ull;
  }
  if(i != 0ull) {
	 i ^= 0x80000000ull;
  }

  return (r << 32) | i;
}

uint64_t Cadd(uint64_t ai,uint64_t bi)
{
  complex a,b;
  uint64_t t;
  
  if(ai==0ull) return(bi); // identity cases
  if(bi==0ull) return(ai);
  if(ai == Cnegative(bi)) return(0ull);

  std::unordered_map< std::pair<uint64_t, uint64_t>, uint64_t, pair_hash>::iterator it;
  std::pair<uint64_t, uint64_t> key = std::make_pair(ai, bi);

  it = cta.find(key);

  if(it != cta.end()) {
	  return it->second;
  }

  a=Cvalue(ai); // if new compute result
  b=Cvalue(bi); 

  int sign_ar = (ai >> 63) & 1u;
  int sign_ai = (ai >> 31) & 1u;
  int sign_br = (bi >> 63) & 1u;
  int sign_bi = (bi >> 31) & 1u;

  if(sign_ar ^ sign_br) {
	  mpfr_sub(tmp_c.r, a.r, b.r, MPFR_RNDN);
  } else {
	  mpfr_add(tmp_c.r, a.r, b.r, MPFR_RNDN);
  }
  if(sign_ar) {
	  mpfr_neg(tmp_c.r, tmp_c.r, MPFR_RNDN);
  }

  if(sign_ai ^ sign_bi) {
	  mpfr_sub(tmp_c.i, a.i, b.i, MPFR_RNDN);
  } else {
	  mpfr_add(tmp_c.i, a.i, b.i, MPFR_RNDN);
  }
  if(sign_ai) {
	  mpfr_neg(tmp_c.i, tmp_c.i, MPFR_RNDN);
  }

  t=Clookup(tmp_c); // save result
  cta.insert(std::make_pair(key, t));
  key = std::make_pair(bi, ai);
  cta.insert(std::make_pair(key, t));
/*  std::cout << " = " << t << " ( ";
  Cprint(tmp_c, 0);
  std::cout << " )" << std::endl;*/
  return(t);
}

uint64_t Csub(uint64_t ai,uint64_t bi)
{
  complex a,b;
  uint64_t t;
  
  if(bi==0x0ull) return(ai); // identity case
  if(ai==0x0ull) return(Cnegative(bi));
  if(ai == bi) return 0ull;

  std::unordered_map<std::pair<uint64_t, uint64_t>, uint64_t, pair_hash>::iterator it;
  std::pair<uint64_t, uint64_t> key;
  key = std::make_pair(ai, bi);
  it = cts.find(key);
  if(it != cts.end()) {
	  return it->second;
  }
  
  a=Cvalue(ai);  // if new compute result
  b=Cvalue(bi);

  int sign_ar = (ai >> 63) & 1u;
  int sign_ai = (ai >> 31) & 1u;
  int sign_br = (bi >> 63) & 1u;
  int sign_bi = (bi >> 31) & 1u;

  if(sign_ar ^ sign_br) {
	  mpfr_add(tmp_c.r, a.r, b.r, MPFR_RNDN);
   } else {
  	  mpfr_sub(tmp_c.r, a.r, b.r, MPFR_RNDN);
    }
    if(sign_ar) {
  	  mpfr_neg(tmp_c.r, tmp_c.r, MPFR_RNDN);
    }

    if(sign_ai ^ sign_bi) {
  	  mpfr_add(tmp_c.i, a.i, b.i, MPFR_RNDN);
    } else {
  	  mpfr_sub(tmp_c.i, a.i, b.i, MPFR_RNDN);
    }
    if(sign_ai) {
  	  mpfr_neg(tmp_c.i, tmp_c.i, MPFR_RNDN);
    }

  t=Clookup(tmp_c); // save result
  cts.insert(std::make_pair(key, t));
  return(t);
}

uint64_t Cmul(uint64_t ai,uint64_t bi)
{

  complex a,b;
  uint64_t t;

  if(ai==0x0000000100000000ull) {
	  return(bi); // identity cases
  }
  if(bi==0x0000000100000000ull) {
	  return(ai);
  }
  if(ai==0ull||bi==0ull) {
	  return(0x0ull);
  }

  if(ai == 0x8000000100000000ull) {
	  return Cnegative(bi);
  }
  if(bi == 0x8000000100000000ull) {
	  return Cnegative(ai);
  }
  
  std::unordered_map<std::pair<uint64_t, uint64_t>, uint64_t, pair_hash>::iterator it;
  std::pair<uint64_t, uint64_t> key = std::make_pair(ai, bi);

  it = ctm.find(key);
  if(it != ctm.end()) {
	  return it->second;
  }
  
  a=Cvalue(ai); // if new compute result
  b=Cvalue(bi);

  int sign_ar = (ai >> 63) & 1u;
  int sign_ai = (ai >> 31) & 1u;
  int sign_br = (bi >> 63) & 1u;
  int sign_bi = (bi >> 31) & 1u;

  mpfr_mul(tmp, a.r, b.r, MPFR_RNDN);
  if(sign_ar ^ sign_br) {
	  mpfr_neg(tmp, tmp, MPFR_RNDN);
  }
  mpfr_mul(tmp2, a.i, b.i, MPFR_RNDN);
  if(sign_ai ^ sign_bi) {
  	  mpfr_neg(tmp2, tmp2, MPFR_RNDN);
  }
  mpfr_sub(tmp_c.r, tmp, tmp2, MPFR_RNDN);
  //tmp_c.r = a.r * b.r - a.i*b.i;

  mpfr_mul(tmp, a.r, b.i, MPFR_RNDN);
  if(sign_ar ^ sign_bi) {
  	  mpfr_neg(tmp, tmp, MPFR_RNDN);
  }
  mpfr_mul(tmp2, a.i, b.r, MPFR_RNDN);
  if(sign_ai ^ sign_br) {
  	  mpfr_neg(tmp2, tmp2, MPFR_RNDN);
  }
  mpfr_add(tmp_c.i, tmp, tmp2, MPFR_RNDN);
  //tmp_c.i = a.r * b.i + a.i * b.r;

  t=Clookup(tmp_c); // save result

  ctm.insert(std::make_pair(key, t));
  key = std::make_pair(bi, ai);
  ctm.insert(std::make_pair(key, t));
/*  std::cout << " = " << t << " ( ";
  Cprint(tmp_c, 0);
  std::cout << " )" << std::endl;*/
  return(t);
}

uint64_t CintMul(int a,uint64_t bi)
{
  complex r;
  r=Cvalue(bi);

  int sign_br = (bi >> 63) & 1u;
  int sign_bi = (bi >> 31) & 1u;

  mpfr_mul_si(tmp_c.r, r.r, a, MPFR_RNDN);
  if(sign_br) {
	  mpfr_neg(tmp_c.r, tmp_c.r, MPFR_RNDN);
  }
  mpfr_mul_si(tmp_c.i, r.i, a, MPFR_RNDN);
  if(sign_bi) {
	  mpfr_neg(tmp_c.i, tmp_c.i, MPFR_RNDN);
  }

  uint64_t t = (Clookup(tmp_c));
  return t;
}

uint64_t Cdiv(uint64_t ai, uint64_t bi)
{
  complex a,b;
  uint64_t t;
  
//  std::cout << "Cdiv: " << ai << "/" << bi<<std::endl;

  if(ai==bi) return(0x100000000ull); // equal case
  if(ai==0ull) return(0x0ull); // identity cases
  if(bi==0x0000000100000000ull) return(ai);

  if(bi == 0x8000000100000000ull) {
	  return Cnegative(ai);
  }
  if(ai == Cnegative(bi)) {
	  return 0x8000000100000000ull;
  }
  //TODO: check whether b != 0

  std::unordered_map<std::pair<uint64_t, uint64_t>, uint64_t, pair_hash>::iterator it;
  std::pair<uint64_t, uint64_t> key = std::make_pair(ai, bi);
  it = ctd.find(key);
  if(it != ctd.end()) {
	  return it->second;
  }

  a=Cvalue(ai); // if new compute result
  b=Cvalue(bi);

  int sign_ar = (ai >> 63) & 1u;
  int sign_ai = (ai >> 31) & 1u;
  int sign_br = (bi >> 63) & 1u;
  int sign_bi = (bi >> 31) & 1u;

  if(mpfr_zero_p(b.i))
  {
	  mpfr_div(tmp_c.r, a.r, b.r, MPFR_RNDN);
	  if(sign_ar ^ sign_br) {
		  mpfr_neg(tmp_c.r, tmp_c.r, MPFR_RNDN);
	  }
	  mpfr_div(tmp_c.i, a.i, b.r, MPFR_RNDN);
	  if(sign_ai ^ sign_br) {
		  mpfr_neg(tmp_c.i, tmp_c.i, MPFR_RNDN);
	  }
  } else {
	  //tmp2 = mpreal();

	  mpfr_mul(tmp, b.r, b.r, MPFR_RNDN);
	  mpfr_mul(tmp2, b.i, b.i, MPFR_RNDN);
	  mpfr_add(tmp, tmp, tmp2, MPFR_RNDN);
	  //tmp = b.r * b.r + b.i * b.i;

	  mpfr_mul(tmp_c.r, a.r, b.r, MPFR_RNDN);
	  if(sign_ar ^ sign_br) {
	  	  mpfr_neg(tmp_c.r, tmp_c.r, MPFR_RNDN);
	  }
	  mpfr_mul(tmp2, a.i, b.i, MPFR_RNDN);
	  if(sign_ai ^ sign_bi) {
	  	  mpfr_neg(tmp2, tmp2, MPFR_RNDN);
	  }
	  mpfr_add(tmp_c.r, tmp_c.r, tmp2, MPFR_RNDN);
	  mpfr_div(tmp_c.r, tmp_c.r, tmp, MPFR_RNDN);

	  //tmp_c.r = (a.r*b.r + a.i*b.i)/tmp;

	  mpfr_mul(tmp_c.i, a.i, b.r, MPFR_RNDN);
	  if(sign_ai ^ sign_br) {
	  	  mpfr_neg(tmp_c.i, tmp_c.i, MPFR_RNDN);
	  }

	  mpfr_mul(tmp2, a.r, b.i, MPFR_RNDN);
	  if(sign_ar ^ sign_bi) {
	  	  mpfr_neg(tmp2, tmp2, MPFR_RNDN);
	  }
	  mpfr_sub(tmp_c.i, tmp_c.i, tmp2, MPFR_RNDN);
	  mpfr_div(tmp_c.i, tmp_c.i, tmp, MPFR_RNDN);

	  //tmp_c.i = (a.i * b.r - a.r * b.i)/tmp;
  }
  t=Clookup(tmp_c); // save result

  ctd.insert(std::make_pair(key, t));

  return(t);
}

void QMDDmakeRootsOfUnity(void)
{
  int i;
  CTa[0]=1;

  mpreal r,s;
  r = mpreal();

  r = Pi * 2 / Radix;

  s = mpreal(r);
  r = cos(r);
  s = sin(s);

  CTa[1] = Cmake(r,s);

  for(i=2;i<Radix;i++)
    CTa[i]=Cmul(CTa[i-1],CTa[1]);
}

/// by PN: returns the absolut value of a complex number
uint64_t CAbs(uint64_t a)
{
  uint64_t b;

  if (a == 0x0000000100000000ull || a == 0x0000000000000000ull) return a; // trivial cases 0/1
  if(a == 0x8000000100000000ull) return 0x0000000100000000ull;
  
    //s=Cvalue(a);
  //printf("CAbs: "); Cprint(s); printf(" is ");

   std::unordered_map<uint64_t, mpreal>::iterator it = Cmag.find(a);

   mpfr_set(tmp_c.r, it->second.mpfr_srcptr(), MPFR_RNDN);
   mpfr_set_si(tmp_c.i, 0, MPFR_RNDN);
   b = Clookup(tmp_c);
  //Cprint(r);   printf("\n");
  return(b);
}

///by PN: returns whether a complex number has norm 1
int CUnit(uint64_t a)
{
 /// BETA 121017
 
 if (a == 0x0000000100000000ull || a == 0x0000000000000000ull || a == 0x8000000100000000ull)
   return a;

 std::unordered_map<uint64_t, mpreal>::iterator it = Cmag.find(a);

 mpfr_add(tmp, it->second.mpfr_srcptr(), Ctol.mpfr_srcptr(), MPFR_RNDN);

 if (mpfr_cmp_si(tmp, 1)  < 0) {
	 return 0;
 }
 else {
    return 1;
 }
}

std::set<uint32_t> complex_entries;
std::set<QMDDnodeptr> visited_nodes;
std::set<uint64_t> cmag_entries;

void addToComplexTable(QMDDedge edge) {

	if(!QMDDterminal(edge)) {
		unsigned int before = visited_nodes.size();
		visited_nodes.insert(edge.p);
		if(before != visited_nodes.size()) {
			for(int i = 0; i < MAXNEDGE; i++) {
				cmag_entries.erase(edge.p->e[i].w & 0x7FFFFFFF7FFFFFFFull);
				uint32_t cr = (uint32_t)((edge.p->e[i].w >> 32) & 0x7FFFFFFFull);
				uint32_t ci = (uint32_t)(edge.p->e[i].w & 0x7FFFFFFFull);

				if(cr > 1) {
					complex_entries.erase(cr);
				}
				if(ci > 1) {
					complex_entries.erase(ci);
				}
			}
			for(int i = 0; i < MAXNEDGE; i++) {
				addToComplexTable(edge.p->e[i]);
			}
		}
	}
}

void cleanCtable(std::vector<QMDDedge> save_edges) {

//	std::cout << "before clean: " << std::endl;
//	std::cout << "  Ctable.size() = " << Ctable.size() << std::endl;
//	std::cout << "  Ctable2.size() = " << Ctable2.size() << std::endl;

	complex_entries.clear();
	std::vector<complex> complex_entries2;
	std::unordered_map<uint32_t, __mpfr_struct>::iterator it;
	for(it = Ctable.begin(); it != Ctable.end(); it++) {
		complex_entries.insert(it->first);
	}
	cmag_entries.clear();
	for(std::unordered_map<uint64_t, mpreal>::iterator it = Cmag.begin(); it != Cmag.end(); it++) {
		cmag_entries.insert(it->first & 0x7FFFFFFF7FFFFFFFull);
	}
	visited_nodes.clear();

	for(std::vector<QMDDedge>::iterator it = save_edges.begin(); it != save_edges.end(); it++) {
		QMDDedge e = *it;

		cmag_entries.erase(e.w & 0x7FFFFFFF7FFFFFFFull);
		uint32_t cr = (uint32_t)((e.w >> 32) & 0x7FFFFFFFull);
		uint32_t ci = (uint32_t)(e.w & 0x7FFFFFFFull);

		if(cr > 1) {
			complex_entries.erase(cr);
		}
		if(ci > 1) {
			complex_entries.erase(ci);
		}
		addToComplexTable(e);
	}
	complex_entries.erase(0x0ul);
	complex_entries.erase(0x1ul);
	cmag_entries.erase(0x100000000ull);
	cmag_entries.erase(0x0ull);

	for(int i = 0; i <8; i++) {
		complex_entries.erase(CTa[i]);
		cmag_entries.erase(CTa[i] & 0x7FFFFFFF7FFFFFFFull);
	}

	complex_entries.erase((uint32_t)(Vm[0][0] & 0x7FFFFFFFull));
	complex_entries.erase((uint32_t)((Vm[0][0] >> 32) & 0x7FFFFFFFull));

	complex_entries.erase((uint32_t)(Vm[0][1] & 0x7FFFFFFFull));
	complex_entries.erase((uint32_t)((Vm[0][1] >> 32) & 0x7FFFFFFFull));

	complex_entries.erase((uint32_t)(Hm[0][0] & 0x7FFFFFFFull));
	complex_entries.erase((uint32_t)((Hm[0][0] >> 32) & 0x7FFFFFFFull));

	complex_entries.erase((uint32_t)(Hm[1][1] & 0x7FFFFFFFull));
	complex_entries.erase((uint32_t)((Hm[1][1] >> 32) & 0x7FFFFFFFull));


	cmag_entries.erase(Vm[0][0]& 0x7FFFFFFF7FFFFFFFull);
	cmag_entries.erase(Vm[0][1]& 0x7FFFFFFF7FFFFFFFull);
	cmag_entries.erase(Hm[0][0]& 0x7FFFFFFF7FFFFFFFull);
	cmag_entries.erase(Hm[1][1]& 0x7FFFFFFF7FFFFFFFull);

	//complex_entries.erase(2);

	std::set<uint32_t>::iterator it2;
	std::unordered_map<uint64_t, mpreal>::iterator it3;

	__mpfr_struct val;
	for(it2 = complex_entries.begin(); it2 != complex_entries.end(); it2++) {
		val = Ctable[*it2];
		std::map<__mpfr_struct, uint32_t, my_double_cmp>::iterator it4 = Ctable2.find(val);

		if(it4 == Ctable2.end()) {
			std::cout << "ERROR: Entry not found: ";
			//Cprint(c, 0);
			std::cout << std::endl;
		}
		if(it4->second != *it2) {
			std::cout << "ERROR: different numbers seem to be equal: " << it4->second << " and " << *it2 << std::endl;
			//Cprint(it4->first, it4->second);
			std::cout << " and ";
			//Cprint(c, *it2);
			std::cout << std::endl;
			exit(0);
		}
		if(it4 == Ctable2.end()) {
			std::cout << "ERROR: could not delete complex entry: " << *it2 << std::endl;
		} else {
			Ctable2.erase(it4);
		}
		Ctable.erase(*it2);



		//it3 = Cmag.find(*it2);
		//Cmag.erase(it3);
		//it3 = Cangle.find(*it2);
		//Cangle.erase(it3);

		//mpfr_clear(c.r);
		mpfr_clear(&val);
	}

	for(std::set<uint64_t>::iterator it=cmag_entries.begin(); it!=cmag_entries.end(); it++) {
		Cmag.erase(*it);
	}


	QMDDinitComputeTable();
	ctm.clear();
	cta.clear();
	cts.clear();
	ctd.clear();
}

