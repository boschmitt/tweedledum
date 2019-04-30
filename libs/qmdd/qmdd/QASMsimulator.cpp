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

#include <QASMsimulator.h>

QASMsimulator::QASMsimulator(std::string filename, bool display_statevector, bool display_probabilities) {
	in = new std::ifstream (filename, std::ifstream::in);
	this->scanner = new QASMscanner(*this->in);
	this->fname = filename;
	this->display_probabilities = display_probabilities;
	this->display_statevector = display_statevector;
}

QASMsimulator::QASMsimulator(bool display_statevector, bool display_probabilities) {
	std::stringstream* in = new std::stringstream();
	(*in) << std::cin.rdbuf();
	this->in = in;
	this->scanner = new QASMscanner(*this->in);
	this->fname = "";
	this->display_probabilities = display_probabilities;
	this->display_statevector = display_statevector;
}

QASMsimulator::~QASMsimulator() {
	delete scanner;
	delete in;

	for(auto it = compoundGates.begin(); it != compoundGates.end(); it++) {
		for(auto it2 = it->second.gates.begin(); it2 != it->second.gates.end(); it2++) {
			delete *it2;
		}
	}

	for(auto it = snapshots.begin(); it != snapshots.end(); it++) {
		delete it->second;
	}
}

void QASMsimulator::scan() {
	t = la;
	la = scanner->next();
	sym = la.kind;
}

void QASMsimulator::check(Token::Kind expected) {
	if (sym == expected) {
		scan();
	} else {
		std::cerr << "ERROR while parsing QASM file: expected '" << Token::KindNames[expected] << "' but found '" << Token::KindNames[sym] << "' in line " << la.line << ", column " << la.col << std::endl;
	}
}

std::pair<int, int> QASMsimulator::QASMargumentQreg() {
	check(Token::Kind::identifier);
	std::string s = t.str;
	if(qregs.find(s) == qregs.end()) {
		std::cerr << "Argument is not a qreg: " << s << std::endl;
	}

	if(sym == Token::Kind::lbrack) {
		scan();
		check(Token::Kind::nninteger);
		int offset = t.val;
		check(Token::Kind::rbrack);
		return std::make_pair(qregs[s].first+offset, 1);
	}
	return std::make_pair(qregs[s].first, qregs[s].second);
}

std::pair<std::string, int> QASMsimulator::QASMargumentCreg() {
	check(Token::Kind::identifier);
	std::string s = t.str;
	if(cregs.find(s) == cregs.end()) {
		std::cerr << "Argument is not a creg: " << s << std::endl;
	}

	int index = -1;
	if(sym == Token::Kind::lbrack) {
		scan();
		check(Token::Kind::nninteger);
		index = t.val;
		if(index < 0 || index >= cregs[s].first) {
			std::cerr << "Index of creg " << s << " is out of bounds: " << index << std::endl;
		}
		check(Token::Kind::rbrack);
	}

	return std::make_pair(s, index);
}


QASMsimulator::Expr* QASMsimulator::QASMexponentiation() {
	Expr* x;

	if(sym == Token::Kind::real) {
		scan();
		return new Expr(Expr::Kind::number, NULL, NULL, t.valReal, "");
		//return mpreal(t.val_real);
	} else if(sym == Token::Kind::nninteger) {
		scan();
		return new Expr(Expr::Kind::number, NULL, NULL, t.val, "");
		//return mpreal(t.val);
	} else if(sym == Token::Kind::pi) {
		scan();
		return new Expr(Expr::Kind::number, NULL, NULL, mpfr::const_pi(), "");
		//return mpfr::const_pi();
	} else if(sym == Token::Kind::identifier) {
		scan();
		return new Expr(Expr::Kind::id, NULL, NULL, 0, t.str);
		//return it->second;
	} else if(sym == Token::Kind::lpar) {
		scan();
		x = QASMexp();
		check(Token::Kind::rpar);
		return x;
	} else if(unaryops.find(sym) != unaryops.end()) {
		Token::Kind op = sym;
		scan();
		check(Token::Kind::lpar);
		x = QASMexp();
		check(Token::Kind::rpar);
		if(x->kind == Expr::Kind::number) {
			if(op == Token::Kind::sin) {
				x->num = sin(x->num);
			} else if(op == Token::Kind::cos) {
				x->num = cos(x->num);
			} else if(op == Token::Kind::tan) {
				x->num = tan(x->num);
			} else if(op == Token::Kind::exp) {
				x->num = exp(x->num);
			} else if(op == Token::Kind::ln) {
				x->num = log(x->num);
			} else if(op == Token::Kind::sqrt) {
				x->num = sqrt(x->num);
			}
			return x;
		} else {
			if(op == Token::Kind::sin) {
				return new Expr(Expr::Kind::sin, x, NULL, 0, "");
			} else if(op == Token::Kind::cos) {
				return new Expr(Expr::Kind::cos, x, NULL, 0, "");
			} else if(op == Token::Kind::tan) {
				return new Expr(Expr::Kind::tan, x, NULL, 0, "");
			} else if(op == Token::Kind::exp) {
				return new Expr(Expr::Kind::exp, x, NULL, 0, "");
			} else if(op == Token::Kind::ln) {
				return new Expr(Expr::Kind::ln, x, NULL, 0, "");
			} else if(op == Token::Kind::sqrt) {
				return new Expr(Expr::Kind::sqrt, x, NULL, 0, "");
			}
		}
	} else {
		std::cerr << "Invalid Expression" << std::endl;
	}
	return NULL;
}

