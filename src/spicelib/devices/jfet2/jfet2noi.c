/**********
based on jfetnoi.c
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Gary W. Ng

Modified to jfet2 for PS model definition ( Anthony E. Parker )
   Copyright 1994  Macquarie University, Sydney Australia.
**********/

#include "ngspice/ngspice.h"
#include "jfet2defs.h"
#include "ngspice/cktdefs.h"
#include "ngspice/iferrmsg.h"
#include "ngspice/noisedef.h"
#include "ngspice/suffix.h"

/*
 * JFET2noise (mode, operation, firstModel, ckt, data, OnDens)
 *    This routine names and evaluates all of the noise sources
 *    associated with JFET2's.  It starts with the model *firstModel and
 *    traverses all of its insts.  It then proceeds to any other models
 *    on the linked list.  The total output noise density generated by
 *    all of the JFET2's is summed with the variable "OnDens".
 */


int
JFET2noise (int mode, int operation, GENmodel *genmodel, CKTcircuit *ckt, Ndata *data, double *OnDens)
{
    NOISEAN *job = (NOISEAN *) ckt->CKTcurJob;

    JFET2model *firstModel = (JFET2model *) genmodel;
    JFET2model *model;
    JFET2instance *inst;
    char name[N_MXVLNTH];
    double tempOnoise;
    double tempInoise;
    double noizDens[JFET2NSRCS];
    double lnNdens[JFET2NSRCS];
    int i;

    /* define the names of the noise sources */

    static char *JFET2nNames[JFET2NSRCS] = {
	/* Note that we have to keep the order consistent with the
	  strchr definitions in JFET2defs.h */
	"_rd",              /* noise due to rd */
	"_rs",              /* noise due to rs */
	"_id",              /* noise due to id */
	"_1overf",          /* flicker (1/f) noise */
	""                  /* total transistor noise */
    };

    for (model=firstModel; model != NULL; model=model->JFET2nextModel) {
	for (inst=model->JFET2instances; inst != NULL; inst=inst->JFET2nextInstance) {

	    switch (operation) {

	    case N_OPEN:

		/* see if we have to to produce a summary report */
		/* if so, name all the noise generators */

		if (job->NStpsSm != 0) {
		    switch (mode) {

		    case N_DENS:
			for (i=0; i < JFET2NSRCS; i++) {
			    (void)sprintf(name,"onoise_%s%s",inst->JFET2name,JFET2nNames[i]);


data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
if (!data->namelist) return(E_NOMEM);
		SPfrontEnd->IFnewUid (ckt,
			&(data->namelist[data->numPlots++]),
			NULL, name, UID_OTHER, NULL);
				/* we've added one more plot */


			}
			break;

		    case INT_NOIZ:
			for (i=0; i < JFET2NSRCS; i++) {
			    (void)sprintf(name,"onoise_total_%s%s",inst->JFET2name,JFET2nNames[i]);


data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
if (!data->namelist) return(E_NOMEM);
		SPfrontEnd->IFnewUid (ckt,
			&(data->namelist[data->numPlots++]),
			NULL, name, UID_OTHER, NULL);
				/* we've added one more plot */


			    (void)sprintf(name,"inoise_total_%s%s",inst->JFET2name,JFET2nNames[i]);


data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
if (!data->namelist) return(E_NOMEM);
		SPfrontEnd->IFnewUid (ckt,
			&(data->namelist[data->numPlots++]),
			NULL, name, UID_OTHER, NULL);
				/* we've added one more plot */

			}
			break;
		    }
		}
		break;

	    case N_CALC:
		switch (mode) {

		case N_DENS:
		    NevalSrc(&noizDens[JFET2RDNOIZ],&lnNdens[JFET2RDNOIZ],
				 ckt,THERMNOISE,inst->JFET2drainPrimeNode,inst->JFET2drainNode,
				 model->JFET2drainConduct * inst->JFET2area * inst->JFET2m);

		    NevalSrc(&noizDens[JFET2RSNOIZ],&lnNdens[JFET2RSNOIZ],
				 ckt,THERMNOISE,inst->JFET2sourcePrimeNode,
				 inst->JFET2sourceNode,model->JFET2sourceConduct
                                 * inst->JFET2area * inst->JFET2m);

		    NevalSrc(&noizDens[JFET2IDNOIZ],&lnNdens[JFET2IDNOIZ],
				 ckt,THERMNOISE,inst->JFET2drainPrimeNode,
				 inst->JFET2sourcePrimeNode,
				 (2.0/3.0 * inst->JFET2m * fabs(*(ckt->CKTstate0 + inst->JFET2gm))));

		    NevalSrc(&noizDens[JFET2FLNOIZ], NULL, ckt,
				 N_GAIN,inst->JFET2drainPrimeNode,
				 inst->JFET2sourcePrimeNode, (double)0.0);
		    noizDens[JFET2FLNOIZ] *= inst->JFET2m * model->JFET2fNcoef * 
				 exp(model->JFET2fNexp *
				 log(MAX(fabs(*(ckt->CKTstate0 + inst->JFET2cd)),N_MINLOG))) /
				 data->freq;
		    lnNdens[JFET2FLNOIZ] = 
				 log(MAX(noizDens[JFET2FLNOIZ],N_MINLOG));

		    noizDens[JFET2TOTNOIZ] = noizDens[JFET2RDNOIZ] +
						     noizDens[JFET2RSNOIZ] +
						     noizDens[JFET2IDNOIZ] +
						     noizDens[JFET2FLNOIZ];
		    lnNdens[JFET2TOTNOIZ] = 
				 log(MAX(noizDens[JFET2TOTNOIZ], N_MINLOG));

		    *OnDens += noizDens[JFET2TOTNOIZ];

		    if (data->delFreq == 0.0) { 

			/* if we haven't done any previous integration, we need to */
			/* initialize our "history" variables                      */

			for (i=0; i < JFET2NSRCS; i++) {
			    inst->JFET2nVar[LNLSTDENS][i] = lnNdens[i];
			}

			/* clear out our integration variables if it's the first pass */

			if (data->freq == job->NstartFreq) {
			    for (i=0; i < JFET2NSRCS; i++) {
				inst->JFET2nVar[OUTNOIZ][i] = 0.0;
				inst->JFET2nVar[INNOIZ][i] = 0.0;
			    }
			}
		    } else {   /* data->delFreq != 0.0 (we have to integrate) */
			for (i=0; i < JFET2NSRCS; i++) {
			    if (i != JFET2TOTNOIZ) {
				tempOnoise = Nintegrate(noizDens[i], lnNdens[i],
				      inst->JFET2nVar[LNLSTDENS][i], data);
				tempInoise = Nintegrate(noizDens[i] * data->GainSqInv ,
				      lnNdens[i] + data->lnGainInv,
				      inst->JFET2nVar[LNLSTDENS][i] + data->lnGainInv,
				      data);
				inst->JFET2nVar[LNLSTDENS][i] = lnNdens[i];
				data->outNoiz += tempOnoise;
				data->inNoise += tempInoise;
				if (job->NStpsSm != 0) {
				    inst->JFET2nVar[OUTNOIZ][i] += tempOnoise;
				    inst->JFET2nVar[OUTNOIZ][JFET2TOTNOIZ] += tempOnoise;
				    inst->JFET2nVar[INNOIZ][i] += tempInoise;
				    inst->JFET2nVar[INNOIZ][JFET2TOTNOIZ] += tempInoise;
                                }
			    }
			}
		    }
		    if (data->prtSummary) {
			for (i=0; i < JFET2NSRCS; i++) {     /* print a summary report */
			    data->outpVector[data->outNumber++] = noizDens[i];
			}
		    }
		    break;

		case INT_NOIZ:        /* already calculated, just output */
		    if (job->NStpsSm != 0) {
			for (i=0; i < JFET2NSRCS; i++) {
			    data->outpVector[data->outNumber++] = inst->JFET2nVar[OUTNOIZ][i];
			    data->outpVector[data->outNumber++] = inst->JFET2nVar[INNOIZ][i];
			}
		    }    /* if */
		    break;
		}    /* switch (mode) */
		break;

	    case N_CLOSE:
		return (OK);         /* do nothing, the main calling routine will close */
		break;               /* the plots */
	    }    /* switch (operation) */
	}    /* for inst */
    }    /* for model */

return(OK);
}
            

