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

#include "QMDDreorder.h"

#define DEBUG_REORDER 0

int debugSift = 0;	// must be set to 0 to suppress all debug outputs
int printLimit = 100;	// number of nodes printed by debug outputs

int RenormFactorCount = 0;	// counts the number of changes to renormalization factors during sifting/reordering.

/** Documentation of computeSpecialMatricesFlag.
 *  There is a global char globalComputeSpecialMatricesFlag (defined in QMDDpackage.h) indicating the value that is to be assigned to all newly created nodes.
 *  Normally its value is 1, but during sifting it is set to 0.
 * 
 *  When computing special matrix properties in QMDDcheckSpecialMatrices, this is only done if the node flag computeSpecialMatricesFlag is set.
 * 
 *  After sifting we need to recompute the whole diagram. So we make a depth-first-traversal with QMDDmarkupSpecialMatrices to set all flags to 2.
 *  And finally we call QMDDrestoreSpecialMatrices to recompute all nodes with a flag value of 2.
 */

int siftingCostFunction(QMDDedge a){
  // cost function used during sifting (objective for minimization)
 
  return ActiveNodeCount; 					// just reduce number of nodes
  //return (ActiveNodeCount - blockMatrixCounter); 		// reduce number of nodes and get as many block matrices as possible
  //return  (ActiveNodeCount - 500 * a.p->block); 			// try to get top node as a block
  //return (ActiveNodeCount - 500 *a.p->block - (200*a.p->block*a.p->e[0].p->block) - (100*a.p->block*a.p->e[0].p->block*a.p->e[0].p->e[0].p->block));
  
}

int checkandsetBlockProperty(QMDDedge a) {
  // checks and sets the block matrix property of a QMDD. this can be done indepently of other checks (symmetry, 0/1, diagonal, etc)
    if (a.p == NULL)
      return 0;
    
    a.p->block = 0;
    
    for(int i=0; i<Radix;i++)	// if there is a non-zero weight off the diagonal, the matrix is not block
      for(int j=0; j<Radix;j++)
	if(i!=j && a.p->e[i*Radix+j].w != COMPLEX_ZERO)
	  return 0;

    a.p->block = 1;	
    return 1;   
}

int checkBlockMatrices(QMDDedge a, int triggerValue) {
  /* traverses a QMDD and returns the number of nodes representing block matrices. Sets all computeSpecialMatricesFlag to 0 to mark already traversed nodes */
  if(QMDDterminal(a)) return 0;
  
  int blockCounter;
  
  if (a.p->computeSpecialMatricesFlag == triggerValue)
    blockCounter = 0;
  else{
    blockCounter = a.p->block;
    for (int i=0; i<Nedge; i++)
      blockCounter += checkBlockMatrices(a.p->e[i], triggerValue);
    a.p->computeSpecialMatricesFlag = triggerValue;
  }
  return blockCounter;
}

void QMDDrestoreSpecialMatrices(QMDDedge a) {
  /* traverses a QMDD and restores the value of computeSpecialMatricesFlag (and recompute special properties), wherever the Flag is set to 2 */
 if (QMDDterminal(a)) return;
 
 if (a.p->computeSpecialMatricesFlag == 2){
 
 for (int i=0; i<Nedge; i++)
   QMDDrestoreSpecialMatrices(a.p->e[i]);
 
    a.p->computeSpecialMatricesFlag = globalComputeSpecialMatricesFlag;
    QMDDcheckSpecialMatrices(a);
 }
 return;
}

void QMDDmarkupSpecialMatrices(QMDDedge a) {
  /* traverses a QMDD and sets all values of computeSpecialMatricesFlag to 2 */
 if (QMDDterminal(a)) return;
 
 if (a.p->computeSpecialMatricesFlag != 2){
   
 for (int i=0; i<Nedge; i++)
   QMDDmarkupSpecialMatrices(a.p->e[i]);
 
 a.p->computeSpecialMatricesFlag = 2;
 }
 return;
}

void QMDDresetVertexWeights(QMDDedge a, uint64_t standardValue) {
 if (!QMDDterminal(a) && a.p->renormFactor != standardValue){
   for(int i=0; i<Nedge; i++) 
      QMDDresetVertexWeights(a.p->e[i], standardValue); 
    
   if(!standardValue && a.p->renormFactor != COMPLEX_ONE)
    RenormalizationNodeCount--;   
  
  a.p->renormFactor = standardValue;
  
 }
 return;
}