QASMsimulator::Expr* QASMsimulator::QASMfactor() {
	Expr* x;
	Expr* y;
	x = QASMexponentiation();
	while (sym == Token::Kind::power) {
		scan();
		y = QASMexponentiation();
		if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
			x->num = pow(x->num, y->num);
			delete y;
		} else {
			x = new Expr(Expr::Kind::power, x, y, 0, "");
		}
	}

	return x;
}

QASMsimulator::Expr* QASMsimulator::QASMterm() {
	QASMsimulator::Expr* x = QASMfactor();
	QASMsimulator::Expr* y;

	while(sym == Token::Kind::times || sym == Token::Kind::div) {
		Token::Kind op = sym;
		scan();
		y = QASMfactor();
		if(op == Token::Kind::times) {
			if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
				x->num = x->num * y->num;
				delete y;
			} else {
				x = new Expr(Expr::Kind::times, x, y, 0, "");
			}
		} else {
			if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
				x->num = x->num / y->num;
				delete y;
			} else {
				x = new Expr(Expr::Kind::div, x, y, 0, "");
			}
		}
	}
	return x;
}

QASMsimulator::Expr* QASMsimulator::QASMexp() {
	Expr* x;
	Expr* y;
	if(sym == Token::Kind::minus) {
		scan();
		x = QASMterm();
		if(x->kind == Expr::Kind::number) {
			x->num = -x->num;
		} else {
			x = new Expr(Expr::Kind::sign, x, NULL, 0, "");
		}
	} else {
		x = QASMterm();
	}

	while(sym == Token::Kind::plus || sym == Token::Kind::minus) {
		Token::Kind op = sym;
		scan();
		y = QASMterm();
		if(op == Token::Kind::plus) {
			if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
				x->num = x->num + y->num;
				delete y;
			} else {
				x = new Expr(Expr::Kind::plus, x, y, 0, "");
			}
		} else {
			if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
				x->num = x->num - y->num;
				delete y;
			} else {
				x = new Expr(Expr::Kind::minus, x, y, 0, "");
			}
		}
	}
	return x;
}

void QASMsimulator::QASMexpList(std::vector<Expr*>& expressions) {
	Expr* x = QASMexp();
	expressions.push_back(x);
	while(sym == Token::Kind::comma) {
		scan();
		expressions.push_back(QASMexp());
	}
}

void QASMsimulator::QASMargsList(std::vector<std::pair<int, int> >& arguments) {
	arguments.push_back(QASMargumentQreg());
	while(sym == Token::Kind::comma) {
		scan();
		arguments.push_back(QASMargumentQreg());
	}
}

