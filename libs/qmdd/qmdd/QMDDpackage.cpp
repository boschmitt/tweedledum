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

#define DEFINE_VARIABLES	// not only declare, but DEFINE global variables
#include "QMDDpackage.h"
#include "QMDDcomplex.h"
#include <set>

/***********************************************

 Private Routines - Used in package - not called by user program.

 ***********************************************/

void QMDDinitGateMatrices(void)
// initializes basic gate matrices
	{
	uint64_t v, vc;

	// init binary gate matrices

	v = Cmake(Qmake(1, 0, 2), Qmake(1, 0, 2));
	vc = Cmake(Qmake(1, 0, 2), Qmake(-1, 0, 2));
	Nm[0][0] = Nm[1][1] = COMPLEX_ZERO;
	Nm[0][1] = Nm[1][0] = COMPLEX_ONE;
	Vm[0][0] = Vm[1][1] = v;
	Vm[0][1] = Vm[1][0] = vc;
	VPm[0][0] = VPm[1][1] = vc;
	VPm[0][1] = VPm[1][0] = v;
	Hm[0][0] = Hm[0][1] = Hm[1][0] = Cmake(Qmake(0, 1, 2), 0);
	Hm[1][1] = Cmake(Qmake(0, -1, 2), 0);
	Zm[0][0] = COMPLEX_ONE;
	Zm[0][1] = Zm[1][0] = COMPLEX_ZERO;
	Zm[1][1] = COMPLEX_M_ONE;
	Sm[0][0] = COMPLEX_ONE;
	Sm[0][1] = Sm[1][0] = COMPLEX_ZERO;
	Sm[1][1] = Cmake(0, Qmake(1, 0, 1));
	ZEROm[0][0] = ZEROm[1][0] = COMPLEX_ONE;
	ZEROm[0][1] = ZEROm[1][0] = COMPLEX_ZERO;
	Qm[0][0] = COMPLEX_ONE;
	Qm[0][1] = Qm[1][0] = COMPLEX_ZERO;
	// These values might be overwritten. Before overwriting them, Cfree has to be called to avoid memory leaks in case a complex number allocates memory

	Qm[1][1] = COMPLEX_ZERO;
	Rm[0][0] = COMPLEX_ZERO;
	Rm[0][1] = COMPLEX_ZERO;
	Rm[1][0] = COMPLEX_ZERO;
	Rm[1][1] = COMPLEX_ZERO;

}

void QMDDpause(void)
// for debugging purposes - not normally used
		{
	char ch;
	scanf("%c", &ch);
}

void QMDDdebugnode(QMDDnodeptr p)
// for debugging purposes - not normally used
		{
	int i;

	if (p == QMDDzero.p) {
		printf("terminal\n");
		return;
	}
	printf("Debug node %ld\n", (intptr_t) p);
	printf("node v %d (%d) edges (w,p) ", (int) QMDDorder[p->v], (int) p->v);
	for (i = 0; i < Nedge; i++) {
		Cprint(p->e[i].w);
		printf(" %ld || ", (intptr_t) p->e[i].p);
	}
	printf("ref %d\n", p->ref);
	//for(i=0;i<NEDGE;i++) QMDDdebugnode(p->e[i].p);
}

ListElementPtr QMDDnewListElement(void) {
	ListElementPtr r, r2;
	int i, j;

	if (Lavail != NULL)	// get node from avail chain if possible
	{
		r = Lavail;
		Lavail = Lavail->next;
	} else {			// otherwise allocate 2000 new nodes
		j = sizeof(ListElement);
		;
		r = (ListElementPtr) malloc(2000 * j);
		r2 = (ListElementPtr) ((int64_t) r + j);
		Lavail = r2;
		for (i = 0; i < 1998; i++, r2 = (ListElementPtr) ((int64_t) r2 + j)) {
			r2->next = (ListElementPtr) ((int64_t) r2 + j);
		}
		r2->next = NULL;
	}
	return (r);
}

void QMDDprint(QMDDedge e, int limit)
// a slightly better QMDD print utility
		{
	ListElementPtr first, q, lastq, pnext;
	uint64_t n, i, j;

	first = QMDDnewListElement();
	first->p = e.p;
	first->next = NULL;
	first->w = COMPLEX_ZERO;
	first->cnt = 1;
	n = 0;
	i = 0;
	printf("top edge weight ");
	Cprint(e.w);
	printf("\n");

	pnext = first;

	while (pnext != NULL) {
		printf("%3d %3d ", pnext->cnt, pnext->p->ref);

		if (pnext->p->block)
			printf("B");
		else
			printf(" ");
		if (pnext->p->diag)
			printf("D");
		else
			printf(" ");
		if (pnext->p->ident)
			printf("I");
		else
			printf(" ");
		if (pnext->p->symm)
			printf("S");
		else
			printf(" ");
		if (pnext->p->renormFactor != COMPLEX_ONE)
			printf("R=%2d", pnext->p->renormFactor);
		else
			printf("    ");
		printf(" %3d| ", i);
		printf(" (%d)", pnext->p->v);

		// if (pnext->cnt != pnext->p->ref)
		//QMDDpause();

		printf("[");
		if (pnext->p != QMDDzero.p)
			for (j = 0; j < Nedge; j++) {
				if (pnext->p->e[j].p == NULL)
					printf("NULL ");
				else {
					if (!QMDDterminal(pnext->p->e[j])) {
						q = first->next;
						lastq = first;
						while (q != NULL && pnext->p->e[j].p != q->p) {
							lastq = q;
							q = q->next;
						}
						if (q == NULL) {
							q = QMDDnewListElement();
							;
							q->p = pnext->p->e[j].p;
							q->next = NULL;
							q->w = n = n + 1;
							q->cnt = 1;
							lastq->next = q;
						} else
							q->cnt = q->cnt + 1;
						printf(" %3d:", q->w);
					} else
						printf("   T:");
					printf(" (%2d)", pnext->p->e[j].w);
					printf(" ");
				}
			}

		printf("] %ld\n", (intptr_t) pnext->p);
		//printf("] \n");
		i++;
		if (i == limit) {
			printf("Printing terminated at %d vertices\n", limit);
			return;
		}
		pnext = pnext->next;
	}
}

/*
 * QMDD2dot export. Nodes representing special matrices (symmetric/identity) are coloured green/red.
 * (probably coded by Mathias Soeken, msoeken@informatik.uni-bremen.de)
 */
