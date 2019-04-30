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

#include <Simulator.h>

Simulator::Simulator() {
	// TODO Auto-generated constructor stub
	epsilon = mpreal(0.01);
	for(int i = 0; i < MAXN; i++) {
		line[i] = -1;
	}
	circ.e = QMDDone;
	QMDDincref(circ.e);
	beforeMeasurement = QMDDone;
	QMDDincref(beforeMeasurement);
	circ.n = 0;
}

Simulator::~Simulator() {
	// TODO Auto-generated destructor stub
}

void Simulator::Reset() {
	QMDDdecref(circ.e);
	QMDDgarbageCollect();
	cleanCtable(std::vector<QMDDedge>());
	nqubits = 0;
	circ.e = QMDDone;
	QMDDincref(circ.e);
	beforeMeasurement = QMDDone;
	QMDDincref(beforeMeasurement);
	circ.n = 0;
	max_active = 0;
	complex_limit = 10000;
	gatecount = 0;
	max_gates = 0x7FFFFFFF;
	intermediate_measurement = false;
	measurement_done = false;
}

void Simulator::AddVariables(int add, std::string name) {
	QMDDedge f = QMDDone;
	QMDDedge edges[4];
	edges[1]=edges[2]=edges[3]=QMDDzero;

	for(int p=0;p<add;p++) {
		edges[0] = f;
		f = QMDDmakeNonterminal(p, edges);
	}
	if(circ.e.p != QMDDzero.p) {
		f = AddVariablesRec(circ.e, f, add);
		dag_edges.clear();
	}
	QMDDincref(f);
	QMDDdecref(circ.e);
	circ.e = f;

	if(nqubits != 0) {
		for(int i = nqubits-1; i >= 0; i--) {
			strncpy(circ.line[i+add].variable, circ.line[i].variable, MAXSTRLEN);
		}
	}
	for(int i = 0; i < add; i++) {
		snprintf(circ.line[nqubits + add - 1 - i].variable, MAXSTRLEN , "%s[%d]",name.c_str(), i);
	}

	nqubits += add;
	circ.n = nqubits;
	if(!measurement_done) {
		QMDDdecref(beforeMeasurement);
		beforeMeasurement = circ.e;
		QMDDincref(beforeMeasurement);
	}
}

QMDDedge Simulator::AddVariablesRec(QMDDedge e, QMDDedge t, int add) {

	if(e.p == QMDDtnode) {
		if(e.w == 0) {
			return QMDDzero;
		}
		t.w = Cmul(e.w, t.w);
		return t;
	}

	std::map<QMDDnodeptr, QMDDedge>::iterator it = dag_edges.find(e.p);
	if(it != dag_edges.end()) {
		QMDDedge e2 = it->second;
		e2.w = Cmul(e.w, e2.w);
		return e2;
	}

	QMDDedge edges[MAXRADIX*MAXRADIX];

	for(int i=0; i<MAXRADIX*MAXRADIX; i++) {
		edges[i] = AddVariablesRec(e.p->e[i], t, add);
	}

	QMDDedge e2 = QMDDmakeNonterminal(e.p->v+add, edges);
	dag_edges[e.p] = e2;
	e2.w = Cmul(e.w, e2.w);
	return e2;
}

mpreal Simulator::AssignProbs(QMDDedge& e) {
	std::unordered_map<uint64_t, mpreal>::iterator it2;
	std::unordered_map<QMDDnodeptr, mpreal>::iterator it = probs.find(e.p);
	if(it != probs.end()) {
		it2 = Cmag.find(e.w & 0x7FFFFFFF7FFFFFFFull);
		return it2->second * it2->second * it->second;
	}
	mpreal sum;
	if(QMDDterminal(e)) {
		sum = mpreal(1);
	} else {
		sum = AssignProbs(e.p->e[0]) + AssignProbs(e.p->e[2]); //+ AssignProbs(e.p->e[1]) + AssignProbs(e.p->e[3]);
	}

	probs.insert(std::pair<QMDDnodeptr, mpreal>(e.p, sum));

	it2 = Cmag.find(e.w & 0x7FFFFFFF7FFFFFFFull);

	return it2->second * it2->second * sum;
}