void QASMsimulator::QASMgate(bool execute) {
	if(sym == Token::Kind::ugate) {
		scan();
		check(Token::Kind::lpar);
		Expr* theta = QASMexp();
		check(Token::Kind::comma);
		Expr* phi = QASMexp();
		check(Token::Kind::comma);
		Expr* lambda = QASMexp();
		check(Token::Kind::rpar);
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);

		if(execute) {
			for(int i = 0; i < target.second; i++) {
				tmp_matrix[0][0] = Cmake(cos(-(phi->num+lambda->num)/2)*cos(theta->num/2), sin(-(phi->num+lambda->num)/2)*cos(theta->num/2));
				tmp_matrix[0][1] = Cmake(-cos(-(phi->num-lambda->num)/2)*sin(theta->num/2), -sin(-(phi->num-lambda->num)/2)*sin(theta->num/2));
				tmp_matrix[1][0] = Cmake(cos((phi->num-lambda->num)/2)*sin(theta->num/2), sin((phi->num-lambda->num)/2)*sin(theta->num/2));
				tmp_matrix[1][1] = Cmake(cos((phi->num+lambda->num)/2)*cos(theta->num/2), sin((phi->num+lambda->num)/2)*cos(theta->num/2));

				line[nqubits-1-(target.first+i)] = 2;

				QMDDedge f = QMDDmvlgate(tmp_matrix, nqubits, line);
				line[nqubits-1-(target.first+i)] = -1;

				ApplyGate(f);
			}
		}
		delete theta;
		delete phi;
		delete lambda;

#if VERBOSE
		std::cout << "Applied gate: U" << std::endl;
#endif

	} else if(sym == Token::Kind::cxgate) {
		scan();
		std::pair<int, int> control = QASMargumentQreg();
		check(Token::Kind::comma);
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);

		if(execute) {
			if(control.second == target.second) {
				for(int i = 0; i < target.second; i++) {
					line[nqubits-1-(control.first+i)] = 1;
					line[nqubits-1-(target.first+i)] = 2;
					QMDDedge f = QMDDmvlgate(Nm, nqubits, line);
					line[nqubits-1-(control.first+i)] = -1;
					line[nqubits-1-(target.first+i)] = -1;
					ApplyGate(f);
				}
			} else if(control.second == 1) {
				for(int i = 0; i < target.second; i++) {
					line[nqubits-1-control.first] = 1;
					line[nqubits-1-(target.first+i)] = 2;
					QMDDedge f = QMDDmvlgate(Nm, nqubits, line);
					line[nqubits-1-control.first] = -1;
					line[nqubits-1-(target.first+i)] = -1;
					ApplyGate(f);
				}
			} else if(target.second == 1) {
				for(int i = 0; i < target.second; i++) {
					line[nqubits-1-(control.first+i)] = 1;
					line[nqubits-1-target.first] = 2;
					QMDDedge f = QMDDmvlgate(Nm, nqubits, line);
					line[nqubits-1-(control.first+i)] = -1;
					line[nqubits-1-target.first] = -1;
					ApplyGate(f);
				}
			} else {
				std::cerr << "Register size does not match for CX gate!" << std::endl;
			}
#if VERBOSE
		std::cout << "Applied gate: CX" << std::endl;
#endif
		}
	} else if(sym == Token::Kind::identifier) {
		scan();
		auto gateIt = compoundGates.find(t.str);
		if(gateIt != compoundGates.end()) {
			std::string gate_name = t.str;

			std::vector<Expr*> parameters;
			std::vector<std::pair<int, int> > arguments;
			if(sym == Token::Kind::lpar) {
				scan();
				if(sym != Token::Kind::rpar) {
					QASMexpList(parameters);
				}
				check(Token::Kind::rpar);
			}
			QASMargsList(arguments);
			check(Token::Kind::semicolon);

			if(execute) {
				std::map<std::string, std::pair<int, int> > argsMap;
				std::map<std::string, Expr*> paramsMap;
				int size = 1;
				int i = 0;
				for(auto it = arguments.begin(); it != arguments.end(); it++) {
					argsMap[gateIt->second.argumentNames[i]] = *it;
					i++;
					if(it->second > 1 && size != 1 && it->second != size) {
						std::cerr << "Register sizes do not match!" << std::endl;
					}
					if(it->second > 1) {
						size = it->second;
					}
				}
				for(unsigned int i = 0; i < parameters.size(); i++) {
					paramsMap[gateIt->second.parameterNames[i]] = parameters[i];
				}

				for(auto it = gateIt->second.gates.begin(); it != gateIt->second.gates.end(); it++) {
					if(Ugate* u = dynamic_cast<Ugate*>(*it)) {
						Expr* theta = RewriteExpr(u->theta, paramsMap);
						Expr* phi = RewriteExpr(u->phi, paramsMap);
						Expr* lambda = RewriteExpr(u->lambda, paramsMap);

						for(int i = 0; i < argsMap[u->target].second; i++) {
							tmp_matrix[0][0] = Cmake(cos(-(phi->num+lambda->num)/2)*cos(theta->num/2), sin(-(phi->num+lambda->num)/2)*cos(theta->num/2));
							tmp_matrix[0][1] = Cmake(-cos(-(phi->num-lambda->num)/2)*sin(theta->num/2), -sin(-(phi->num-lambda->num)/2)*sin(theta->num/2));
							tmp_matrix[1][0] = Cmake(cos((phi->num-lambda->num)/2)*sin(theta->num/2), sin((phi->num-lambda->num)/2)*sin(theta->num/2));
							tmp_matrix[1][1] = Cmake(cos((phi->num+lambda->num)/2)*cos(theta->num/2), sin((phi->num+lambda->num)/2)*cos(theta->num/2));

							line[nqubits-1-(argsMap[u->target].first+i)] = 2;
							QMDDedge f = QMDDmvlgate(tmp_matrix, nqubits, line);
							line[nqubits-1-(argsMap[u->target].first+i)] = -1;

							ApplyGate(f);
						}
						delete theta;
						delete phi;
						delete lambda;
					} else if(CXgate* cx = dynamic_cast<CXgate*>(*it)) {
						if(argsMap[cx->control].second == argsMap[cx->target].second) {
							for(int i = 0; i < argsMap[cx->target].second; i++) {
								line[nqubits-1-(argsMap[cx->control].first+i)] = 1;
								line[nqubits-1-(argsMap[cx->target].first+i)] = 2;
								QMDDedge f = QMDDmvlgate(Nm, nqubits, line);
								line[nqubits-1-(argsMap[cx->control].first+i)] = -1;
								line[nqubits-1-(argsMap[cx->target].first+i)] = -1;
								ApplyGate(f);
							}
						} else if(argsMap[cx->control].second == 1) {
							for(int i = 0; i < argsMap[cx->target].second; i++) {
								line[nqubits-1-argsMap[cx->control].first] = 1;
								line[nqubits-1-(argsMap[cx->target].first+i)] = 2;
								QMDDedge f = QMDDmvlgate(Nm, nqubits, line);
								line[nqubits-1-argsMap[cx->control].first] = -1;
								line[nqubits-1-(argsMap[cx->target].first+i)] = -1;
								ApplyGate(f);
							}
						} else if(argsMap[cx->target].second == 1) {
							for(int i = 0; i < argsMap[cx->target].second; i++) {
								line[nqubits-1-(argsMap[cx->control].first+i)] = 1;
								line[nqubits-1-argsMap[cx->target].first] = 2;
								QMDDedge f = QMDDmvlgate(Nm, nqubits, line);
								line[nqubits-1-(argsMap[cx->control].first+i)] = -1;
								line[nqubits-1-argsMap[cx->target].first] = -1;
								ApplyGate(f);
							}
						} else {
							std::cerr << "Register size does not match for CX gate!" << std::endl;
						}
					}
				}

#if VERBOSE
		std::cout << "Applied gate: " << gate_name << std::endl;
#endif
			}
		} else {
			std::cerr << "Undefined gate: " << t.str << std::endl;
		}
	}
}