void QMDD2dot(QMDDedge e, int limit, std::ostream& oss,
		QMDDrevlibDescription circ) {
	/* first part of dot file*/
	std::ostringstream nodes;
	/*second part of dot file*/
	std::ostringstream edges;

	edges << std::endl;
	/*Initialize Graph*/
	nodes << "digraph \"QMDD\" {" << std::endl
			<< "graph [center=true, ordering=out];" << std::endl
			<< "node [shape=circle, center=true]; " << std::endl;
	nodes << "\"" << "T" << "\" " << "[ shape = box, label=\"" << "1" << "\" ];"
			<< std::endl;

	 /* Define Nodes */
	ListElementPtr first, q, lastq, pnext;
	uint64_t n, i, j;

	first = QMDDnewListElement();
	first->p = e.p;
	first->next = NULL;
	first->w = COMPLEX_ZERO;
	first->cnt = 1;
	n = 0;
	i = 0;

	nodes << "\"R\"";
	//füge Kante zwischen helper node und neuem Knoten hinzu
	if (e.w == COMPLEX_ONE) {
		nodes << " [label=\"\", shape=point];" << std::endl;
#if DOT_USE_CMAG
		edges << "\"R\" -> \"0\" [penwidth=5];" << std::endl;
#else
		edges << "\"R\" -> \"0\"" << std::endl;
#endif
	} else {
		nodes << " [label=\"\", shape=point];" << std::endl;
#if DOT_USE_CMAG
		edges << "\"R\" -> \"0\" [penwidth=" << Cmag[e.w & 0x7FFFFFFF7FFFFFFFull]*5 << "];" << std::endl;
#else
		edges << "\"R\" -> \"0\" [label=\"(";
		//<< c.r << ", " << c.i
		Cprint(e.w, edges);
		edges << ")\" ];" << std::endl;
#endif
	}

	//printf("top edge weight ");
	//Cprint(Cvalue(e.w));

	pnext = first;

	while (pnext != NULL) {

		/* Zeichne Knoten*/

		if (pnext->p->ident)
			nodes << "\"" << i << "\" " << "[ label=\""
					<< circ.line[((int) pnext->p->v)].variable
					<< "\" ,style=filled, fillcolor=red ];" << std::endl;
		else if (pnext->p->symm)
			nodes << "\"" << i << "\" " << "[ label=\""
					<< circ.line[((int) pnext->p->v)].variable
					<< "\" ,style=filled, fillcolor=green ];" << std::endl;
		else
			nodes << "\"" << i << "\" " << "[ label=\""
					<< circ.line[((int) pnext->p->v)].variable
					<< "\" ,style=filled, fillcolor=lightgray ];" << std::endl;

		/*begin line*/
		if (pnext->p != QMDDzero.p) {
			edges << "{rank=same;";
			for(int k=0; k<MAXNEDGE; k++) {
#if DOT_OUTPUT_VECTOR
				if(k % MAXRADIX != 0) continue;
#endif
				edges << " \"" << i << "h" << k << "\"";
			}
			edges << "}" << std::endl;
#if false
			edges << "{rank=same; " << "\"" << i << "h" << "0\" " << "\"" << i
					<< "h" << "1\" " << "\"" << i << "h" << "2\" " << "\"" << i
					<< "h" << "3\"" << "}" << std::endl;
#endif

			for (j = 0; j < Nedge; j++) {
#if DOT_OUTPUT_VECTOR
				if (j % MAXRADIX != 0) {
					continue;
				}
#endif
				if (pnext->p->e[j].p == NULL)
					;
				else {
					if (!QMDDterminal(pnext->p->e[j])) {
						q = first->next;
						lastq = first;
						while (q != NULL && pnext->p->e[j].p != q->p) {
							lastq = q;
							q = q->next;
						}
						if (q == NULL) {
							q = QMDDnewListElement();
							q->p = pnext->p->e[j].p;
							q->next = NULL;
							q->w = n = n + 1;
							q->cnt = 1;
							lastq->next = q;
						} else {
							q->cnt = q->cnt + 1;
						}
						nodes << "\"" << i << "h" << j << "\" ";

						//connect helper node

						edges << "\"" << i << "\" -> \"" << i << "h" << j
								<< "\" [arrowhead=none";

						switch (j) {
						case 0:
							edges << ",color=darkgreen";
							break;
						case 1:
							edges << ",color=blue";
							break;
						case 2:
							edges << ",color=red";
							break;
						case 3:
							edges << ",color=gold";
							break;
						default:
							break;
						}
						edges << "];" << std::endl;
//		      << q->w  << "\" [label=\"" ;

						//	    "1."Cvalue(pnext->p->e[j].w);
						//füge Kante zwischen helper node und neuem Knoten hinzu
						if (/*c.r ==  1 && c.i == 0*/pnext->p->e[j].w == COMPLEX_ONE) {
							nodes << " [label=\"\", shape=point];" << std::endl;
#if DOT_USE_CMAG
							edges << "\"" << i << "h" << j << "\" -> \"" << q->w
									<< "\" [penwidth=5];" << std::endl;

#else
							edges << "\"" << i << "h" << j << "\" -> \"" << q->w
									<< "\";" << std::endl;
#endif
						} else {
							nodes << " [label=\"\", shape=point];" << std::endl;
#if DOT_USE_CMAG
							edges << "\"" << i << "h" << j << "\" -> \"" << q->w << "\" [penwidth=" << Cmag[pnext->p->e[j].w & 0x7FFFFFFF7FFFFFFFull]*5;
							//<< c.r << ", " << c.i
							edges << "];" << std::endl;

#else
							edges << "\"" << i << "h" << j << "\" -> \"" << q->w
									<< "\" [label=\" (";
							Cprint(pnext->p->e[j].w, edges);
							//<< c.r << ", " << c.i
							edges << ")\" ];" << std::endl;
#endif
						}

					} else {

						nodes << "\"" << i << "h" << j << "\" ";
						nodes << " [label=\"\", shape=point ";

						edges << "\"" << i << "\" -> \"" << i << "h" << j
								<< "\" [arrowhead=none";
						switch (j) {
						case 0:
							edges << ",color=darkgreen";
							break;
						case 1:
							edges << ",color=blue";
							break;
						case 2:
							edges << ",color=red";
							break;
						case 3:
							edges << ",color=gold";
							break;
						default:
							break;
						}
						edges << "];" << std::endl;
						//connect helper node
						if (/*c.r == 0 && c.i == 0*/pnext->p->e[j].w == COMPLEX_ZERO) {

							nodes << ", fillcolor=red, color=red";
						} else if (pnext->p->e[j].w == COMPLEX_ONE) {
#if DOT_USE_CMAG
							edges << "\"" << i << "h" << j << "\" -> \"T\""
									<< " [penwidth=5];" << std::endl;
#else
							edges << "\"" << i << "h" << j << "\"-> \"T\";"
									<< std::endl;
#endif
						} else {
#if DOT_USE_CMAG
							edges << "\"" << i << "h" << j << "\" -> \"T\""
									<< " [penwidth=" << (Cmag[pnext->p->e[j].w & 0x7FFFFFFF7FFFFFFFull]*5);
							//<< c.r << ", " << c.i
							edges << "];" << std::endl;

#else
							edges << "\"" << i << "h" << j
									<< "\"-> \"T\" [label= \"(";
							Cprint(pnext->p->e[j].w, edges);
							//<< c.r <<", " << c.i
							edges << ")\", ];" << std::endl;
#endif
						}
						nodes << "];" << std::endl;

					}
				}
			}
		}
		i++;
		pnext = pnext->next;
	}
	oss << nodes.str() << edges.str() << std::endl << "}" << std::endl;
}

void QMDDdotExport(QMDDedge basic, int n, char outputFilename[],
		QMDDrevlibDescription circ, int show) {
// export a QMDD in .dot format to the file specified by outputFilename
// and call DOT->SVG export (optional, requires dot package)

	char sysCall[100];

	std::ofstream init(outputFilename);
	QMDD2dot(basic, n, init, circ);
	init.close();

	if (show) {
		strcpy(sysCall, "dot -Tsvg ");
		strcat(sysCall, outputFilename);
		strcat(sysCall, " -o ");
		strcat(sysCall, outputFilename);
		strcat(sysCall, ".svg");

		system(sysCall);
	}
}

#ifdef __NormA__
QMDDedge QMDDnormalize(QMDDedge e)
//  normalize a QMDD node adjusting edge e
//  uses normalization as defined in the ISMVL-2006
//  paper by Miller and Thornton modified for garbage line collapsing
		{
	int i, j;

	e.w = COMPLEX_ONE;
	for (i = 0; (e.p->e[i].p == NULL || e.p->e[i].w == COMPLEX_ZERO) && i < Nedge; i++)
		; //look for the first non-zero entry
	if (i == Nedge) {
		e.w = COMPLEX_ZERO;
		return (e); // check validity
	}
	// normalize edge by setting the first non-zero weight as the new incoming weight and adjusting all other weights
	if (e.p->e[i].w == COMPLEX_ONE)
		return (e);
	e.w = e.p->e[i].w;
	for (j = 0; j < Nedge; j++)
		if (i == j)
			e.p->e[j].w = COMPLEX_ONE;
		else if (e.p->e[j].p != NULL && e.p->e[j].w != COMPLEX_ZERO) {
			e.p->e[j].w = Cdiv(e.p->e[j].w, e.w);
		}
	return (e);
}

#endif

#ifdef __NormB__
QMDDedge QMDDnormalize(QMDDedge e)
//  normalize a QMDD node adjusting edge e
//  uses normalization as defined in the ISMVL-2007
//  paper by Miller, Feinstein and Thornton

// ADDON by Niemann (new normalization): take the leftmost edge with largest [smallest non-zero] modulus
{
	int i,j,c;  // i,j are counters. c contains the largest weight found so far

	for(i=0;(e.p->e[i].p==NULL||e.p->e[i].w==COMPLEX_ZERO);i++);
	//move forward to first non-zero entry
	c=e.p->e[i].w;

	/** This part for the standard double Package **/

	for(j=i+1;j<Nedge;j++)// consider all further edges...
	if(e.p->e[j].p!=NULL&&e.p->e[j].w!=COMPLEX_ZERO)//  ..and if valid...
	if(Cgt(e.p->e[j].w,c)) { // .. compare the weights
		i=j;
		c=e.p->e[j].w;
	}
	if(e.p->e[i].w==COMPLEX_ONE) {e.w = COMPLEX_ONE; return(e);} // already normalized!

	e.w=e.p->e[i].w;
	for(j=0;j<Nedge;j++)
	if(i==j) e.p->e[j].w=COMPLEX_ONE;
	else if(e.p->e[j].p!=NULL&&e.p->e[j].w!=COMPLEX_ZERO)
	e.p->e[j].w=Cdiv(e.p->e[j].w,e.w);

	return(e);
}
#endif