void Simulator::MeasureAll(bool reset_state) {
	std::unordered_map<QMDDnodeptr, mpreal>::iterator it;
	std::unordered_map<uint64_t, mpreal>::iterator it2;

	probs.clear();

	mpreal p,p0,p1,tmp,w;
	p = AssignProbs(circ.e);

	if(abs(p -1) > epsilon) {
		if(p == 0) {
			std::cerr << "ERROR: numerical instabilities led to a 0-vector! Abort simulation!" << std::endl;
			exit(1);
		}
		std::cerr << "WARNING in measurement: numerical instability occurred during simulation: |alpha|^2 + |beta|^2 = " << p << ", but should be 1!"<< std::endl;
	}

	QMDDedge cur = circ.e;
	for(int i = QMDDinvorder[circ.e.p->v]; i >= 0;--i) {

		it = probs.find(cur.p->e[0].p);
		it2 = Cmag.find(cur.p->e[0].w & 0x7FFFFFFF7FFFFFFFull);
		p0 = it->second * it2->second * it2->second;
		it2 = Cmag.find(cur.p->e[1].w & 0x7FFFFFFF7FFFFFFFull);
		it = probs.find(cur.p->e[1].p);
		p0 += it->second * it2->second * it2->second;

		it = probs.find(cur.p->e[2].p);
		it2 = Cmag.find(cur.p->e[2].w & 0x7FFFFFFF7FFFFFFFull);
		p1 = it->second * it2->second * it2->second;
		it = probs.find(cur.p->e[3].p);
		it2 = Cmag.find(cur.p->e[3].w & 0x7FFFFFFF7FFFFFFFull);
		p1 += it->second * it2->second * it2->second;

		it2 = Cmag.find(cur.w & 0x7FFFFFFF7FFFFFFFull);

		mpreal tmp = p0 + p1;
		p0 /= tmp;
		p1 /= tmp;

		mpreal n = mpreal(rand()) / RAND_MAX;

		if(n < p0) {
			measurements[cur.p->v] = 0;
			cur = cur.p->e[0];
		} else {
			measurements[cur.p->v] = 1;
			cur = cur.p->e[2];
		}
	}

	if(reset_state) {
		QMDDdecref(circ.e);

		QMDDedge e = QMDDone;
		QMDDedge edges[4];
		edges[1]=edges[3]=QMDDzero;

		for(int p=0;p<circ.n;p++) {
			if(measurements[p] == 0) {
				edges[0] = e;
				edges[2] = QMDDzero;
			} else {
				edges[0] = QMDDzero;
				edges[2] = e;
			}
			e = QMDDmakeNonterminal(p, edges);
		}
		QMDDincref(e);
		circ.e = e;
		QMDDgarbageCollect();
		cleanCtable(std::vector<QMDDedge>());
	}
	probs.clear();

	measurement_done = true;
}

int Simulator::MeasureOne(int index) {

	std::pair<mpreal, mpreal> probs = AssignProbsOne(circ.e, index);

	QMDDedge e = circ.e;

#if VERBOSE
	std::cout << "  -- measure qubit " << circ.line[index].variable << ": " << std::flush;
#endif

	mpreal sum = probs.first + probs.second;
	mpreal norm_factor;

	if(abs(sum - 1) > epsilon) {
		if(sum == 0) {
			std::cerr << "ERROR: numerical instabilities led to a 0-vector! Abort simulation!" << std::endl;
			exit(1);
		}
		std::cerr << "WARNING in measurement: numerical instability occurred during simulation: |alpha|^2 + |beta|^2 = " << sum << ", but should be 1!"<< std::endl;
	}

#if VERBOSE
	std::cout << "p0 = " << probs.first << ", p1 = " << probs.second << std::flush;
#endif

	mpreal n = (mpreal(rand()) / RAND_MAX);

	line[index] = 2;

	QMDD_matrix measure_m;
	measure_m[0][0] = measure_m[0][1] = measure_m[1][0] = measure_m[1][1] = COMPLEX_ZERO;

	int measurement;

	if(n < probs.first/sum) {
#if VERBOSE
		std::cout << " -> measure 0" << std::endl;
#endif
		measure_m[0][0] = COMPLEX_ONE;
		measurement = 0;
		norm_factor = probs.first;
	} else {
#if VERBOSE
		std::cout << " -> measure 1" << std::endl;
#endif
		measure_m[1][1] = COMPLEX_ONE;
		measurement = 1;
		norm_factor = probs.second;
	}

	QMDDedge f = QMDDmvlgate(measure_m, circ.n, line);

	line[index] = -1;


	e = QMDDmultiply(f,e);
	QMDDdecref(circ.e);
	QMDDincref(e);
	circ.e = e;
	circ.e.w = e.w = Cmul(e.w, Cmake(sqrt(mpreal(1)/mpreal(norm_factor)), mpreal(0)));

	measurement_done = true;
	return measurement;
}