QMDDedge QMDDbuildIntermediate(QMDDedge a){
    
  QMDDedge r, e[MAXNEDGE];
  uint64_t weight, factor;
  
  // terminals are always OK (renormalization factor == 1)
    if (QMDDterminal(a))
      return a;
  
  // extract weight and look for renormalized node in canonic form (with incoming weight 1)
  weight = a.w;
  a.w=COMPLEX_ONE;
  
  r=CTlookup(a,a,renormalize);     // check in compute table
  
  if(r.p!=NULL){ 
    r.w = Cmul(r.w, weight);	 
    return(r); }
    
  // renormalize all children  
  for(int i=0; i<Nedge; i++) 
    e[i] = QMDDbuildIntermediate(a.p->e[i]);
  
  
  if(a.p->renormFactor != COMPLEX_ONE){
    /// extract non-trivial factor and set it to 1 in the original node.
    /// the special use of CT prevents us from getting problems when encountering the same node again!!!
    factor = a.p->renormFactor;
    a.p->renormFactor = COMPLEX_ONE;
    
    r=QMDDmakeNonterminal(a.p->v, e);
    a.p->renormFactor = factor;   // new: only change renormFactor to 1 temporarily when performing unique table lookup PN 2013/3/7
        
    r.w = Cmul(r.w, factor); // note: NEW version, only 1 multiplication!
			     // OLD version was like this:
			     //   for(int i=0; i<Nedge; i++)
			     //       e[i].w = Cmul(e[i].w,a.p->renormFactor);
    //RenormalizationNodeCount--;
    
  } else
  r=QMDDmakeNonterminal(a.p->v, e);
  
  CTinsert(a,a,r,renormalize);      // put result in compute table
  r.w = Cmul(r.w, weight);	    // don't forget the original weight which we extracted above
  
  return(r); 
}

QMDDedge QMDDrenormalize(QMDDedge a) {
 /** new routine for renormalization **/
 // build intermediate QMDD
 
 a = QMDDbuildIntermediate(a);
 
 //QMDDprintActive(QMDDinvorder[a.p->v]+1);
 
 QMDDresetVertexWeights(a, COMPLEX_ZERO);
 QMDDresetVertexWeights(a, COMPLEX_ONE);
 //RenormalizationNodeCount = 0;
 return a;
}

void QMDDchangeNonterminal(short v,QMDDedge edge[],QMDDnodeptr p)
{
// make a QMDD nonterminal node using existing space pointed to by p
  int i,redundant;
  QMDDedge e,olde;
  QMDDnode old;

  redundant=1;		// check if redundant node
  e=edge[0];
  i=1;
  while(redundant&&(i<Nedge))
  {
    //redundancy criterion: all edges (that are not NULL-edges) point to the same node with same weight
    redundant=(edge[i].p==NULL)||((edge[i].w==e.w)&&(edge[i].p==e.p));
    i++;
  }

  if(redundant) 
  {
    printf("invalid redundant node in QMDDchangeNonterminal %d\n",v);
    QMDDprint(e,25);
    exit(4); // error if redundant
  }
  
  // now focus on vertex pointed to by p
  e.p=p;
  e.w=COMPLEX_ONE;
  e.p->computeSpecialMatricesFlag = globalComputeSpecialMatricesFlag;
  
   if(checkandsetBlockProperty(e)) // decrement counter when modifying block matrix
     blockMatrixCounter--;
  
  e.p->v=v;
  memcpy(e.p->e,edge,Nedge*sizeof(QMDDedge));
  olde=e;
  memcpy(&old,e.p,sizeof(QMDDnode));	  
  
  e=QMDDnormalize(e); // normalize it, this may not change the pointer!
  if(olde.p!=e.p) printf("Normalization collapse in change nonterminal\n");
  
  if(e.w != COMPLEX_ONE) {
   // normalization factor changed! adjust renormalization factor
   if (debugSift) { printf("Debug: adjusting renormalization factor of node %ld. From ", (intptr_t) e.p); Cprint(e.p->renormFactor);}
   
    RenormFactorCount++;
   
    if (e.p->renormFactor == COMPLEX_ONE) // renormFactor is sure to become non-trivial
      RenormalizationNodeCount++;
      
    e.p->renormFactor = Cmul(e.p->renormFactor, e.w);
    if (debugSift) { printf(" to ");  Cprint(e.p->renormFactor); printf("\n");}
    e.w=COMPLEX_ONE;
    
    if (e.p->renormFactor == COMPLEX_ONE)
      RenormalizationNodeCount--;
   
    if (debugSift) printf("Number of active nodes to be renormalized: %d\n", RenormalizationNodeCount);
   //QMDDpause();
   //debugSift = 0;
  }

  olde=e;
  e=QMDDutLookup(e);  // look it up in the unique tables
  if(olde.p!=e.p) { // found copy of node in the unique table! (this shall never happen!)
    printf("??? node changed by Unique table-lookup. transfer refs from old vertex (%ld: %d) to new vertex (%ld: %d). ", (intptr_t) olde.p, olde.p->ref, (intptr_t) e.p, e.p->ref);
    QMDDpause();
    QMDDdebugnode(e.p);
    
    printf("DANGER: Don't understand that???\n");
    QMDDdebugnode(olde.p);
    QMDDprint(e,-1);
    QMDDpause();
  } 
  
  QMDDincref(e);
  QMDDdecref(olde);
  
  if(checkandsetBlockProperty(e))
    blockMatrixCounter++;
  
  return;		  // return result
}