#ifdef __NormC__
QMDDedge QMDDnormalize(QMDDedge e) {
	int i, j;

	e.w = COMPLEX_ONE;
	//complex c;

	///bool first = true;

	int max = -1;

	std::unordered_map<uint64_t, mpreal>::iterator last = Cmag.end();
	for (i = 0; i < Nedge; i++) {
		if((e.p->e[i].p == NULL || e.p->e[i].w == COMPLEX_ZERO)) {
			continue;
		}
#ifdef QMDDcomplex_H_QOmega
		std::unordered_map<uint64_t, mpreal>::iterator it = Cmag.find(e.p->e[i].w);
#else
		std::unordered_map<uint64_t, mpreal>::iterator it = Cmag.find(e.p->e[i].w & 0x7FFFFFFF7FFFFFFFull);
#endif
		if(it == Cmag.end()) {
			std::cout << "Error: magnitude not found: " << e.p->e[i].w << std::endl;
			std::cout << "Magnitudes: " << Cmag.size() << std::endl;
			for(auto it2 = Cmag.begin(); it2 != Cmag.end(); it2++) {
				std::cout << it2->first << ": " << it2->second << std::endl;
			}
			exit(111);
		}
		if(max == -1) {
			last = it;
			//mpfr_set(tmp_c.r, it->second.mpfr_srcptr(), MPFR_RNDN);
			max = i;
		} else if(mpfr_cmp(it->second.mpfr_srcptr(), last->second.mpfr_srcptr()) > 0) {
//			mpfr_set(tmp_c.r, it->second.mpfr_srcptr(), MPFR_RNDN);
			last = it;
			max = i;
		}
	}
//	std::cout << "max: " << max  << std::endl;
	//	std::cout << "max: " << Cprint(tmp_c, )

		; //look for the first non-zero entry
	if (max == -1) {
		e.w = COMPLEX_ZERO;
		return (e); // check validity
	}

	i = max;

	// normalize edge by setting the first non-zero weight as the new incoming weight and adjusting all other weights
	if (e.p->e[i].w == COMPLEX_ONE)
		return (e);
	e.w = e.p->e[i].w;
	for (j = 0; j < Nedge; j++)
		if (i == j)
			e.p->e[j].w = COMPLEX_ONE;
		else if (e.p->e[j].p != NULL && e.p->e[j].w != COMPLEX_ZERO) {
			e.p->e[j].w = Cdiv(e.p->e[j].w, e.w);
		}
	return (e);
}

#endif


void QMDDcheckSpecialMatrices(QMDDedge e)
//  check if e points to a block, identity, diagonal, symmetric or 0/1-matrix and
//  marks top node if it does
		{
	int i, j, w;
	QMDDedge t;

	// only perform checks if flag is set
	if (!e.p->computeSpecialMatricesFlag)
		return;

	e.p->ident = 0; 	   // assume not identity
	e.p->diag = 0;	   	   // assume not diagonal
	e.p->block = 0;		   // assume not block
	e.p->symm = 1;		   // assume symmetric
	e.p->c01 = 1;		   // assume 0/1-matrix

	/****************** CHECK IF 0-1 MATRIX ***********************/

	for (i = 0; i < Nedge; i++)  // check if 0-1 matrix
		if ((e.p->e[i].w != COMPLEX_ONE && e.p->e[i].w != COMPLEX_ZERO )  || (!e.p->e[i].p->c01)) {
			e.p->c01 = 0;
			break;
		}

	/****************** CHECK IF Symmetric MATRIX *****************/

	for (i = 0; i < Radix; i++)  // check if symmetric matrix (on diagonal)
		if (!(e.p->e[Radix * i + i].p->symm)) {
			e.p->symm = 0;
			break;
		}

	//printf("specMat for %d. symm=%s", (intptr_t) e.p, e.p->symm?"diag":"nodiag");

	for (i = 0; e.p->symm && i < Radix - 1; i++) { // check off diagonal entries for transpose properties
		for (j = i + 1; j < Radix; j++) {
			t = QMDDtranspose(e.p->e[i * Radix + j]);
			if (!QMDDedgeEqual(t, e.p->e[j * Radix + i])) {
				e.p->symm = 0;
				break;
			}
		}
	}

	//printf(",%s\n", e.p->symm?"offdiag":"notOffDiag");

	w = QMDDinvorder[e.p->v];
	if (w != 0)
		w = QMDDorder[w - 1];
	// w:= variable one level below current level or 0 if already at the bottom

	/****************** CHECK IF Block MATRIX ***********************/

	for (i = 0; i < Radix; i++) // check off diagonal entries
		for (j = 0; j < Radix; j++)
			if (e.p->e[i * Radix + j].p == NULL
					|| (i != j && e.p->e[i * Radix + j].w != COMPLEX_ZERO))
				return;
	e.p->block = 1;

	/****************** CHECK IF Diagonal MATRIX ***********************/
	// will only reach this point if block == 1
	e.p->diag = 1;
	for (i = 0; i < Radix; i++) // check diagonal entries to verify matrix is diagonal
			{
		// necessary condition: edge points to a diagonal matrix
		e.p->diag = e.p->e[i * Radix + i].p->diag;
		j = Radix * i + i;

		// skipped variable: edge pointing to terminal with non-zero weight from level > 0
		if ((QMDDterminal(e.p->e[j])) && e.p->e[j].w != COMPLEX_ZERO
				&& QMDDinvorder[e.p->v] != 0) /*return; /*/
			e.p->diag = 0;
		// skipped variable: edge pointing to an irregular level (non-terminal)
		if ((!QMDDterminal(e.p->e[j])) && e.p->e[j].p->v != w) /*return; /*/
			e.p->diag = 0;

		if (!e.p->diag)
			return;
	}

	/****************** CHECK IF Identity MATRIX ***********************/
	// will only reach this point if diag == 1
	for (i = 0; i < Radix; i++)  // check diagonal entries
			{
		j = Radix * i + i;
		// if skipped variable, then matrix cannot be diagonal (and we will not reach this point)!
		if (e.p->e[j].w != COMPLEX_ONE || e.p->e[j].p->ident == 0)
			return;
	}
	e.p->ident = 1;
	return;
}

QMDDedge QMDDutLookup(QMDDedge e) {
//  lookup a node in the unique table for the appropriate variable - if not found insert it
//  only normalized nodes shall be stored.

	intptr_t key;
	int i;
	unsigned int v;
	QMDDnodeptr p;

	if (QMDDterminal(e)) // there is a unique terminal node
			{
		e.p = QMDDzero.p;
		return (e);
	}

	UTlookups++;

	key = 0;
// note hash function shifts pointer values so that order is important
// suggested by Dr. Nigel Horspool and helps significantly
	for (i = 0; i < Nedge; i++)
		key += ((intptr_t)(e.p->e[i].p) >> i) + (e.p->e[i].w >> 32) + e.p->e[i].w;
	key = (key) & HASHMASK;

	//TODO: remove again after fixing hash function
	UTkeys[key]++;
	v = (unsigned int) e.p->v;
	p = Unique[v][key]; // find pointer to appropriate collision chain
	//lastp=NULL;	    // pN: not necessary, don't need to jump back to predecessor
	while (p != NULL)    // search for a match
	{
		if (memcmp(e.p->e, p->e, Nedge * sizeof(QMDDedge)) == 0) {
			// Match found
			e.p->next = Avail; 	// put node pointed to by e.p on avail chain
			Avail = e.p;

			// NOTE: reference counting is to be adjusted by function invoking the table lookup
			UTmatch++;		// record hash table match

			e.p = p;// and set it to point to node found (with weight unchanged)

			if (p->renormFactor != COMPLEX_ONE) {
				printf(
						"Debug: table lookup found a node with active renormFactor with v=%d (id=%ld).\n",
						p->v, (intptr_t) p);
				if (p->ref != 0)
					printf("was active!");
				else
					printf("was inactive!");
				exit(66);
				e.w = Cdiv(e.w, e.p->renormFactor);
			}
			return (e);
		}

		UTcol++; 		// record hash collision
		//lastp=p;
		p = p->next;
	}
	e.p->next = Unique[v][key]; // if end of chain is reached, this is a new node
	Unique[v][key] = e.p;       // add it to front of collision chain

	QMDDnodecount++;          // count that it exists
	if (QMDDnodecount > QMDDpeaknodecount)
		QMDDpeaknodecount = QMDDnodecount;

	if (!QMDDterminal(e))
		QMDDcheckSpecialMatrices(e); // check if it is identity or diagonal if nonterminal

	return (e);                // and return
}

void QMDDinitComputeTable(void)
// set compute table to empty and
// set toffoli gate table to empty and
// set identity table to empty
		{
	int i;

	CTable_add.clear();
	CTable_mult.clear();
	CTable_transpose.clear();
	CTable_conjugateTranspose.clear();
	CTable_renormalize.clear();

	/*  for(i=0;i<CTSLOTS;i++)
	 {
	 CTable[i].r.p=NULL;
	 CTable[i].which=none;
	 }*/
	for (i = 0; i < TTSLOTS; i++)
		TTable[i].e.p = NULL;
	for (i = 0; i < MAXN; i++)
		QMDDid[i].p = NULL;
	QMDDnullEdge.p = NULL;
	QMDDnullEdge.w = COMPLEX_ONE;
}

