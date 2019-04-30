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

#ifndef QASM_SIMULATOR_H_
#define QASM_SIMULATOR_H_

#include <QASMscanner.hpp>
#include <QASMtoken.hpp>
#include <Simulator.h>
#include <stack>

class QASMsimulator : public Simulator {
public:
	QASMsimulator(bool display_statevector, bool display_probabilities);
	QASMsimulator(std::string fname, bool display_statevector, bool display_probabilities);
	virtual ~QASMsimulator();

	void Simulate();
	void Simulate(int shots);
	void Reset();

private:
	class Expr {
	public:
		enum class Kind {number, plus, minus, sign, times, sin, cos, tan, exp, ln, sqrt, div, power, id};
		mpreal num;
		Kind kind;
		Expr* op1 = NULL;
		Expr* op2 = NULL;
		std::string id;

		Expr(Kind kind,  Expr* op1, Expr* op2, mpreal num, std::string id) {
			this->kind = kind;
			this->op1 = op1;
			this->op2 = op2;
			this->num = num;
			this->id = id;
		}

		Expr(const Expr& expr) {
			this->kind = expr.kind;
			this->num = expr.num;
			this->id = expr.id;
			if(expr.op1 != NULL) {
				this->op1 = new Expr(*expr.op1);
			}
			if(expr.op2 != NULL) {
				this->op2 = new Expr(*expr.op2);
			}
		}

		~Expr() {
			if(op1 != NULL) {
				delete op1;
			}
			if(op2 != NULL) {
				delete op2;
			}
		}
	};

	class BasisGate {
	public:
		virtual ~BasisGate() = default;
	};

	class Ugate : public BasisGate {
	public:
		Expr* theta;
		Expr* phi;
		Expr* lambda;
		std::string target;

		Ugate(Expr* theta, Expr* phi, Expr* lambda, std::string target) {
			this->theta = theta;
			this->phi = phi;
			this->lambda = lambda;
			this->target = target;
		}

		~Ugate() {
			if(theta != NULL) {
				delete theta;
			}
			if(phi != NULL) {
				delete phi;
			}
			if(lambda != NULL) {
				delete lambda;
			}
		}
	};

	class CXgate : public BasisGate {
	public:
		std::string control;
		std::string target;

		CXgate(std::string control, std::string target) {
			this->control = control;
			this->target = target;
		}
	};

	class CompoundGate {
	public:
		std::vector<std::string> parameterNames;
		std::vector<std::string> argumentNames;
		std::vector<BasisGate*> gates;
		bool opaque;
	};

	class Snapshot {
	public:
		~Snapshot() {
			if(probabilities != NULL) {
				delete[] probabilities;
			}
			if(statevector != NULL) {
				delete[] statevector;
			}
		}

		unsigned long long len;
		double* probabilities;
		std::string* statevector;
		std::map<std::string, double> probabilities_ket;
	};

	void scan();
	void check(Token::Kind expected);

	Token la,t;
	Token::Kind sym = Token::Kind::none;

	std::string fname;
  	std::istream* in;
	QASMscanner* scanner;
	std::map<std::string, std::pair<int ,int> > qregs;
	std::map<std::string, std::pair<int, int*> > cregs;
	std::pair<int, int> QASMargumentQreg();
	std::pair<std::string, int> QASMargumentCreg();
	Expr* QASMexponentiation();
	Expr* QASMfactor();
	Expr* QASMterm();
	QASMsimulator::Expr* QASMexp();
	void QASMgateDecl();
	void QASMopaqueGateDecl();
	void QASMgate(bool execute = true);
	void QASMqop(bool execute = true);
	void QASMexpList(std::vector<Expr*>& expressions);
	void QASMidList(std::vector<std::string>& identifiers);
	void QASMargsList(std::vector<std::pair<int, int> >& arguments);
	std::set<Token::Kind> unaryops {Token::Kind::sin,Token::Kind::cos,Token::Kind::tan,Token::Kind::exp,Token::Kind::ln,Token::Kind::sqrt};

	QMDD_matrix tmp_matrix;

	std::map<std::string, CompoundGate> compoundGates;
	Expr* RewriteExpr(Expr* expr, std::map<std::string, Expr*>& exprMap);
	void printExpr(Expr* expr);


	bool display_statevector;
	bool display_probabilities;

	std::map<int, Snapshot*> snapshots;
};

#endif /* QASM_SIMULATOR_H_ */