int QMDDcheckDontCare(QMDDnodeptr p, int v2) { // checks whether node p is a dont-care for v2, i.e. whether there is a v2-child or not
  for(int i=0;i<Nedge;i++)
  {
    if(p->e[i].p->v==v2) // edge points to v2-node
    	return 0;
  }
 // no edge points to v2 
 return 1;
      
}

void QMDDswapNode(QMDDnodeptr p,int v1,int v2, int swap)
// perform variable swap for one node pointed to by p
// checked: OK! pN 120814
{
  QMDDedge table[MAXNEDGE][MAXNEDGE],e[MAXNEDGE];
  int i,j,cont;
  
  if(v1==255) printf("V1 ERROR IN SWAP\n");
  if(v2==255) printf("V2 ERROR IN SWAP\n");
  // copy info to transposition table
  cont=0;
  
  // consider all outgoing edges
  for(i=0;i<Nedge;i++)
  {
    if(QMDDterminal(p->e[i])||p->e[i].p->v!=v2) // edge points to terminal or skips a variable
    {
      for(j=0;j<Nedge;j++) 
	    table[j][i]=p->e[i];
      // what about the weights? don't have to be adjusted!
      if(p->e[i].p->v!=v2 && (!QMDDterminal(p->e[i])) && debugSift) printf("DANGER: Skipping a variable.\n");
    }
    else // "normal" edge to a vertex labeled v2, c.f. page 108 in MVL book
    {
      for(j=0;j<Nedge;j++) 
      {
        table[j][i]=p->e[i].p->e[j];
        table[j][i].w=Cmul(table[j][i].w,p->e[i].w);
	if (p->e[i].p->renormFactor != COMPLEX_ONE){
	   if(debugSift)  printf("Debug: table mult renormFactor.\n");
	   table[j][i].w=Cmul(table[j][i].w,p->e[i].p->renormFactor);  // include renormalization factors on v2-level
	}
      }
      cont=1;
    }
  }
  if(cont==0) { // v1-vertex does not point to any valid v2-vertex | This case may not happen due to preprocessing in QMDDswap
    if (debugSift) printf("No valid v2-vertex (don't-care).\n");
    printf("Encountered don't-care-node in QMDDswapnode.....illegal action.\n");
    exit(25);
    return;
  } else {
// build new sub nodes: ATTENTION first introduce new references before removing OLD ones!
for(i=0;i<Nedge;i++)
    {
      e[i]=QMDDmakeNonterminal(v1,table[i]);
      checkandsetBlockProperty(e[i]);
      QMDDincref(e[i]);
    }

for(i=0;i<Nedge;i++) // no longer count references from v1-vertex
    {
	QMDDdecref(p->e[i]);
    }

    
    
// build new top node
    QMDDchangeNonterminal(v2,e,p);		 // note replace top node / not a new node 
    Active[v1]--;				   // two lines added DMM 071010
    Active[v2]++;
  }
}

void QMDDswap(int i)
// swap variables at positions i and i-1 in the variable order
// note variable positions are numbered 0,1,2,... from bottom of QMDD

{
  int t,v1,v2;
  QMDDnodeptr table[NBUCKET],p,pnext,ptemp, plast;
  char tempLab[MAXSTRLEN]; 
  
  v1=QMDDorder[i];
  v2=QMDDorder[i-1];
  
  if (debugSift)
    printf("\nswap %d and %d (%d and %d).\n", i, i-1, v1, v2);
  
  
  
// update variable order
  t=QMDDorder[i];
  QMDDorder[i]=QMDDorder[i-1];
  QMDDorder[i-1]=t;
  QMDDinvorder[QMDDorder[i]]=i;
  QMDDinvorder[QMDDorder[i-1]]=i-1;
// swap labels
  strcpy(tempLab,Label[i]);
  strcpy(Label[i],Label[i-1]);
  strcpy(Label[i-1],tempLab);
// copy unique table for variable v1 and empty source
  for(t=0;t<NBUCKET;t++)
  {
    table[t]=Unique[v1][t];
    Unique[v1][t]=NULL;
  }
  
// process nodes one at a time

/// FIRST RUN: check for don't care nodes and insert them immediately
  for(t=0;t<NBUCKET;t++)
  {
    p=table[t];
    plast=NULL;  // pointing to the node just before p in the table-collision-chain
    while(p!=NULL)
    {
      pnext=p->next;
      if(p->ref!=0) {
	//printf("found node at slot %d. node %d\n", t, (intptr_t) p);
	if(QMDDcheckDontCare(p,v2)) { // is active don't care, so immediately put it back to Unique table
	  //printf("Debug: found don't care node  %d (does not point to %d) and reinsert it!\n", (intptr_t) p, v2);
         //printf("DC");
	//***** putting node to the front of the Unique table collision chain *****//
	ptemp=Unique[v1][t];
	Unique[v1][t] = p;
	p->next = ptemp;
	
	//***** and delete from table[] *******************************************//
	if (plast == NULL) // p was the first entry in the collision chain
	  table[t] = pnext;
	else
	  plast->next = pnext;
        } else {
	  // if not a dont-care-node, don't forget to readjust the plast pointer
	 plast = p;  // ALERT: very important!
	}
      }
      p=pnext;
    }
  }

/// SECOND RUN: modify remaining active nodes
for(t=0;t<NBUCKET;t++)
  {
    p=table[t];
    while(p!=NULL)
    {
      pnext=p->next;
      if(p->ref!=0) QMDDswapNode(p,v1,v2, i);
      else {
	// remove inactive node and mark as available
        //p->next=Avail; // WARNING: potentially buggy ;-)
        //Avail=p;
      }
      p=pnext;
    }
  }
  return;
}