void QMDDgarbageCollect(void)
// a simple garbage collector that removes nodes with 0 ref count from the unique
// tables placing them on the available space chain
		{
	int i, j;
	int count, counta;
	QMDDnodeptr p, lastp, nextp;

	if (QMDDnodecount < GCcurrentLimit)
		return; // do not collect if below GCcurrentLimit node count
	count = counta = 0;
	//printf("starting garbage collector %d nodes\n",QMDDnodecount);
	for (i = 0; i < MAXN; i++)
		for (j = 0; j < NBUCKET; j++) {
			lastp = NULL;
			p = Unique[i][j];
			while (p != NULL) {
				if (p->ref == 0) {
					if (p == QMDDtnode)
						printf("error in garbage collector\n");
					count++;
					nextp = p->next;
					if (lastp == NULL)
						Unique[i][j] = p->next;
					else
						lastp->next = p->next;
					p->next = Avail;
					Avail = p;
					p = nextp;
				} else {
					lastp = p;
					p = p->next;
					counta++;
				}
			}
		}
	//printf("%d nodes recovered %d nodes active\n",count,counta);
	GCcurrentLimit += GCLIMIT_INC;
	QMDDnodecount = counta;
	QMDDinitComputeTable(); // IMPORTANT sets compute table to empty after garbage collection
}

QMDDnodeptr QMDDgetNode(void) {
// get memory space for a node
//
	QMDDnodeptr r, r2;
	int i, j;

	if (Avail != NULL)	// get node from avail chain if possible
	{
		r = Avail;
		Avail = Avail->next;
	} else {			// otherwise allocate 2000 new nodes

		//printf("no space available. allocate 2000 new nodes\n");

		j = sizeof(QMDDnode);//+Nedge*sizeof(QMDDedge);				// estimated value of a QMDDnode ! DANGER ous. pN calculated 44=sizeof(QMDDnode)+Nedge*sizeof(QMDDedge)
		r = (QMDDnodeptr) malloc(2000 * j);
		r2 = (QMDDnodeptr) ((int64_t) r + j);
		Avail = r2;
		for (i = 0; i < 1998; i++, r2 = (QMDDnodeptr) ((int64_t) r2 + j)) {
			r2->next = (QMDDnodeptr) ((int64_t) r2 + j);
		}
		r2->next = NULL;
	}
	r->next = NULL;
	r->ref = 0;			// set reference count to 0
	r->ident = r->diag = r->block = 0;		// mark as not identity or diagonal
	return (r);
}

void QMDDincref(QMDDedge e)
// increment reference counter for node e points to
// and recursively increment reference counter for 
// each child if this is the first reference
//
// a ref count saturates and remains unchanged if it has reached
// MAXREFCNT
		{
	int i;

	if (QMDDterminal(e))
		return;

	if (e.p->ref == MAXREFCNT) {
		printf("MAXREFCNT reached\n\n\n");
		std::cout << "e.w=" << e.w << /*" e.sentinel=" << e.sentinel <<*/ std::endl;
		QMDDdebugnode(e.p);
		return;
	}
	e.p->ref++;

	/*if (e.p->ref > largestRefCount) { //record maximum of Reference counts
	 largestRefCount = e.p->ref;
	 if (largestRefCount % 1000 == 0)
	 printf("new MAXREF %d  ... %d, %d\n", largestRefCount, (intptr_t) e.p, (intptr_t) QMDDzero.p);
	 }*/

	if (e.p->ref == 1) {
		if (!QMDDterminal(e))
			for (i = 0; i < Nedge; i++)
				if (e.p->e[i].p != NULL)
					QMDDincref(e.p->e[i]);

		Active[e.p->v]++;
		ActiveNodeCount++;

		/******* Part added for sifting purposes ********/
		if (e.p->block)
			blockMatrixCounter++;
		/******* by Niemann, November 2012 ********/

	}
}

void QMDDdecref(QMDDedge e)
// decrement reference counter for node e points to
// and recursively decrement reference counter for 
// each child if this is the last reference
//
// a ref count saturates and remains unchanged if it has reached
// MAXREFCNT
		{
	int i;

	if (QMDDterminal(e))
		return;

	if (e.p->ref == MAXREFCNT)
		return;
	e.p->ref--;
	if (e.p->ref == (unsigned int) -1) // ERROR CHECK
			{
		printf("error in decref %d\n", e.p->ref);
		QMDDdebugnode(e.p);
		exit(8);
	}
	if (e.p->ref == 0) {
		if (!QMDDterminal(e))
			for (i = 0; i < Nedge; i++)
				if (e.p->e[i].p != NULL)
					QMDDdecref(e.p->e[i]);
		Active[e.p->v]--;
		if (Active[e.p->v] < 0)
			printf("ERROR in decref\n");
		ActiveNodeCount--;

		/******* Part added for sifting purposes ********/
		if (e.p->renormFactor != COMPLEX_ONE) {
			RenormalizationNodeCount--;
			e.p->renormFactor = COMPLEX_ONE;
		}
		if (e.p->block)
			blockMatrixCounter--;
		/******* by Niemann, November 2012 ********/
	}
}

int QMDDnodeCount(QMDDedge e)
// a very simplistic recursive routine for counting
// number of unique nodes in a QMDD
// NEEDS TO BE REDONE
		{
	int i, sum;

	for (i = 0; i < Ncount; i++)
		if (Nlist[i] == e.p)
			return (0);
	Nlist[Ncount] = e.p;
	Ncount++;
	sum = 1;
	if (!QMDDterminal(e))
		for (i = 0; i < Nedge; i++)
			if (e.p->e[i].p != NULL)
				sum += QMDDnodeCount(e.p->e[i]);
	if (sum > MAXNODECOUNT)
		return (MAXNODECOUNT);
	return (sum);
}

void QMDDradixPrint(int p, int n)
// prints p as an n bit Radix number
// with leading 0's and no CR
		{
	int i, buffer[MAXN];
	for (i = 0; i < n; i++) {
		buffer[i] = p % Radix;
		p = p / Radix;
	}
	for (i = n - 1; i >= 0; i--)
		printf("%d", buffer[i]);
}

#define CThash(a,b) (((((int64_t)a.p+(int64_t)b.p)>>3)+(int)a.w+(int)b.w+(int)which)&CTMASK)

QMDDedge CTlookup(QMDDedge a, QMDDedge b, CTkind which) {
// Lookup a computation in the compute table
// return NULL if not a match else returns result of prior computation
	//int i;
	QMDDedge r;

	r.p = NULL;
	CTlook[which]++;
	//i = CThash(a, b);

	computeKey ck;
	ck.a = a;
	ck.b = b;
	if (which == mult) {
		std::unordered_map<computeKey, QMDDedge, computeHasher>::iterator it =
				CTable_mult.find(ck);
		if (it != CTable_mult.end()) {
			CThit[which]++;
			return it->second;
		}
	} else if (which == add) {
		std::unordered_map<computeKey, QMDDedge, computeHasher>::iterator it =
				CTable_add.find(ck);
		if (it != CTable_mult.end()) {
			CThit[which]++;
			return it->second;
		}
	} else if (which == transpose) {
		std::unordered_map<computeKey, QMDDedge, computeHasher>::iterator it =
				CTable_transpose.find(ck);
		if (it != CTable_mult.end()) {
			CThit[which]++;
			return it->second;
		}
	} else if (which == conjugateTranspose) {
		std::unordered_map<computeKey, QMDDedge, computeHasher>::iterator it =
				CTable_transpose.find(ck);
		if (it != CTable_conjugateTranspose.end()) {
			CThit[which]++;
			return it->second;
		}
	} else if (which == renormalize) {
		std::unordered_map<computeKey, QMDDedge, computeHasher>::iterator it =
				CTable_transpose.find(ck);
		if (it != CTable_renormalize.end()) {
			CThit[which]++;
			return it->second;
		}
	} else {
		std::cout << "unsupported operation: " << which << std::endl;
	}
	return r;

	/*  if(CTable[i].which!=which) return(r);
	 if(CTable[i].a.p!=a.p||CTable[i].a.w!=a.w) return(r);
	 if(CTable[i].b.p!=b.p||CTable[i].b.w!=b.w) return(r);
	 if((CTable[i].r.p)->v==-1) return(r);
  	 CThit[which]++;
  return(CTable[i].r);
  */
}

void CTinsert(QMDDedge a, QMDDedge b, QMDDedge r, CTkind which) {
// put an entry into the compute table
	//int i;
	computeKey ck;
	ck.a = a;
	ck.b = b;
	if (which == mult) {
		CTable_mult[ck] = r;
	} else if (which == add) {
		CTable_add[ck] = r;
	} else if (which == transpose) {
		CTable_transpose[ck] = r;
	} else if (which == conjugateTranspose) {
		CTable_conjugateTranspose[ck] = r;
	} else if (which == renormalize) {
		CTable_renormalize[ck] = r;
	} else {
		std::cout << "unsupported operation: " << which << std::endl;
	}
	/*  i=CThash(a,b);
	 CTable[i].a=a;
	 CTable[i].b=b;
	 CTable[i].r=r;
	 CTable[i].which=which;*/
}

int TThash(int n, int m, int t, int line[]) {
	int i, j;
	i = t;
	for (j = 0; j < n; j++)
		if (line[j] == 1)
			i = (i << 3) + j;
	return (i & TTMASK);
}

