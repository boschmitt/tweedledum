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

#ifndef SRC_SIMULATOR_H_
#define SRC_SIMULATOR_H_

#include <QMDDcore.h>
#include <QMDDpackage.h>
#include <QMDDcomplex.h>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>

#include <gmp.h>
#include <mpreal.h>

#define VERBOSE 0


class Simulator {
public:
	Simulator();
	virtual void Simulate() = 0;
	virtual void Simulate(int shots) = 0;
	virtual void Reset();
	int GetGatecount() {
		return gatecount;
	}
	int GetQubits() {
		return nqubits;
	}
	int GetMaxActive() {
		return max_active;
	}
	virtual ~Simulator();

protected:
	int MeasureOne(int index);
	void MeasureAll(bool reset_state=true);
	void ApplyGate(QMDD_matrix& m);
	void ApplyGate(QMDDedge gate);
	void AddVariables(int add, std::string name);
	void ResetQubit(int index);
	mpreal GetProbability();

	int line[MAXN];
	int measurements[MAXN];
	unsigned int nqubits = 0;
	QMDDrevlibDescription circ;

	bool intermediate_measurement = false;
	void ResetBeforeMeasurement();

	uint64_t GetElementOfVector(unsigned long long element);
private:

	mpreal GetProbabilityRec(QMDDedge& e);
	QMDDedge AddVariablesRec(QMDDedge e, QMDDedge t, int add);
	mpreal AssignProbs(QMDDedge& e);
	std::pair<mpreal, mpreal> AssignProbsOne(QMDDedge e, int index);

	std::unordered_map<QMDDnodeptr, mpreal> probs;
	std::map<QMDDnodeptr,mpreal> probsMone;
	std::set<QMDDnodeptr> visited_nodes2;
	std::map<QMDDnodeptr, QMDDedge> dag_edges;

	int max_active = 0;
	unsigned int complex_limit = 10000;
	int gatecount = 0;
	int max_gates = 0x7FFFFFFF;

	bool measurement_done = false;
	mpreal epsilon;
	QMDDedge beforeMeasurement;
};

#endif /* SRC_SIMULATOR_H_ */