void QASMsimulator::Reset() {
	Simulator::Reset();
	qregs.clear();

	for(auto it = cregs.begin(); it != cregs.end(); it++) {
		delete it->second.second;
	}
	cregs.clear();
	delete scanner;
	in->clear();
	in->seekg(0, in->beg);
	this->scanner = new QASMscanner(*this->in);

	for(auto it = snapshots.begin(); it != snapshots.end(); it++) {
		delete it->second;
	}
	snapshots.clear();
}

void QASMsimulator::Simulate(int shots) {
	if(shots < 1) {
		std::cerr << "Shots have to be greater than 0!" << std::endl;
	}

	std::map<std::string, int> result;

	Simulate();
	if(!intermediate_measurement) {
		ResetBeforeMeasurement();
		for(int i = 0; i < shots; i++) {
			MeasureAll(false);
			std::stringstream s;
			for(int i=circ.n-1;i >=0; i--) {
				s << measurements[i];
			}
			if(result.find(s.str()) != result.end()) {
				result[s.str()]++;
			} else {
				result[s.str()] = 1;
			}
		}
	} else {
		MeasureAll(false);
		std::stringstream s;
		for(int i=circ.n-1;i >=0; i--) {
			s << measurements[i];
		}
		result[s.str()] = 1;
		for(int i = 1; i < shots; i++) {
			Reset();
			Simulate();
			MeasureAll(false);
			std::stringstream s;
			for(int j=circ.n-1;j >=0; j--) {
				s << measurements[j];
			}
			if(result.find(s.str()) != result.end()) {
				result[s.str()]++;
			} else {
				result[s.str()] = 1;
			}
		}
	}

	std::cout << "{" << std::endl << "  \"counts\": {" << std::endl;
	auto it = result.begin();
	std::cout << "    \"" << it->first << "\": " << it->second;
	for(it++; it != result.end(); it++) {
		std::cout << ",\n    \"" << it->first << "\": " << it->second;
	}
	std::cout << "  }";

	if(snapshots.size() > 0) {
		std::cout << "," << std::endl;
		std::cout << "  \"snapshots\": {" << std::endl;
		for(auto it = snapshots.begin(); it != snapshots.end(); it++) {
			std::cout << "    \"" << it->first << "\": {" << std::endl;
			if(display_probabilities) {
				std::cout << "      \"probabilities\": [" << it->second->probabilities[0];
				for(unsigned long long i = 1; i < it->second->len; i++) {
					std::cout << ", " << it->second->probabilities[i];
				}
				std::cout << "]," << std::endl;
				std::cout << "      \"probabilities_ket\": {";
				auto it2 = it->second->probabilities_ket.begin();
				std::cout << "\"" << it2->first << "\": " << it2->second;
				it2++;
				for(; it2 != it->second->probabilities_ket.end(); it2++) {
					std::cout << ", \"" << it2->first << "\": " << it2->second;
				}
				std::cout << "}";
			}
			if(display_statevector && it->second->statevector != NULL) {
				if(display_probabilities) {
					std::cout << "," << std::endl;
				}
				std::cout << "      \"statevector\": [\"" << it->second->statevector[0];
				for(unsigned long long i = 1; i < it->second->len; i++) {
					std::cout << "\", \"" << it->second->statevector[i];
				}
				std::cout << "\"]";
			}
			std::cout << std::endl << "    }" << (std::next(it,1) != snapshots.end() ? "," : "") << std::endl;
		}
		std::cout << "  }" << std::endl;
	}
	std::cout << "}" << std::endl;
}