QMDDedge TTlookup(int n, int m, int t, int line[])
// does not work with SIFTING!! Idea: TT should be initialized
		{
	QMDDedge r;
	int i;
	r.p = NULL;
	i = TThash(n, m, t, line);
	if (TTable[i].e.p == NULL || TTable[i].t != t || TTable[i].m != m
			|| TTable[i].n != n)
		return (r);
	//for(j=0;j<n;j++) if(TTable[i].line[j]!=line[j]) return(r);
	if (0 == memcmp(TTable[i].line, line, n * sizeof(int)))
		return (TTable[i].e);
	return (r);
}

void TTinsert(int n, int m, int t, int line[], QMDDedge e) {
	int i;
	i = TThash(n, m, t, line);
	TTable[i].n = n;
	TTable[i].m = m;
	TTable[i].t = t;
	memcpy(TTable[i].line, line, n * sizeof(int));
	TTable[i].e = e;
}

void QMDDfillmat(uint64_t mat[MAXDIM][MAXDIM], QMDDedge a, int r, int c,
		int dim, short v, char vtype[])
// recursively scan an QMDD putting values in entries of mat
// v is the variable index
		{
	int i, expand;
	QMDDedge e;

	if (a.p == NULL)
		return;

	if (v == -1) // terminal node case
			{
		if (r >= MAXDIM || c >= MAXDIM) {
			printf("out of bounds, r=%d, c=%d\n", r, c);
			return;
		}
		mat[r][c] = a.w;
	} else {
		expand = (QMDDterminal(a)) || v != QMDDinvorder[a.p->v];
		for (i = 0; i < Nedge; i++) {
			if ((vtype[v] == 0) || (vtype[v] == 1 && i < Radix)
					|| (vtype[v] == 2 && (i % Radix == 0))) {
				if (expand) {
					QMDDfillmat(mat, a, r + (i / Radix) * dim / Radix,
							c + (i % Radix) * dim / Radix, dim / Radix, v - 1,
							vtype);
				} else {
					e = a.p->e[i];
					e.w = Cmul(a.w, e.w);
					//e.w = 1; // pN
					QMDDfillmat(mat, e, r + (i / Radix) * dim / Radix,
							c + (i % Radix) * dim / Radix, dim / Radix, v - 1,
							vtype);
				}
			}
		}
	}
}

void recQMDDrcPrint(QMDDedge p, short n, short w)
// called by QMDDcolumnPrint and QMDDrowPrint to do actual printing
// w==2 for column; w==1 for row
		{
	int i, j, k, limit;
	QMDDedge e;

	if (QMDDterminal(p))
		k = n + 1;
	else
		k = n - QMDDinvorder[p.p->v];
	limit = 1;
	for (j = 0; j < k; j++)
		limit *= Radix;
	for (j = 0; j < limit; j++)
		if (QMDDterminal(p)) {
			if (p.w == COMPLEX_ONE || p.w == COMPLEX_ZERO)
				printf("%d", p.w);
			else
				Cprint(p.w);
			printf(" ");
		} else {
			k = 0;
			for (i = 0; i < Radix; i++) {
				e = p.p->e[k];
				e.w = Cmul(e.w, p.w);
				recQMDDrcPrint(e, QMDDinvorder[p.p->v] - 1, w);
				if (w == 1)
					k++;
				else
					k += Radix;
			}
		}
}

void QMDDpermPrint(QMDDedge e, int row, int col) {
	int i;
	if (QMDDterminal(e)) {
		if (e.w != COMPLEX_ONE)
			printf("error in permutation printing/n");
		else
			PermList[col] = row;
	} else
		for (i = 0; i < Nedge; i++)
			if (e.p->e[i].p != NULL && e.p->e[i].w != COMPLEX_ZERO)
				QMDDpermPrint(e.p->e[i], row * Radix + i / Radix,
						col * Radix + i % Radix);
}

/***************************************

 Public Routines

 ***************************************/

QMDDedge QMDDmakeNonterminal(short v, QMDDedge edge[]) {
// make a QMDD nonterminal node and return an edge pointing to it
// node is not recreated if it already exists
	QMDDedge e;
	int i, redundant;

	redundant = 1;// check if redundant node = all edges point to the same node (with same weight)
	e = edge[0];

	i = 1;
	while (redundant && (i < Nedge)) {
		redundant = (edge[i].p == NULL)
				|| ((edge[i].w == e.w) && (edge[i].p == e.p));
		i++;
	}
	if (redundant)
		return (edge[0]); // return 0 child if redundant

	e.p = QMDDgetNode();  // get space and form node
	e.w = COMPLEX_ONE;
	//e.sentinel = 0;  // see QMDDpackage.h for information about sentinel
	e.p->v = v;
	e.p->renormFactor = COMPLEX_ONE;
	e.p->computeSpecialMatricesFlag = globalComputeSpecialMatricesFlag;

	memcpy(e.p->e, edge, Nedge * sizeof(QMDDedge));
	e = QMDDnormalize(e); // normalize it
	e = QMDDutLookup(e);  // look it up in the unique tables
	return (e);		  // return result
}

QMDDedge QMDDmakeTerminal(uint64_t w)
// make a terminal - actually make an edge with appropriate weight
// as there is only one terminal QMDDone
		{

	QMDDedge e;

	e.p = QMDDtnode;
	e.w = w;
	//e.sentinel = 0;	// see QMDDpackage.h for information about sentinel
	return (e);
}

void QMDDinit(int verbose) {
// initialize QMDD package - must be called before other routines are used
//
	int i, j;

	if (verbose) {
		printf(QMDDversion);
		printf("compiled: %s %s\n\n", __DATE__, __TIME__);
		printf("Edge size %ld bytes\n", sizeof(QMDDedge));
		printf("Node size %ld bytes\n",
				sizeof(QMDDnode) + Nedge * sizeof(QMDDedge));
		printf(
				"Max variables %d\nUT buckets / variable %d\nCompute table slots %d\nToffoli table slots %d\nGarbage collection limit %d\nGarbage collection increment %d\nComplex number table size %d\n",
				MAXN, NBUCKET, CTSLOTS, TTSLOTS, GCLIMIT1, GCLIMIT_INC,
				COMPLEXTSIZE);
	}


	Nedge = Radix * Radix;	   // set number of edges

	QMDDcomplexInit();	   // init complex number package
	QMDDinitComputeTable();  // init computed table to empty

	GCcurrentLimit = GCLIMIT1; // set initial garbage collection limit

	UTcol = UTmatch = UTlookups = 0;

	for(int i=0; i<NBUCKET; i++) {
		UTkeys[i] = 0;
	}

	QMDDnodecount = 0;			// zero node counter
	QMDDpeaknodecount = 0;
	Nlabel = 0;                		// zero variable label counter
	Nop[0] = Nop[1] = Nop[2] = 0;		// zero op counter
	CTlook[0] = CTlook[1] = CTlook[2] = CThit[0] = CThit[1] = CThit[2] = 0;	// zero CTable counters
	Avail = NULL;				// set available node list to empty
	Lavail = NULL;				// set available element list to empty
	QMDDtnode = QMDDgetNode();// create terminal node - note does not go in unique table
	QMDDtnode->ident = 1;
	QMDDtnode->diag = 1;
	QMDDtnode->block = 0;  // changed by Niemann 121109
	QMDDtnode->symm = 1;
	QMDDtnode->c01 = 1;
	QMDDtnode->renormFactor = COMPLEX_ONE;
	QMDDtnode->computeSpecialMatricesFlag = 0;
	for (i = 0; i < Nedge; i++) {
		QMDDtnode->e[i].p = NULL;
		QMDDtnode->e[i].w = COMPLEX_ZERO;
	}
	QMDDtnode->v = -1;// pN 120814: if ==1 it counts for Active[1]...bad for sifting

	QMDDzero = QMDDmakeTerminal(COMPLEX_ZERO);
	QMDDone = QMDDmakeTerminal(COMPLEX_ONE);


	for (i = 0; i < MAXN; i++)
		for (j = 0; j < NBUCKET; j++) // set unique tables to empty
			Unique[i][j] = NULL;
	for (i = 0; i < MAXN; i++) //  set initial variable order to 0,1,2... from bottom up
			{
		QMDDorder[i] = QMDDinvorder[i] = i;
		Active[i] = 0;
	}
	ActiveNodeCount = 0;
	QMDDinitGateMatrices();
	if (verbose)
		printf(
				"QMDD initialization complete\n----------------------------------------------------------\n");
}