void Simulator::ResetQubit(int index) {
	std::pair<mpreal, mpreal> probs = AssignProbsOne(circ.e, index);

	QMDDedge e = circ.e;

#if VERBOSE
	std::cout << "  -- reset qubit " << circ.line[index].variable << ": " << std::flush;
#endif

	mpreal sum = probs.first + probs.second;
	mpreal norm_factor;

#if VERBOSE
	std::cout << "p0 = " << probs.first << ", p1 = " << probs.second << std::flush;
#endif


	if(abs(sum - 1) > epsilon) {
		std::cout << "Numerical error occurred during simulation: |alpha0|^2 + |alpha1|^2 = " << sum << ", but should be 1 before reset!"<< std::endl;
		exit(1);
	}

	line[index] = 2;

	if(probs.first == 0) {
		QMDDedge f = QMDDmvlgate(Nm, circ.n, line);
		e = QMDDmultiply(f,e);
		QMDDdecref(circ.e);
		QMDDincref(e);
		circ.e = e;
		probs.first = mpreal(1);
	}


	QMDD_matrix measure_m;
	measure_m[0][1] = measure_m[1][0] = measure_m[1][1] = COMPLEX_ZERO;
	measure_m[0][0] = COMPLEX_ONE;
	norm_factor = probs.first;

	QMDDedge f = QMDDmvlgate(measure_m, circ.n, line);

	line[index] = -1;

	e = QMDDmultiply(f,e);
	QMDDdecref(circ.e);
	QMDDincref(e);
	circ.e = e;
	circ.e.w = e.w = Cmul(e.w, Cmake(sqrt(mpreal(1)/mpreal(probs.first)), mpreal(0)));
}

std::pair<mpreal, mpreal> Simulator::AssignProbsOne(QMDDedge e, int index) {
	probs.clear();
	AssignProbs(e);
	std::queue<QMDDnodeptr> q;
	mpreal pzero, pone;
	pzero = pone = mpreal(0);
	mpreal prob;

	probsMone.clear();
	visited_nodes2.clear();

	visited_nodes2.insert(e.p);
	auto it2 = Cmag.find(e.w & 0x7FFFFFFF7FFFFFFFull);
	probsMone[e.p] = it2->second * it2->second;
	mpreal tmp1;
	q.push(e.p);

	while(q.front()->v != index) {
		QMDDnodeptr ptr = q.front();
		q.pop();
		mpreal prob = probsMone[ptr];

		if(ptr->e[0].w != COMPLEX_ZERO) {
			auto it2 = Cmag.find(ptr->e[0].w & 0x7FFFFFFF7FFFFFFFull);
			tmp1 = prob * it2->second * it2->second;

			if(visited_nodes2.find(ptr->e[0].p) != visited_nodes2.end()) {
				probsMone[ptr->e[0].p] = probsMone[ptr->e[0].p] + tmp1;
			} else {
				probsMone[ptr->e[0].p] = tmp1;
				visited_nodes2.insert(ptr->e[0].p);
				q.push(ptr->e[0].p);
			}
		}

		if(ptr->e[2].w != COMPLEX_ZERO) {
			auto it2 = Cmag.find(ptr->e[2].w & 0x7FFFFFFF7FFFFFFFull);
			tmp1 = prob * it2->second * it2->second;

			if(visited_nodes2.find(ptr->e[2].p) != visited_nodes2.end()) {
				probsMone[ptr->e[2].p] = probsMone[ptr->e[2].p] + tmp1;
			} else {
				probsMone[ptr->e[2].p] = tmp1;
				visited_nodes2.insert(ptr->e[2].p);
				q.push(ptr->e[2].p);
			}
		}
	}

	while(q.size() != 0) {
		QMDDnodeptr ptr = q.front();
		q.pop();

		if(ptr->e[0].w != COMPLEX_ZERO) {
			auto it2 = Cmag.find(ptr->e[0].w & 0x7FFFFFFF7FFFFFFFull);
			tmp1 = probsMone[ptr] * probs[ptr->e[0].p] * it2->second * it2->second;
			pzero = pzero + tmp1;
		}

		if(ptr->e[2].w != COMPLEX_ZERO) {
			auto it2 = Cmag.find(ptr->e[2].w & 0x7FFFFFFF7FFFFFFFull);
			tmp1 = probsMone[ptr] * probs[ptr->e[2].p] * it2->second * it2->second;
			pone = pone + tmp1;
		}
	}


	probs.clear();
	probsMone.clear();
	visited_nodes2.clear();

	return std::make_pair(pzero, pone);
}