void QASMsimulator::QASMidList(std::vector<std::string>& identifiers) {
	check(Token::Kind::identifier);
	identifiers.push_back(t.str);
	while(sym == Token::Kind::comma) {
		scan();
		check(Token::Kind::identifier);
		identifiers.push_back(t.str);
	}
}

QASMsimulator::Expr* QASMsimulator::RewriteExpr(Expr* expr, std::map<std::string, Expr*>& exprMap) {
	if(expr == NULL) {
		return NULL;
	}
	Expr* op1 = RewriteExpr(expr->op1, exprMap);
	Expr* op2 = RewriteExpr(expr->op2, exprMap);

	if(expr->kind == Expr::Kind::number) {
		return new Expr(expr->kind, op1, op2, expr->num, expr->id);
	} else if(expr->kind == Expr::Kind::plus) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = op1->num + op2->num;
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::minus) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = op1->num - op2->num;
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::sign) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = -op1->num;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::times) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = op1->num * op2->num;
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::div) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = op1->num / op2->num;
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::power) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = pow(op1->num,op2->num);
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::sin) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = sin(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::cos) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = cos(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::tan) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = tan(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::exp) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = exp(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::ln) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = log(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::sqrt) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = sqrt(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::id) {
		return new Expr(*exprMap[expr->id]);
	}

	return new Expr(expr->kind, op1, op2, expr->num, expr->id);
}