QMDDedge QMDDadd(QMDDedge x, QMDDedge y)
// adds two matrices represented by QMDD
// the two QMDD should have the same variable set and ordering
		{
	QMDDedge e1, e2, e[MAXNEDGE], r;
	int i, w;

	if (x.p == NULL)
		return (y);  // handles partial matrices i.e.
	if (y.p == NULL)
		return (x);  // column and row vetors
	Nop[add]++;
	if ((!MultMode) && (QMDDterminal(y) || x.p > y.p)) {
		e1 = x;
		x = y;
		y = e1;
	}

	if (x.w == COMPLEX_ZERO) {
		return (y);
	}
	if (y.w == COMPLEX_ZERO) {
		return (x);
	}
	if (x.p == y.p) {
		r = y;
		r.w = Cadd(x.w, y.w);
		if (r.w == COMPLEX_ZERO)
			r = QMDDzero;
		return (r);
	}

	uint64_t xweight; //, yweight;
	char newCT = 1;

	if (newCT) {
		xweight = x.w;
		//yweight = y.w;
		x.w = COMPLEX_ONE;
		y.w = Cdiv(y.w, xweight);
	}

	r = CTlookup(x, y, add);
	if (r.p != NULL) {
		if (newCT) {
			r.w = Cmul(r.w, xweight);
		}
		return (r);
	}

	if (QMDDterminal(x))
		w = y.p->v;
	else {
		w = x.p->v;
		if (!QMDDterminal(y))
			if (QMDDinvorder[y.p->v] > QMDDinvorder[w])
				w = y.p->v;
	}

	for (i = 0; i < Nedge; i++) {
		if (!QMDDterminal(x) && x.p->v == w) {
			e1 = x.p->e[i];
			e1.w = Cmul(e1.w, x.w);
		} else {
			if ((!MultMode) || (i % Radix == 0)) {
				e1 = x;
				if (y.p->e[i].p == 0)
					e1 = QMDDnullEdge;
			} else {
				e1.p = NULL;
				e1.w = COMPLEX_ZERO;
			}
		}
		if (!QMDDterminal(y) && y.p->v == w) {
			e2 = y.p->e[i];
			e2.w = Cmul(e2.w, y.w);
		} else {
			if ((!MultMode) || (i % Radix == 0)) {
				e2 = y;
				if (x.p->e[i].p == 0)
					e2 = QMDDnullEdge;
			} else {
				e2.p = NULL;
				e2.w = COMPLEX_ZERO;
			}
		}
		e[i] = QMDDadd(e1, e2);
	}
	r = QMDDmakeNonterminal(w/*x.p->v*/, e);  /// sept 29
	CTinsert(x, y, r, add);

	if (newCT) {
		r.w = Cmul(r.w, xweight);
	}
	return (r);
}

QMDDedge QMDDmultiply2(QMDDedge x, QMDDedge y, int var)

// new multiply routine designed to handle missing variables properly
// var is number of variables
		{
	QMDDedge e1, e2, e[MAXNEDGE], r;
	int i, j, k, w;

	uint64_t xweight, yweight;
	char newCT = 1;

	if (x.p == NULL)
		return (x);
	if (y.p == NULL)
		return (y);

	Nop[mult]++;

	if (x.w == COMPLEX_ZERO || y.w == COMPLEX_ZERO)  // the 0 case
			{
		return (QMDDzero);
	}

	if (var == 0)
		return (QMDDmakeTerminal(Cmul(x.w, y.w)));

	/// added by Niemann Nov 2012
	// extract and store edge weights of the factors
	// compute /look up product without these factors
	// multiply result with the product of the initial edge weights
	//
	// this gives a higher CT hit rate!
	if (newCT) {
		xweight = x.w;
		yweight = y.w;
		x.w = COMPLEX_ONE;
		y.w = COMPLEX_ONE;
	}

	r = CTlookup(x, y, mult);
	if (r.p != NULL) {
		if (newCT) {
			r.w = Cmul(r.w, xweight);
			r.w = Cmul(r.w, yweight);
		}
		return (r);
	}

	w = QMDDorder[var - 1];

	if (x.p->v == w && x.p->v == y.p->v) {
		if (x.p->ident) {
			r = y;
			if (!newCT)
				r.w = Cmul(r.w, x.w);
			CTinsert(x, y, r, mult);
			if (newCT)
				r.w = Cmul(xweight, yweight);
			return (r);
		}
		if (y.p->ident) {
			r = x;
			if (!newCT)
				r.w = Cmul(r.w, y.w);
			CTinsert(x, y, r, mult);
			if (newCT)
				r.w = Cmul(xweight, yweight);
			return (r);
		}

	}

	for (i = 0; i < Nedge; i += Radix) {
		for (j = 0; j < Radix; j++) {
			e[i + j].p = NULL;
			e[i + j].w = COMPLEX_ZERO;
			for (k = 0; k < Radix; k++) {
				if (!QMDDterminal(x) && x.p->v == w) {
					e1 = x.p->e[i + k];
					e1.w = Cmul(e1.w, x.w);
				} else {
					e1 = x;
				}
				if (!QMDDterminal(y) && y.p->v == w) {
					e2 = y.p->e[j + Radix * k];
					e2.w = Cmul(e2.w, y.w);
				} else {
					e2 = y;
				}

				e[i + j] = QMDDadd(e[i + j], QMDDmultiply2(e1, e2, var - 1));
			}
		}
	}
	r = QMDDmakeNonterminal(w, e);
	CTinsert(x, y, r, mult);
	if (newCT) {
		r.w = Cmul(r.w, xweight);
		r.w = Cmul(r.w, yweight);
	}
	return (r);
}

QMDDedge QMDDmultiply(QMDDedge x, QMDDedge y) {
	int var;

	var = 0;
	if (!QMDDterminal(x) && (QMDDinvorder[x.p->v] + 1) > var)
		var = QMDDinvorder[x.p->v] + 1;
	if (!QMDDterminal(y) && (QMDDinvorder[y.p->v] + 1) > var)
		var = QMDDinvorder[y.p->v] + 1;

	return (QMDDmultiply2(x, y, var));
}

QMDDedge QMDDkron(QMDDedge a, QMDDedge b)
// form Kronecker product of two QMDDs pointed to by a and b
// note Kronecker product is not commutative
		{
	QMDDedge e[MAXNEDGE], r;
	int i, j;

	if (a.p == NULL)
		return (a);
	Nop[kronecker]++;
	if (a.w == COMPLEX_ZERO) {
		return (QMDDzero);
	}
	if (QMDDterminal(a) && a.w == COMPLEX_ONE) {
		return (b);
	}
	/********************************
	 if(QMDDterminal(a))
	 {
	 r=b;
	 r.w=Cmul(b.w,a.w);
	 return(r);
	 }
	 ********************************/
	r = CTlookup(a, b, kronecker);
	if (r.p != NULL)
		return (r);

	if (a.p->ident) {
		for (i = 0; i < Radix; i++)
			for (j = 0; j < Radix; j++)
				if (i == j)
					e[i * Radix + j] = b;
				else
					e[i * Radix + j] = QMDDzero;
		r = QMDDmakeNonterminal(a.p->v, e);
		r.w = Cmul(r.w, a.w);
		CTinsert(a, b, r, kronecker);
		return (r);
	}

	for (i = 0; i < Nedge; i++)
		e[i] = QMDDkron(a.p->e[i], b);
	r = QMDDmakeNonterminal(a.p->v, e);
	r.w = Cmul(r.w, a.w);
	CTinsert(a, b, r, kronecker);
	return (r);
}

QMDDedge QMDDtranspose(QMDDedge a)
// returns a pointer to the transpose of the matrix a points to
		{
	QMDDedge r, e[MAXNEDGE];
	int i, j;

	if (a.p == NULL)
		return (a);		 // NULL pointer
	if (QMDDterminal(a) || a.p->symm)
		return (a); // terminal / or symmetric case   ADDED by Niemann Nov. 2012
	r = CTlookup(a, a, transpose);     // check in compute table
	if (r.p != NULL)
		return (r);

	for (i = 0; i < Radix; i++) // transpose submatrices and rearrange as required
		for (j = i; j < Radix; j++) {
			e[i * Radix + j] = QMDDtranspose(a.p->e[j * Radix + i]);
			if (i != j)
				e[j * Radix + i] = QMDDtranspose(a.p->e[i * Radix + j]);
		}

	r = QMDDmakeNonterminal(a.p->v, e);           // create new top vertex
	r.w = Cmul(r.w, a.w);		      // adjust top weight
	CTinsert(a, a, r, transpose);      // put in compute table
	return (r);
}

QMDDedge QMDDconjugateTranspose(QMDDedge a)
// returns a pointer to the conjugate transpose of the matrix pointed to by a
		{
	QMDDedge r, e[MAXNEDGE];
	int i, j;

	//complex c;

	if (a.p == NULL)
		return (a);		  // NULL pointer
	if (QMDDterminal(a)) 			  // terminal case
			{
		a.w = Conj(a.w);
		return (a);
	}
	r = CTlookup(a, a, conjugateTranspose);  // check if in compute table
	if (r.p != NULL)
		return (r);

	for (i = 0; i < Radix; i++)	// conjugate transpose submatrices and rearrange as required
		for (j = i; j < Radix; j++) {
			e[i * Radix + j] = QMDDconjugateTranspose(a.p->e[j * Radix + i]);
			if (i != j)
				e[j * Radix + i] = QMDDconjugateTranspose(
						a.p->e[i * Radix + j]);
		}
	r = QMDDmakeNonterminal(a.p->v, e);    // create new top node

	//Awin Zulehner (9.8.2017): BUG FIXED: Conjugate first, then mult!!

	r.w = Cmul(r.w, Conj(a.w));  // adjust top weight including conjugate

	CTinsert(a, a, r, conjugateTranspose); // put it in the compute table
	return (r);
}