int QMDDsift(int n, QMDDedge *root, QMDDrevlibDescription *circ, std::ostream &os)//, long *etimep)
// implementation of a simple sifting procedure
// n is the number of variables
// *etime is the time required for the sifting process
// returns the largest number of nodes encountered during
// the sifting
// POSSIBLE IMPROVEMENT: "closest-end-first" determine whether it's nearer to top or bottom and start sifting in that direction
{

  char free[MAXN];
  QMDDedge rootEdge;
  
  rootEdge.p = root->p;
  rootEdge.w = root->w;
    
  int i,j,k,l,max,min,largest,p, oldmin = siftingCostFunction(rootEdge), currentCost;
  int siftVariable;
  
  //doRenorm = 0; 
  globalComputeSpecialMatricesFlag = 0;
   blockMatrixCounter = checkBlockMatrices(rootEdge,0);
  
  //*etimep=cpuTime();
  largest=0;
  for(i=0;i</*n */circ->n;i++)
  {
    free[i]=1;

  }
  RenormFactorCount = 0;
  
  long otime = cpuTime();

  for(i=0;i<n;i++)
  {
    max=-1;
    if (debugSift) printf("\n(j, free, Active, max) = ");
    for(j=0;j<n;j++)
    {
     if (debugSift) printf("(%d,%d, %d, %d) - ", j, free[QMDDorder[j]], Active[QMDDorder[j]], max);
      if(free[QMDDorder[j]]&&Active[QMDDorder[j]]>max) // choose max width lowest variable for sifting from untouched ("free") variables
      {
        siftVariable=j; k=j;
        max=Active[QMDDorder[j]];
      } 
    }
if (debugSift || DEBUG_REORDER) printf("\nChoosing variable %d (%s) for sifting (%d active nodes)...\n", siftVariable, circ->line[QMDDorder[siftVariable]].variable,  max);

if(debugSift) {
QMDDprint(rootEdge, printLimit);
QMDDmatrixPrint2(rootEdge);
  //QMDDpause();
}
    min=siftingCostFunction(rootEdge); // global variable containing the total number of active nodes
    if(min>largest) largest=min;
    free[QMDDorder[k]]=0;
    p=siftVariable; // p=k; // p:= position with smallest ActiveNodeCount
    
if (debugSift) printf("Currently, we have %d active nodes in total (largest: %d)...\n", min, largest);
if (debugSift) printf("Sifting top down from position %d...", p);
    
    for(j=p;j>0;j--) // sift from position p down
    {
      if (debugSift) { printf("Order/InvOrder: ");
        for(l=0;l<n;l++) 
	  {
	    if (debugSift) printf("(%d, %d) - ", (int) QMDDorder[l], (int) QMDDinvorder[l]);
	  }
	    printf("\n");
      }
      QMDDswap(j);
      currentCost = siftingCostFunction(rootEdge);
      if (debugSift) { QMDDprint(rootEdge, printLimit); printf("CostFunction (ActiveNodes): %d  ", currentCost); }
          if (debugSift==2) { QMDDpause(); }
          
          
      if(currentCost>largest) largest=currentCost;
      if(currentCost<min)
      {
        min=currentCost;
        p=j-1; 
      }
/*  	if(GCswitch) {
  		QMDDgarbageCollect();
  	}*/

    }
if (debugSift || DEBUG_REORDER) printf("completed. Best position was %d with %d active nodes. \n", p, min);
if (debugSift) printf("Sifting bottom up...");
    for(j=1;j<n;j++) // now sift from bottom to top
    {
       if (debugSift)  if (j==n-1)  printf("\nReplacing Top Node (%d)!\n", Active[QMDDorder[j]]);
      QMDDswap(j);
      currentCost = siftingCostFunction(rootEdge);
      if (debugSift) { QMDDprint(rootEdge, printLimit); printf("Active: %d  = %d+%d+%d+%d + 1", currentCost, Active[0], Active[1], Active[2], Active[3]); }
          if (debugSift==2) { QMDDpause(); }
      if(currentCost>largest) largest=currentCost;
      if(currentCost<=min)
      {
        min=currentCost;
        p=j;
      }
/*  	if(GCswitch) {
  		QMDDgarbageCollect();
  	}*/

    }
    
if (debugSift) printf("completed. Best position was %d with %d active nodes. \n", p, min);  
//printf("Best position was %d with %d active nodes. \n", p, min); 
if (debugSift) printf("Sifting back to position %d...", p); 
for(j=n-1;j!=p;j--) {
      QMDDswap(j); // sift back to best position
      currentCost = siftingCostFunction(rootEdge);
      if (debugSift) { QMDDprint(rootEdge, printLimit); printf("Active: %d  ", currentCost); }
          if (debugSift==2) { QMDDpause(); }
  /*    	if(GCswitch) {
      		QMDDgarbageCollect();
      	}*/

    }
  
  
  //QMDDprintActive(n);    
    
/*	if(GCswitch) {
		QMDDgarbageCollect();
	}*/

if (debugSift) printf("... completed. Start sifting next variable (if any).\n");
    if(currentCost!=min) { QMDDprint(rootEdge, 1000); printf("Node count error in sifting\n"); printf("Sifting completed. CostFunction: Actual %d, Min %d, Start %d, Largest %d.\n", currentCost, min, oldmin, largest); return(currentCost); }
  }
//  *etimep=usertime()-*etimep;
otime=cpuTime()-otime;	

    //printf("CPU time is: ");
    printCPUtime(otime, os);
    char counters[20]; 
    sprintf(counters, "; %3d; %3d;", RenormFactorCount, RenormalizationNodeCount);
    os << counters; 
    
    
  QMDDinitComputeTable();
   
  if(RenormalizationNodeCount) {
   //printf("#There are %d active nodes to be renormalized. Fixing that.\n", RenormalizationNodeCount);
   if(debugSift) {
   QMDDprint(rootEdge, printLimit);
   QMDDmatrixPrint2(rootEdge);
   }
   QMDDedge temp_dd = rootEdge;
   //printf("#ActiveNodeCount before = %d. ", ActiveNodeCount);
   //QMDDprint(rootEdge, 400);
   
   rootEdge = QMDDrenormalize(rootEdge);
   //printf("after ren = %d. ", ActiveNodeCount);
   QMDDincref(rootEdge);
   //printf("after inc = %d. ", ActiveNodeCount);
   QMDDdecref(temp_dd);
   //printf("after dec = %d. \n", ActiveNodeCount);
   //QMDDpause();
   //QMDDprint(rootEdge, 400);
   root->p = rootEdge.p;
   root->w = rootEdge.w;
   if(debugSift) QMDDmatrixPrint2(rootEdge);
   if (RenormalizationNodeCount) {
    printf("ERROR: couldn't renormalize (%d nodes remaining)!", RenormalizationNodeCount);
    exit(555);
   }  
  }
 globalComputeSpecialMatricesFlag = 1; 
 QMDDmarkupSpecialMatrices(rootEdge);
 QMDDrestoreSpecialMatrices(rootEdge);
 //return ActiveNodeCount;
 return(largest);
}