uint64_t Simulator::GetElementOfVector(unsigned long long element) {
	QMDDedge e = circ.e;
	if(QMDDterminal(e)) {
		return 0;
	}
	uint64_t l = COMPLEX_ONE;
	do {
		l = Cmul(l, e.w);
		//cout << "variable q" << QMDDinvorder[e.p->v] << endl;
		unsigned long long tmp = (element >> QMDDinvorder[e.p->v]) & 1;
		e = e.p->e[2*tmp];
		//element = element % (int)pow(MAXRADIX, QMDDinvorder[e.p->v]+1);
	} while(!QMDDterminal(e));
	l = Cmul(l, e.w);

	return l;
}

mpreal Simulator::GetProbabilityRec(QMDDedge& e) {

	std::unordered_map<uint64_t, mpreal>::iterator it2;
	std::unordered_map<QMDDnodeptr, mpreal>::iterator it = probs.find(e.p);
	if(it != probs.end()) {
		it2 = Cmag.find(e.w & 0x7FFFFFFF7FFFFFFFull);
		return it2->second * it2->second * it->second;
	}
	mpreal sum;
	if(QMDDterminal(e)) {
		sum = mpreal(1);
	} else if(line[e.p->v] == 0) {
		sum = GetProbabilityRec(e.p->e[0]);
	} else if(line[e.p->v] == 1) {
		sum = GetProbabilityRec(e.p->e[2]);
	} else {
		sum = GetProbabilityRec(e.p->e[0]) + GetProbabilityRec(e.p->e[2]); //+ AssignProbs(e.p->e[1]) + AssignProbs(e.p->e[3]);
	}

	probs.insert(std::pair<QMDDnodeptr, mpreal>(e.p, sum));

	it2 = Cmag.find(e.w & 0x7FFFFFFF7FFFFFFFull);

	return it2->second * it2->second * sum;
}


mpreal Simulator::GetProbability() {
	mpreal result = GetProbabilityRec(circ.e);
	probs.clear();
	return result;
}

void Simulator::ApplyGate(QMDDedge gate) {
	gatecount++;

	QMDDedge tmp;

	tmp = QMDDmultiply(gate, circ.e);
	QMDDincref(tmp);
	QMDDdecref(circ.e);
	circ.e = tmp;

	if(!measurement_done) {
		QMDDdecref(beforeMeasurement);
		beforeMeasurement = circ.e;
		QMDDincref(beforeMeasurement);
	}

	QMDDgarbageCollect();

	if(ActiveNodeCount > max_active) {
		max_active = ActiveNodeCount;
	}

	if(Ctable.size() > complex_limit) {
		std::vector<QMDDedge> v;
		v.push_back(circ.e);
		v.push_back(beforeMeasurement);

		cleanCtable(v);
		v.clear();
		if(complex_limit < 2*Ctable.size()) {
			complex_limit *= 2;
		}
	}

	if(measurement_done) {
		intermediate_measurement = true;
	}
}

void Simulator::ApplyGate(QMDD_matrix& m) {
	QMDDedge f = QMDDmvlgate(m, circ.n, line);
	ApplyGate(f);
}

void Simulator::ResetBeforeMeasurement() {
	QMDDdecref(circ.e);
	circ.e = beforeMeasurement;
	QMDDincref(circ.e);
}