QMDDedge QMDDtrace(QMDDedge a, unsigned char var, char remove[], char all)
// compute the trace or partial trace of the matrix represented by the QMDD with top edge a
// returns an edge pointing to the QMDD representing the result
// var is the index of the 'expected' variable (-1 for a terminal node)
// remove[] determines which variables are to 'removed' / traced out
// all==1 means all variables are to be removed
// CT is only used implicitly (through QMDDadd).
//
// Author: Philipp Niemann Nov. 2012
		{
	QMDDedge r, e[MAXNEDGE];
	int w = QMDDinvorder[a.p->v];

	if (QMDDedgeEqual(QMDDzero, a))
		return QMDDzero;

	if (var == (unsigned char) -1) // terminal expected
			{
		if (QMDDterminal(a))
			return (a);
		printf("Terminal expected - not found - in QMDDtrace/n");
		r.p = NULL;
		return r;
	} else { 			// nonterminal case

		if (remove[var] || all == 1) // the expected variable is to be removed
				{
			if (var == w) { 		// encounter expected variable
				r = QMDDzero;
				for (int i = 0; i < Radix; i++) {
					r = QMDDadd(r,
							QMDDtrace(a.p->e[i * Radix + i], var - 1, remove,
									all));
				}
				r.w = Cmul(r.w, a.w);
				return r;
			} else { 		// unexpected variable => skipped expected variable
				r = QMDDtrace(a, var - 1, remove, all);
				r.w = CintMul(Radix, r.w);// have to multiply by Radix to get the correct result
				return r;
			}
		} else { 			// the expected variable is to be retained
			if (var == w) {   	// encounter expected variable
				for (int i = 0; i < Radix; i++)
					for (int j = 0; j < Radix; j++)
						e[i * Radix + j] = QMDDtrace(a.p->e[i * Radix + j],
								var - 1, remove, all);
				r = QMDDmakeNonterminal(a.p->v, e);
				r.w = Cmul(r.w, a.w);
				return r;
			} else { 		// skipped variable
				r = QMDDtrace(a, var - 1, remove, all);
				return r;
			}
		}
	}
}

QMDDedge QMDDident(int x, int y)
// build a QMDD for the identity matrix for variables x to y (x<y)
		{
	int i, j, k;
	QMDDedge e, f, edge[MAXNEDGE];

	/// added by Niemann
	if (y < 0)
		return QMDDone;

	if (x == 0 && QMDDid[y].p != NULL) {
		return (QMDDid[y]);
	}
	if (y >= 1 && (f = QMDDid[y - 1]).p != NULL) {
		for (i = 0; i < Radix; i++)
			for (j = 0; j < Radix; j++)
				if (i == j)
					edge[i * Radix + j] = f;
				else
					edge[i * Radix + j] = QMDDzero;
		e = QMDDmakeNonterminal(QMDDorder[y], edge);
		QMDDid[y] = e;
		return (e);
	}
	for (i = 0; i < Radix; i++)
		for (j = 0; j < Radix; j++)
			if (i == j)
				edge[i * Radix + j] = QMDDone;
			else
				edge[i * Radix + j] = QMDDzero;
	e = QMDDmakeNonterminal(QMDDorder[x], edge);
	for (k = x + 1; k <= y; k++) {
		for (i = 0; i < Radix; i++)
			for (j = 0; j < Radix; j++)
				if (i == j)
					edge[i * Radix + j] = e;
				else
					edge[i * Radix + j] = QMDDzero;
		e = QMDDmakeNonterminal(QMDDorder[k], edge);
	}
	if (x == 0)
		QMDDid[y] = e;
	return (e);
}

QMDDedge QMDDmvlgate(QMDD_matrix mat, int n, int line[])
// build matrix representation for a single gate
// line is the vector of connections
// -1 not connected
// 0...Radix-1 indicates a control by that value
// Radix indicates the line is the target
		{

	//printf("QMDDmvlgate: %d,%d;%d,%d   %d\n", mat[0][0], mat[0][1], mat[1][0], mat[1][1], n);

	QMDDedge e, f, em[MAXNEDGE], fm[MAXNEDGE], temp;
	int i, i1, i2, w, z, j, k, t;

	for (i = 0; i < Radix; i++)
		for (j = 0; j < Radix; j++)
			em[i * Radix + j] = QMDDmakeTerminal(mat[i][j]);
	e = QMDDone;
	for (z = 0; line[w = QMDDorder[z]] < Radix; z++) //process lines below target
			{
		if (line[w] >= 0) //  control line below target in QMDD
				{
			for (i1 = 0; i1 < Radix; i1++)
				for (i2 = 0; i2 < Radix; i2++) {
					i = i1 * Radix + i2;
					if (i1 == i2)
						f = e;
					else
						f = QMDDzero;
					for (k = 0; k < Radix; k++)
						for (j = 0; j < Radix; j++) {
							t = k * Radix + j;
							if (k == j) {
								if (k == line[w]) {
									fm[t] = em[i];
								} else
									fm[t] = f;
							} else
								fm[t] = QMDDzero;
						}
					em[i] = QMDDmakeNonterminal(w, fm);
				}
		} else // not connected
		{
			for (i = 0; i < Nedge; i++) {
				for (i1 = 0; i1 < Radix; i1++)
					for (i2 = 0; i2 < Radix; i2++)
						if (i1 == i2)
							fm[i1 + i2 * Radix] = em[i];
						else
							fm[i1 + i2 * Radix] = QMDDzero;
				em[i] = QMDDmakeNonterminal(w, fm);
			}
		}
		e = QMDDident(0, z);
	}
	e = QMDDmakeNonterminal(QMDDorder[z], em);  // target line

	for (z++; z < n; z++) // go through lines above target
		if (line[w = QMDDorder[z]] >= 0) //  control line above target in QMDD
				{
			temp = QMDDident(0, z - 1);
			for (i = 0; i < Radix; i++)
				for (j = 0; j < Radix; j++)
					if (i == j) {
						if (i == line[w])
							em[i * Radix + j] = e;
						else
							em[i * Radix + j] = temp;
					} else
						em[i * Radix + j] = QMDDzero;
			e = QMDDmakeNonterminal(w, em);
		} else // not connected
		{
			for (i1 = 0; i1 < Radix; i1++)
				for (i2 = 0; i2 < Radix; i2++)
					if (i1 == i2)
						fm[i1 + i2 * Radix] = e;
					else
						fm[i1 + i2 * Radix] = QMDDzero;
			e = QMDDmakeNonterminal(w, fm);
		}
	return (e);
}

QMDDedge QMDDgate(QMDD_matrix mat, int n, int c, int t)
// for building 0 or 1 control binary gates
// c is the control variable
// t is the target variable
		{
	int line[MAXN], i;

	for (i = 0; i < n; i++)
		line[i] = -1;
	if (c >= 0)
		line[c] = Radix - 1;
	line[t] = Radix;
	return (QMDDmvlgate(mat, n, line));
}