int myQMDDsift(int n, QMDDedge *root, QMDDrevlibDescription *circ, std::ostream &os, int lowerbound, int upperbound)//, long *etimep)
// implementation of a simple sifting procedure
// n is the number of variables
// *etime is the time required for the sifting process
// returns the largest number of nodes encountered during
// the sifting
// POSSIBLE IMPROVEMENT: "closest-end-first" determine whether it's nearer to top or bottom and start sifting in that direction
{

  char free[MAXN];
  QMDDedge rootEdge;

  rootEdge.p = root->p;
  rootEdge.w = root->w;
  int i,j,k,l,max,min,largest,p, oldmin = siftingCostFunction(rootEdge), currentCost;
  int siftVariable;

  //doRenorm = 0;
  globalComputeSpecialMatricesFlag = 0;
   blockMatrixCounter = checkBlockMatrices(rootEdge,0);

  //*etimep=cpuTime();
  largest=0;
  for(i=0;i<n;i++)
  {
    free[i]=1;

  }
  RenormFactorCount = 0;

  long otime = cpuTime();

  for(i=lowerbound;i<upperbound;i++)
  {
    max=-1;
    if (debugSift) printf("\n(j, free, Active, max) = ");

    fflush(stdout);
    for(j=lowerbound;j<upperbound;j++)
    {
     if (debugSift) printf("(%d,%d, %d, %d) - ", j, free[QMDDorder[j]], Active[QMDDorder[j]], max);
      if(free[QMDDorder[j]]&&Active[QMDDorder[j]]>max) // choose max width lowest variable for sifting from untouched ("free") variables
      {
        siftVariable=j; k=j;
        max=Active[QMDDorder[j]];
      }
    }
if (debugSift || DEBUG_REORDER) printf("\nChoosing variable %d (%s) for sifting (%d active nodes)...\n", siftVariable, circ->line[QMDDorder[siftVariable]].variable,  max);

if(debugSift) {
QMDDprint(rootEdge, printLimit);
QMDDmatrixPrint2(rootEdge);
  //QMDDpause();
}
    min=siftingCostFunction(rootEdge); // global variable containing the total number of active nodes
    if(min>largest) largest=min;
    free[QMDDorder[k]]=0;
    p=siftVariable; // p=k; // p:= position with smallest ActiveNodeCount

if (debugSift) printf("Currently, we have %d active nodes in total (largest: %d)...\n", min, largest);
if (debugSift) printf("Sifting top down from position %d...", p);

    for(j=p;j>lowerbound;j--) // sift from position p down
    {
      if (debugSift) { printf("Order/InvOrder: ");
        for(l=lowerbound;l<upperbound;l++)
	  {
	    if (debugSift) printf("(%d, %d) - ", (int) QMDDorder[l], (int) QMDDinvorder[l]);
	  }
	    printf("\n");
      }
      QMDDswap(j);
      currentCost = siftingCostFunction(rootEdge);
      if (debugSift) { QMDDprint(rootEdge, printLimit); printf("CostFunction (ActiveNodes): %d  ", currentCost); }
          if (debugSift==2) { QMDDpause(); }


      if(currentCost>largest) largest=currentCost;
      if(currentCost<min)
      {
        min=currentCost;
        p=j-1;
      }
  	if(GCswitch) {
  		QMDDgarbageCollect();
  	}

    }
if (debugSift || DEBUG_REORDER) printf("completed. Best position was %d with %d active nodes. \n", p, min);
if (debugSift) printf("Sifting bottom up...");
    for(j=lowerbound+1;j<upperbound;j++) // now sift from bottom to top
    {
       if (debugSift)  if (j==n-1)  printf("\nReplacing Top Node (%d)!\n", Active[QMDDorder[j]]);
      QMDDswap(j);
      currentCost = siftingCostFunction(rootEdge);
      if (debugSift) { QMDDprint(rootEdge, printLimit); printf("Active: %d  = %d+%d+%d+%d + 1", currentCost, Active[0], Active[1], Active[2], Active[3]); }
          if (debugSift==2) { QMDDpause(); }
      if(currentCost>largest) largest=currentCost;
      if(currentCost<=min)
      {
        min=currentCost;
        p=j;
      }
  	if(GCswitch) {
  		QMDDgarbageCollect();
  	}

    }

if (debugSift) printf("completed. Best position was %d with %d active nodes. \n", p, min);
//printf("Best position was %d with %d active nodes. \n", p, min);
if (debugSift) printf("Sifting back to position %d...", p);
for(j=upperbound-1;j!=p;j--) {
      QMDDswap(j); // sift back to best position
      currentCost = siftingCostFunction(rootEdge);
      if (debugSift) { QMDDprint(rootEdge, printLimit); printf("Active: %d  ", currentCost); }
          if (debugSift==2) { QMDDpause(); }
      	if(GCswitch) {
      		QMDDgarbageCollect();
      	}

    }


  //QMDDprintActive(n);
//QMDDgarbageCollect();

if (debugSift) printf("... completed. Start sifting next variable (if any).\n");
    if(currentCost!=min) { QMDDprint(rootEdge, 1000); printf("Node count error in sifting\n"); printf("Sifting completed. CostFunction: Actual %d, Min %d, Start %d, Largest %d.\n", currentCost, min, oldmin, largest); return(currentCost); }
  }
//  *etimep=usertime()-*etimep;
otime=cpuTime()-otime;

    //printf("CPU time is: ");
    printCPUtime(otime, os);
    char counters[20];
    sprintf(counters, "; %3d; %3d;", RenormFactorCount, RenormalizationNodeCount);
    os << counters;


  QMDDinitComputeTable();

  if(RenormalizationNodeCount) {
   //printf("#There are %d active nodes to be renormalized. Fixing that.\n", RenormalizationNodeCount);
   if(debugSift) {
   QMDDprint(rootEdge, printLimit);
   QMDDmatrixPrint2(rootEdge);
   }
   QMDDedge temp_dd = rootEdge;
   //printf("#ActiveNodeCount before = %d. ", ActiveNodeCount);
   //QMDDprint(rootEdge, 400);

   rootEdge = QMDDrenormalize(rootEdge);
   //printf("after ren = %d. ", ActiveNodeCount);
   QMDDincref(rootEdge);
   //printf("after inc = %d. ", ActiveNodeCount);
   QMDDdecref(temp_dd);
   //printf("after dec = %d. \n", ActiveNodeCount);
   //QMDDpause();
   //QMDDprint(rootEdge, 400);
   root->p = rootEdge.p;
   root->w = rootEdge.w;
   if(debugSift) QMDDmatrixPrint2(rootEdge);
   if (RenormalizationNodeCount) {
    printf("ERROR: couldn't renormalize (%d nodes remaining)!", RenormalizationNodeCount);
    exit(555);
   }
  }
 globalComputeSpecialMatricesFlag = 1;
 QMDDmarkupSpecialMatrices(rootEdge);
 QMDDrestoreSpecialMatrices(rootEdge);
 //return ActiveNodeCount;
 return(largest);
}



