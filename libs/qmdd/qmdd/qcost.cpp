//*************************************************************************
// Computes gate cost based on the cost table on
// Dmitri Maslov's page http://webhome.cs.uvic.ca/~dmaslov/definitions.html
// matches the cost function in QUIVER
//
#include "qcost.h"

int gate_qcost(int size, int n,int kind)
{
  int avail,cost;

    if(kind==PERES_GATE||kind==INV_PERES_GATE) return(4);
      avail=n-size+ANCILLARY;
        if(size==1) cost=1;
	  else if(size==2) cost=1;
	    else if(size==3) cost=5;
	      else if(size==4) cost=13;
	        else if(size==5)
		  {
		      if(avail>=2) cost=26;
		          else cost=29;
			    }
			      else if(size==6)
			        {
				    if(avail>=3) cost=38;
				        else if(avail>=1) cost=52;
					    else cost=61;
					      }
					        else if(size==7)
						  {
						      if(avail>=4) cost=50;
						          else if(avail>=1) cost=80;
							      else cost = 125;
							        }
								  else if(size==8)
								    {
								        if(avail>=5) cost=62;
									    else if(avail>=1) cost=100;
									        else cost=253;
										  }
										    else if(size==9)
										      {
										          if(avail>=6) cost=74;
											      else if(avail>=1) cost=128;
											          else cost=509;
												    }
												      else if(size==10)
												        {
													    if(avail>=7) cost=86;
													        else if(avail>=1) cost=152;
														    else cost=1021;
														      }
														        else /* size>10 */
															  {
															      if(avail>=size-3) cost=12*size-34;
															          else if(avail>=1) cost=24*size-88;
																      else cost=(1<<size)-3;
																        }
																	  return(cost);
																	  }