void QASMsimulator::QASMopaqueGateDecl() {
	check(Token::Kind::opaque);
	check(Token::Kind::identifier);

	CompoundGate gate;
	std::string gateName = t.str;
	if(sym == Token::Kind::lpar) {
		scan();
		if(sym != Token::Kind::rpar) {
			QASMidList(gate.parameterNames);
		}
		check(Token::Kind::rpar);
	}
	QASMidList(gate.argumentNames);

	compoundGates[gateName] = gate;

	check(Token::Kind::semicolon);
	//Opaque gate has an empty body
}


void QASMsimulator::QASMgateDecl() {
	check(Token::Kind::gate);
	check(Token::Kind::identifier);

	CompoundGate gate;
	std::string gateName = t.str;
	if(sym == Token::Kind::lpar) {
		scan();
		if(sym != Token::Kind::rpar) {
			QASMidList(gate.parameterNames);
		}
		check(Token::Kind::rpar);
	}
	QASMidList(gate.argumentNames);
	check(Token::Kind::lbrace);


	while(sym != Token::Kind::rbrace) {
		if(sym == Token::Kind::ugate) {
			scan();
			check(Token::Kind::lpar);
			Expr* theta = QASMexp();
			check(Token::Kind::comma);
			Expr* phi = QASMexp();
			check(Token::Kind::comma);
			Expr* lambda = QASMexp();
			check(Token::Kind::rpar);
			check(Token::Kind::identifier);

			gate.gates.push_back(new Ugate(theta, phi, lambda, t.str));
			check(Token::Kind::semicolon);
		} else if(sym == Token::Kind::cxgate) {
			scan();
			check(Token::Kind::identifier);
			std::string control = t.str;
			check(Token::Kind::comma);
			check(Token::Kind::identifier);
			gate.gates.push_back(new CXgate(control, t.str));
			check(Token::Kind::semicolon);

		} else if(sym == Token::Kind::identifier) {
			scan();
			std::string name = t.str;

			std::vector<Expr* > parameters;
			std::vector<std::string> arguments;
			if(sym == Token::Kind::lpar) {
				scan();
				if(sym != Token::Kind::rpar) {
					QASMexpList(parameters);
				}
				check(Token::Kind::rpar);
			}
			QASMidList(arguments);
			check(Token::Kind::semicolon);

			CompoundGate g = compoundGates[name];
			std::map<std::string, std::string> argsMap;
			for(unsigned int i = 0; i < arguments.size(); i++) {
				argsMap[g.argumentNames[i]] = arguments[i];
			}

			std::map<std::string, Expr*> paramsMap;
			for(unsigned int i = 0; i < parameters.size(); i++) {
				paramsMap[g.parameterNames[i]] = parameters[i];
			}

			for(auto it = g.gates.begin(); it != g.gates.end(); it++) {
				if(Ugate* u = dynamic_cast<Ugate*>(*it)) {
					gate.gates.push_back(new Ugate(RewriteExpr(u->theta, paramsMap), RewriteExpr(u->phi, paramsMap), RewriteExpr(u->lambda, paramsMap), argsMap[u->target]));
				} else if(CXgate* cx = dynamic_cast<CXgate*>(*it)) {
					gate.gates.push_back(new CXgate(argsMap[cx->control], argsMap[cx->target]));
				} else {
					std::cerr << "Unexpected gate!" << std::endl;
				}
			}

			for(auto it = parameters.begin(); it != parameters.end(); it++) {
				delete *it;
			}
		} else if(sym == Token::Kind::barrier) {
			scan();
			std::vector<std::string> arguments;
			QASMidList(arguments);
			check(Token::Kind::semicolon);
			//Nothing to do here for the simulator
		} else {
			std::cerr << "Error in gate declaration!" << std::endl;
		}
	}

#if VERBOSE & 0
	std::cout << "Declared gate \"" << gateName << "\":" << std::endl;
	for(auto it = gate.gates.begin(); it != gate.gates.end(); it++) {
		if(Ugate* u = dynamic_cast<Ugate*>(*it)) {
			std::cout << "  U(";
			printExpr(u->theta);
			std::cout << ", ";
			printExpr(u->phi);
			std::cout << ", ";
			printExpr(u->lambda);
			std::cout << ") "<< u->target << ";" << std::endl;
		} else if(CXgate* cx = dynamic_cast<CXgate*>(*it)) {
			std::cout << "  CX " << cx->control << ", " << cx->target << ";" << std::endl;
		} else {
			std::cout << "other gate" << std::endl;
		}
	}
#endif

	compoundGates[gateName] = gate;

	check(Token::Kind::rbrace);
}