int QMDDsift(int n, QMDDedge *root, QMDDrevlibDescription *circ)//, long *etimep)
{
    std::ostringstream oss;
	int r = QMDDsift(n, root, circ, oss);
	printf("#CPU Time / #renormFactors / #renormFactors at the end: ");
	oss << std::endl;
	std::cout << oss.str();  
	
	if (ActiveNodeCount<DYNREORDERLIMIT)  // update dyn reorder treshold
	  dynamicReorderingTreshold=DYNREORDERLIMIT;
	else
	  dynamicReorderingTreshold=ActiveNodeCount;  
	
 return r;
}


int lookupLabel(char buffer[], char moveLabel[], QMDDrevlibDescription *circ) {
// returns the line index of the label found in buffer or -1 if no match was found  

  int k = 0;
  char ch;
  
  while(buffer[k]!=' '&&buffer[k]!='\n' && k < 3)
  {
    ch = buffer[k];
    if(ch>='a'&&ch<='z') ch=ch-'a'+'A'; // convert lowercase letters to uppercase
    moveLabel[k] = ch;
    k++;
  }
  moveLabel[k]=0;
  if (!k){
    printf("No valid label found. \n"); return -1;
  } else {
    k=-1;
    for(int i=0;i<circ->n;i++) 			// lookup up label in table
    {
      if(0==strcmp(moveLabel,circ->line[i].variable)) 
      {
	k=i; // return if found
      }
    }
    
  }
  return k;
}