void QMDDmatrixPrint(QMDDedge a, short v, char vtype[], std::ostream &os)
// a 0-1 matrix is printed more compactly
//
// Note: 0 entry and 1 entry in complex value table always denote
// the values 0 and 1 respectively, so it is sufficient to print the index
// for a 0-1 matrix.
//
// v is the variable index for the top vertex
		{

	uint64_t mat[MAXDIM][MAXDIM];

	int m, n, i, j, mode, p, perm;

	std::set<uint64_t> cTabPrint;
	//short cTabPrint[COMPLEXTSIZE]; // print only values of used entries of the UniqueTable   // pN 120920
	bool cTabPrintFlag = false;

//	for (int i = 0; i < COMPLEXTSIZE; i++)
//		cTabPrint[i] = 0;

	if (QMDDterminal(a))
		n = 0;
	else
		n = v + 1;
	m = 1;
	for (i = 0; i < n; i++)
		m *= Radix;
	if (n > MAXND) {
		printf("Matrix is too big to print. No. of vars=%d\n", n);
		return;
	}

//printf("DebugPN: Matrix is OK to print. No. of vars=%d. m=%d, v=%d, vtype=%s\n",n,m,v,vtype);

	//v=QMDDinvorder[v]; // convert variable index to position
	for (i = 0; i < MAXDIM; i++)
		for (j = 0; j < MAXDIM; j++)
			mat[i][j] = -1;

//	printf("DebugPN: No problems so far.");
	QMDDfillmat(mat, a, 0, 0, m, v, vtype); // convert to matrix
//	printf("DebugPN: No problems so far. Matrix filled.");

	mode = 0;
	for (i = 0; i < m; i++)          // check if 0-1 matrix (mode==1)
		for (j = 0; j < m; j++)
			if (mat[i][j] > mode) {
				mode = mat[i][j];
				if (mode > 2)
					break;
			}

	//DELETE THIS:
	mode = 3;

	perm = mode == 1; // note: assumes 0-1 matrix is a permutation matrix - should add verification

	//printf("Print matrix with Radix=%d\n", Radix);

	for (i = 0; i < m; i++)          // display matrix
			{
		for (j = 0; j < m; j++) {
			if (mode > 2 && m <= MAXDIM) {           // display complex value
				//Cprint(Cvalue(mat[i][j]), os);
				cTabPrint.insert(mat[i][j]);

				cTabPrintFlag = true;
				if (mat[i][j] < 10)
					os << " ";
				os << mat[i][j];
				os << " ";
			} else {
				if (perm && mat[i][j] == 1)    // record permutation value
						{
					p = j;
				}
				if (m <= MAXDIM)
					if (mat[i][j] == 0) {
						if (mode == 2)
							os << " ."; // print 0 as a period for clarity
						else
							os << ".";
					} else if (mat[i][j] == 1) {
						if (mode == 2)
							os << " 1";
						else
							os << "1";
					} else
						os << "-1";
			}
			if (j == m / 2 - 1)
				os << "|";
		}
		if (perm) {
			os << "  ";
			//QMDDradixPrint(p,n);   // print permutation value if permutation matrix
			os << " " << i << " " << p;
			os << " ";
			int temp = i;
			std::ostringstream temp2;
			while (temp != 0) {
				temp2 << temp % 2;
				temp = temp / 2;
			}
			std::string::reverse_iterator rit;
			for (int j = 0; j < (pow(2, n) - temp2.str().size()); j++)
				os << "0";
			for (rit = temp2.str().rbegin(); rit != temp2.str().rend(); rit++) {
				os << *rit;
			}

			os << " ";
			temp = p;
			temp2.clear();
			while (temp != 0) {
				temp2 << temp % 2;
				temp = temp / 2;
			}
			for (int j = 0; j < (pow(2, n) - temp2.str().size()); j++)
				os << "0";
			for (rit = temp2.str().rbegin(); rit != temp2.str().rend(); rit++) {
				os << *rit;
			}
		}
		os << "\n";
		if (i == m / 2 - 1) {
			for (j = 0; j < m; j++)
				os << " --";
			os << "\n";
		}
	}
	if (cTabPrintFlag) {
		os << "ComplexTable values: "; //(0): 0; (1): 1; ";

		for(std::set<uint64_t>::iterator it = cTabPrint.begin(); it != cTabPrint.end(); it++) {
			os << "(" << *it << "):";
			Cprint(*it, os);
			os << "; ";
		}
	}

	os << "\n";
}

void QMDDmatrixPrint(QMDDedge a, short v, char vtype[]) {
	std::ostringstream oss;
	QMDDmatrixPrint(a, v, vtype, oss);
	std::cout << oss.str();
}

void QMDDmatrixPrint2(QMDDedge a, std::ostream &os) {
	char v[MAXN];
	int i;

	for (i = 0; i < MAXN; i++)
		v[i] = 0;
	QMDDmatrixPrint(a, a.p->v, v, os);
}

void QMDDmatrixPrint2(QMDDedge a, std::ostream &os, short n) {
	char v[MAXN];
	int i;

	for (i = 0; i < MAXN; i++)
		v[i] = 0;
	QMDDmatrixPrint(a, n, v, os);
}

void QMDDmatrixPrint2(QMDDedge a) {
	char v[MAXN];
	int i;

	for (i = 0; i < MAXN; i++)
		v[i] = 0;
	QMDDmatrixPrint(a, QMDDinvorder[a.p->v], v);
}

void QMDDpermutationPrint(QMDDedge a)
// print permutation represented by a QMDD
// it is assumed the QMDD does represent a permutation matrix if it is a 0-1 matrix
// Max vars is MAXND with eMAXDIM set to 2^MAXND
		{
	int n, d, i;
	char fname[80], ch1;

	do {
		printf("please enter name of output file for permutation data: ");
		scanf("%s", fname);
		scanf("%c", &ch1); // discard end of line
		outfile = fopen(fname, "w"); // open the file for writing
		if (outfile == NULL)
			printf("Invalid file name, try again: ");
	} while (outfile == NULL);

	if (QMDDterminal(a))
		n = 0;
	else
		n = QMDDinvorder[a.p->v] + 1;
	d = 1;
	for (i = 0; i < n; i++)
		d *= Radix;
	QMDDpermPrint(a, 0, 0);
	fprintf(outfile, "%d\n", n);
	for (i = 0; i < d; i++) {
		fprintf(outfile, " %d", PermList[i]);
		if ((i + 1) % 16 == 0)
			fprintf(outfile, "\n");
	}
	fprintf(outfile, "\n");
	fclose(outfile);
}

int QMDDsize(QMDDedge e)
// counts number of unique nodes in a QMDD
		{
	Ncount = 0;
	return (QMDDnodeCount(e));
}

void QMDDstatistics(void)
// displays QMDD package statistics
		{
	printf("\nCurrent # nodes in unique tables: %ld\n\n", QMDDnodecount);
	printf("Total compute table lookups: %ld\n",
			CTlook[0] + CTlook[1] + CTlook[2]);
	printf("Number of ops: adds %ld mults %ld Kronecker %ld\n", Nop[add],
			Nop[mult], Nop[kronecker]);
	printf(
			"Compute table hit ratios: \naddition %ld/%ld %5.2f per cent \nmultiplication %ld/%ld %5.2f per cent \nKronecker product %ld/%ld %5.2f per cent\n",
			CThit[add], CTlook[add], (float) CThit[add] / CTlook[add] * 100,
			CThit[mult], CTlook[mult], (float) CThit[mult] / CTlook[mult] * 100,
			CThit[kronecker], CTlook[kronecker],
			(float) CThit[kronecker] / CTlook[kronecker] * 100);
	printf("UniqueTable Collisions: %ld, Matches: %ld\n", UTcol, UTmatch);

}

QMDDedge QMDDmakeColumn(int64_t c[], int first, int last, int n)
// makes a QMDD representation of a column vector
		{
	QMDDedge e[MAXNEDGE], f;
	int d, start, i;

	if (first == last) // terminal case
			{
		f = QMDDmakeTerminal(c[first]);
		return (f);
	}
	d = (last - first + 1) / Radix;
	start = first;
	for (i = 0; i < Nedge; i++) {
		if (i % Radix == 0) {
			e[i] = QMDDmakeColumn(c, start, start + d - 1, n - 1);
			start += d;
		} else {
			e[i].p = NULL;
			e[i].w = COMPLEX_ZERO;
		}
	}
	f = QMDDmakeNonterminal(QMDDorder[n - 1], e);
	return (f);
}

QMDDedge QMDDdiracket(short v, char value)
// build a column vector for variable v to
// represent |value> value = 0 or 1
		{
	QMDDedge e[MAXNEDGE], f;
	if (value == 0) {
		e[0] = QMDDone;
		e[2] = QMDDzero;
	} else {
		e[0] = QMDDzero;
		e[2] = QMDDone;
	}
	e[1].p = e[3].p = NULL;
	e[1].w = e[3].w = COMPLEX_ZERO;
	f = QMDDmakeNonterminal(v, e);
	return (f);
}

void QMDDcolumnPrint(QMDDedge p, int n)
// print a column vector represented as a QMDD
// [values]'
// n is the number of variables
// vector is printed transposed
		{
	printf(")[");
	recQMDDrcPrint(p, n - 1, 2);
	printf("]'\n");
}

QMDDedge QMDDmakeRow(uint64_t c[], int first, int last, int n)
// makes a QMDD representation of a row vector
// STILL UNDER TEST - NOT RELIABLE
		{
	QMDDedge e[MAXNEDGE], f;
	int mid;

	if (first == last) // terminal case
			{
		f = QMDDmakeTerminal(c[first]);
		return (f);
	}
	mid = (first + last) / 2;
	e[0] = QMDDmakeRow(c, first, mid, n - 1);
	e[1] = QMDDmakeRow(c, mid + 1, last, n - 1);
	e[2].p = e[3].p = NULL;
	e[2].w = e[3].w = COMPLEX_ZERO;
	f = QMDDmakeNonterminal(QMDDorder[n - 1], e);
	return (f);
}

void QMDDrowPrint(QMDDedge p, int n)
// print a row vector represented as a QMDD in form
// [values]
// n is the number of variables
		{
	printf(")[");
	recQMDDrcPrint(p, n - 1, 1);
	printf("]\n");
}

void QMDDprintActive(int n) {
// print number of active nodes for variables 0 to n-1

	printf("#printActive: %d. ", ActiveNodeCount);
	for (int i = 0; i < n; i++)
		printf(" %d ", Active[i]);
	printf("\n");
}

void throwException(const char message[], int exitCode) {
	printf(message);
	exit(exitCode);
}