void QASMsimulator::printExpr(Expr* expr) {
	if(expr->kind == Expr::Kind::number) {
		std::cout << expr->num;
	} else if(expr->kind == Expr::Kind::plus) {
		printExpr(expr->op1);
		std::cout << " + ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::minus) {
		printExpr(expr->op1);
		std::cout << " - ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::sign) {
		std::cout << "( - ";
		printExpr(expr->op1);
		std::cout << " )";
	} else if(expr->kind == Expr::Kind::times) {
		printExpr(expr->op1);
		std::cout << " * ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::div) {
		printExpr(expr->op1);
		std::cout << " / ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::power) {
		printExpr(expr->op1);
		std::cout << " ^ ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::sin) {
		std::cout << "sin(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::cos) {
		std::cout << "cos(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::tan) {
		std::cout << "tan(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::exp) {
		std::cout << "exp(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::ln) {
		std::cout << "ln(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::sqrt) {
		std::cout << "sqrt(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::id) {
		std::cout << expr->id;
	}
}

void QASMsimulator::QASMqop(bool execute) {
	if(sym == Token::Kind::ugate || sym == Token::Kind::cxgate || sym == Token::Kind::identifier) {
		QASMgate(execute);
	} else if(sym == Token::Kind::measure) {
		scan();
		std::pair<int, int> qreg = QASMargumentQreg();

		check(Token::Kind::minus);
		check(Token::Kind::gt);
		std::pair<std::string, int> creg = QASMargumentCreg();
		check(Token::Kind::semicolon);

		if(execute) {
			int creg_size = (creg.second == -1) ? cregs[creg.first].first : 1;

			if(qreg.second == creg_size) {
				if(creg_size == 1) {
					cregs[creg.first].second[creg.second] = MeasureOne(nqubits-1-(qreg.first));
				} else {
					for(int i = 0; i < creg_size; i++) {
						cregs[creg.first].second[i] = MeasureOne(nqubits-1-(qreg.first+i));
					}
				}
			} else {
				std::cerr << "Mismatch of qreg and creg size in measurement" << std::endl;
			}
		}
	} else if(sym == Token::Kind::reset) {
		scan();
		std::pair<int, int> qreg = QASMargumentQreg();

		check(Token::Kind::semicolon);

		if(execute) {
			for(int i = 0; i < qreg.second; i++) {
				ResetQubit(nqubits-1-(qreg.first+i));
			}
		}

	}
}