void QMDDreorder(int order[],int n, QMDDedge *root)
// reorder variables to given order
// can be used for verification of sifting
{
  QMDDedge rootEdge;
  
  rootEdge.p = root->p;
  rootEdge.w = root->w;
  QMDDedge temp_dd;
  
  int i,j;
  
  if (order[0] == -1) {
  for(i=0;i<n+1;i++)
    order[i] = i;
  }
  
  globalComputeSpecialMatricesFlag = 0; 
 
//  QMDDprintActive(n);

  for(i=0;i<n-1;i++)
  {
//    printf("\nreordering %d\n",i);
    for(j=i;order[i]!=QMDDorder[j];j++);
#if DEBUG_REORDER
    printf("%d is on level %d. pulling down. ", i, j);
#endif
    if(j>n-1) printf("error in reorder\n");
    for(;j>i;j--) { 
      //printf(" %d ",j);
      QMDDswap(j);}
#if DEBUG_REORDER
   printf("Active Nodes: %d, Nodes: %d\n", ActiveNodeCount, QMDDnodecount);
#endif
   if(GCswitch) {
    	QMDDgarbageCollect();
    }

//    QMDDprint(rootEdge, printLimit);
    //QMDDpause();
  }
//  QMDDprintActive(n);
  //QMDDpause();
//  printf("Done.\n");
  
  
  QMDDinitComputeTable();
  
  if(RenormalizationNodeCount) {
   printf("#There are %d active nodes to be renormalized. Fixing that.\n", RenormalizationNodeCount);

   temp_dd = rootEdge;
   rootEdge = QMDDrenormalize(rootEdge);
 
   QMDDincref(rootEdge);
   QMDDdecref(temp_dd);
   
  }

  root->p = rootEdge.p;
  root->w = rootEdge.w;
  
  
  
 globalComputeSpecialMatricesFlag = 1; 
 QMDDmarkupSpecialMatrices(rootEdge);
 QMDDrestoreSpecialMatrices(rootEdge);
 
 //return (root);
}






typedef enum{TOP, BOTTOM, UP, DOWN} moveType; // move variable kinds 


int QMDDmoveVariable(QMDDedge *basic, char buffer[], QMDDrevlibDescription *circ){
/**  call is 'move{up/down/top/bottom} VAR'. 
 *   moves variable to the respective position.
 *   returns 1 if the variable order is changed and 0 otherwise.
 */ 
  int k, order[MAXN], bufferOffset, p, q=1, i;
  moveType moveDirection;
  char moveLabel[3];
  
  // look for moving direction
  
  if(strncmp("down", buffer,4) == 0) {
    moveDirection = DOWN;
    bufferOffset = 4;
  } else if(strncmp("up", buffer,2) == 0) {
    moveDirection = UP;
    bufferOffset = 2;
    
  } else if(strncmp("top", buffer,3) == 0) {
    moveDirection = TOP;
    bufferOffset = 3;
    q=circ->n;
  } else if(strncmp("bottom", buffer,6) == 0) {
    moveDirection = BOTTOM;
    bufferOffset = 6;
    q=circ->n;
  } else {
    printf("No valid direction (up/down/top/bottom) recognized.\n");
    return 0;
  }
  bufferOffset++;
  
  k = lookupLabel(&buffer[bufferOffset], moveLabel, circ);
  if (k == -1) {
    printf("Invalid label %s.\n", moveLabel); return 0;
  }
  printf("Label %s found at index %d. ", moveLabel, k);
  
  p = QMDDinvorder[k];		  
  
  if (moveDirection == UP || moveDirection == TOP) {
    if(p==circ->n-1) {
      printf("Already at the top! \n");
      return 0;
    }
    for (i=0; i<p; i++)		// don't touch variables 0..p-1
      order[i]=QMDDorder[i];
    for (; i<p+q && i<circ->n; i++)     // move down all variables p+1 ... p+q
      order[i]=QMDDorder[i+1];
    order[i++] = k;			// insert variable at position p+q
    for (; i<circ->n; i++)		// don't touch variables p+q+1...n
      order[i]=QMDDorder[i];
  }
  
  if (moveDirection == DOWN || moveDirection == BOTTOM) {
    if(p==0) {
      printf("Already at the bottom! \n");
      return 0;
    }
    for (i=0; i<p-q; i++)		// don't touch variables 0..p-q-1
      order[i]=QMDDorder[i];
    order[i++] = k;			// insert variable at position p-q
    for (; i<=p; i++)     		// move up all variables p-q+1 to p-1
      order[i]=QMDDorder[i-1];
    for (; i<circ->n; i++)		// don't touch variables p+1...n
      order[i]=QMDDorder[i];
  }
 QMDDreorder(order, circ->n, basic);      
 return 1;
}

void SJTalgorithm(QMDDedge a, int n){
  /* Steinhaus-Johnson-Trotter algorithm to generate all permutations by a minimal number of adjacent swaps */
  /* uses pseudocode description from: 
   * 	Combinatorial Algorithms:Theory and Practice
	Edward M. Reingold, Jurg Nievergelt and Narsingh Deo
	Prentice-Hall 1977 ISBN 0-13-152447-X
   */
  
 globalComputeSpecialMatricesFlag = 0; 
  
 int perm[MAXN+2], invperm[MAXN+2], dir[MAXN+2];
 int m, temp, cost;
 int min, max;
 int printFlag;
 
 min=max=siftingCostFunction(a);
  
  for (int i=1; i<= n; i++){
   perm[i] = invperm[i] = i;
   dir[i] = -1;
  }
  
  dir[1] = 0;
  perm[0] = perm[n+1] = m = n+1;
    
  while (m != 1) {
    
    printFlag = 0;
    cost = siftingCostFunction(a);
    if (cost<min)
    printFlag = min = cost;
    else if (cost > max)
    printFlag = max = cost;
    
    // Output permutation ( if new min or max)
    for (int i=1; printFlag && i<= n; i++)
	printf("%d (%ld), ", perm[i], QMDDorder[i-1]);
    
    m=n;
    
    while(perm[invperm[m]+dir[m]] > m) {
      dir[m] = -dir[m];
      m = m-1;
    }

    /* This part is for swapping variables in the QMDD and printing/keeping track of the cost function values */
    // Cost function
    cost = siftingCostFunction(a);
    if (printFlag)
      printf("%d \n", cost);
 
    
    
    
    if (dir[m] == -1)
	QMDDswap(invperm[m]-1);
    else if(m != 1)
        QMDDswap(invperm[m]);
    else
        QMDDswap(1);
    /* END */
    
    temp = perm[invperm[m]];
    perm[invperm[m]] = perm[invperm[m]+dir[m]];
    perm[invperm[m]+dir[m]] = temp;
   
    temp = invperm[perm[invperm[m]]];
    invperm[perm[invperm[m]]] = invperm[m];
    invperm[m] = temp;
  }
    
    for (int i=1; i<= n; i++)
	printf("%d (%ld), ", perm[i], QMDDorder[i-1]);
    printf("%d \n", siftingCostFunction(a));
  
    QMDDinitComputeTable();
  
  if(RenormalizationNodeCount) {
   printf("There are %d active nodes to be renormalized. Should fix that!\n", RenormalizationNodeCount);
   a = QMDDrenormalize(a);
 }
  
 globalComputeSpecialMatricesFlag = 1; 
 QMDDmarkupSpecialMatrices(a);
 QMDDrestoreSpecialMatrices(a);
  
  printf("Cost Function: initial = %d, min = %d, max = %d.\n", cost,min,max);
  //QMDDprint(a,200);
  
  
}