void QASMsimulator::Simulate() {

	scan();
	check(Token::Kind::openqasm);
	check(Token::Kind::real);
	check(Token::Kind::semicolon);

	do {
		if(sym == Token::Kind::qreg) {

			scan();
			check(Token::Kind::identifier);
			std::string s = t.str;
			check(Token::Kind::lbrack);
			check(Token::Kind::nninteger);
			int n = t.val;
			check(Token::Kind::rbrack);
			check(Token::Kind::semicolon);
			//check whether it already exists

			qregs[s] = std::make_pair(nqubits, n);
			AddVariables(n, s);
		} else if(sym == Token::Kind::creg) {
			scan();
			check(Token::Kind::identifier);
			std::string s = t.str;
			check(Token::Kind::lbrack);
			check(Token::Kind::nninteger);
			int n = t.val;
			check(Token::Kind::rbrack);
			check(Token::Kind::semicolon);
			int* reg = new int[n];

			//Initialize cregs with 0
			for(int i=0; i<n; i++) {
				reg[i] = 0;
			}
			cregs[s] = std::make_pair(n, reg);

		} else if(sym == Token::Kind::ugate || sym == Token::Kind::cxgate || sym == Token::Kind::identifier || sym == Token::Kind::measure || sym == Token::Kind::reset) {
			QASMqop();
		} else if(sym == Token::Kind::gate) {
			QASMgateDecl();
		} else if(sym == Token::Kind::include) {
			scan();
			check(Token::Kind::string);
			std::string fname = t.str;
			scanner->addFileInput(fname);
			check(Token::Kind::semicolon);
		} else if(sym == Token::Kind::barrier) {
			scan();
			std::vector<std::pair<int, int> > args;
			QASMargsList(args);
			check(Token::Kind::semicolon);
			//Nothing to do here for simulator
		} else if(sym == Token::Kind::opaque) {
			QASMopaqueGateDecl();
		} else if(sym == Token::Kind::_if) {
			scan();
			check(Token::Kind::lpar);
			check(Token::Kind::identifier);
			std::string creg = t.str;
			check(Token::Kind::eq);
			check(Token::Kind::nninteger);
			int n = t.val;
			check(Token::Kind::rpar);

			auto it = cregs.find(creg);
			if(it == cregs.end()) {
				std::cerr << "Error in if statement: " << creg << " is not a creg!" << std::endl;
			} else {
				int creg_num = 0;
				for(int i = it->second.first-1; i >= 0; i--) {
					creg_num = (creg_num << 1) | (it->second.second[i] & 1);
				}
				QASMqop(creg_num == n);
			}

		} else if(sym == Token::Kind::snapshot) {
			scan();
			check(Token::Kind::lpar);
			check(Token::Kind::nninteger);
			int n = t.val;
			check(Token::Kind::rpar);

			std::vector<std::pair<int, int> > arguments;
			QASMargsList(arguments);

			check(Token::Kind::semicolon);

			for(auto it = arguments.begin(); it != arguments.end(); it++) {
				if(it->second != 1) {
					std::cerr << "ERROR in snapshot: arguments must be qubits" << std::endl;
				}
			}

			//TODO: check whether no argument occurs twice!


			Snapshot* snapshot = new Snapshot();
			if(display_probabilities) {
				snapshot->len = 1ull << (unsigned long long)arguments.size();
				snapshot->probabilities = new double[snapshot->len];
				for(unsigned long long i = 0; i < snapshot->len; i++) {
					int j = arguments.size()-1;
					for(auto it = arguments.begin(); it != arguments.end(); it++) {
						line[nqubits-1-it->first] = (i >> j--) & 1;
					}
					snapshot->probabilities[i] = GetProbability().toDouble();
					if(snapshot->probabilities[i] > 0.0) {
						std::stringstream ss;
						for(int j = arguments.size()-1; j >= 0; j--) {
							ss << ((i >> j) & 1);
						}
						snapshot->probabilities_ket[ss.str()] = snapshot->probabilities[i];
					}
				}
				for(auto it = arguments.begin(); it != arguments.end(); it++) {
					line[nqubits-1-it->first] = -1;
				}
			}
			if(display_statevector) {
				if(arguments.size() != nqubits) {
					std::cerr << "Snapshot must contain all qubits when containing statevector!" << std::endl;
				} else {
					snapshot->len = 1ull << (unsigned long long)arguments.size();
					snapshot->statevector = new std::string[snapshot->len];

					for(unsigned long long i = 0; i < snapshot->len; i++) {
						unsigned long long entry = 0;
						int j = arguments.size()-1;
						for(auto it = arguments.begin(); it != arguments.end(); it++) {
							entry |= ((i >> j--) & 1) << (nqubits-1-it->first);
						}
						uint64_t res = GetElementOfVector(entry);
						std::stringstream ss;
						Cprint(res, ss);
						snapshot->statevector[i] = ss.str();
					}
				}
			}

			snapshots[n] = snapshot;
		} else if(sym == Token::Kind::probabilities) {
			std::cout << "Probabilities of the states |";
			for(int i=nqubits-1; i>=0; i--) {
				std::cout << circ.line[i].variable << " ";
			}
			std::cout << ">:" << std::endl;
			for(int i=0; i<(1<<nqubits);i++) {
				uint64_t res = GetElementOfVector(i);
				std::cout << "  |";
				for(int j=nqubits-1; j >= 0; j--) {
					std::cout << ((i >> j) & 1);
				}
				std::cout << ">: " << Cmag[res & 0x7FFFFFFF7FFFFFFFull]*Cmag[res & 0x7FFFFFFF7FFFFFFFull];
				std::cout << std::endl;
			}
			scan();
			check(Token::Kind::semicolon);
		} else {
            std::cerr << "ERROR: unexpected statement: started with " << Token::KindNames[sym] << "!" << std::endl;
            exit(1);
		}
	} while (sym != Token::Kind::eof);
}
