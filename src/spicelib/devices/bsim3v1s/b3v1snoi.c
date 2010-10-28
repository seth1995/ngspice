/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1995 Gary W. Ng and Min-Chie Jeng.
Modified by Paolo Nenzi 2002
File:  b3v1snoi.c
**********/

#include "ngspice.h"
#include "bsim3v1sdef.h"
#include "cktdefs.h"
#include "iferrmsg.h"
#include "noisedef.h"
#include "suffix.h"
#include "const.h"  /* jwan */

/*
 * BSIM3v1Snoise (mode, operation, firstModel, ckt, data, OnDens)
 *    This routine names and evaluates all of the noise sources
 *    associated with MOSFET's.  It starts with the model *firstModel and
 *    traverses all of its insts.  It then proceeds to any other models
 *    on the linked list.  The total output noise density generated by
 *    all of the MOSFET's is summed with the variable "OnDens".
 */

/*
 Channel thermal and flicker noises are calculated based on the value
 of model->BSIM3v1SnoiMod.
 If model->BSIM3v1SnoiMod = 1,
    Channel thermal noise = SPICE2 model
    Flicker noise         = SPICE2 model
 If model->BSIM3v1SnoiMod = 2,
    Channel thermal noise = BSIM3v1S model
    Flicker noise         = BSIM3v1S model
 If model->BSIM3v1SnoiMod = 3,
    Channel thermal noise = SPICE2 model
    Flicker noise         = BSIM3v1S model
 If model->BSIM3v1SnoiMod = 4,
    Channel thermal noise = BSIM3v1S model
    Flicker noise         = SPICE2 model
 */


static double
StrongInversionNoiseEval_b3v1s(double vgs, double vds, BSIM3v1Smodel *model, 
                         BSIM3v1Sinstance *here, double freq, double temp)
{
struct bsim3v1sSizeDependParam *pParam;
double cd, esat, DelClm, EffFreq, N0, Nl, Vgst;
double T0, T1, T2, T3, T4, T5, T6, T7, T8, T9;
double Ssi;

    pParam = here->pParam;
    cd = fabs(here->BSIM3v1Scd);
    if (vds > here->BSIM3v1Svdsat)
    {   esat = 2.0 * pParam->BSIM3v1Svsattemp / here->BSIM3v1Sueff;
	T0 = ((((vds - here->BSIM3v1Svdsat) / pParam->BSIM3v1Slitl) + model->BSIM3v1Sem)
	   / esat);
        DelClm = pParam->BSIM3v1Slitl * log (MAX(T0, N_MINLOG));
    }
    else 
        DelClm = 0.0;
    EffFreq = pow(freq, model->BSIM3v1Sef);
    T1 = CHARGE * CHARGE * 8.62e-5 * cd * temp * here->BSIM3v1Sueff;
    T2 = 1.0e8 * EffFreq * model->BSIM3v1Scox
       * pParam->BSIM3v1Sleff * pParam->BSIM3v1Sleff;
    Vgst = vgs - here->BSIM3v1Svon;
    N0 = model->BSIM3v1Scox * Vgst / CHARGE;
    if (N0 < 0.0)
	N0 = 0.0;
    Nl = model->BSIM3v1Scox * (Vgst - MIN(vds, here->BSIM3v1Svdsat)) / CHARGE;
    if (Nl < 0.0)
	Nl = 0.0;

    T3 = model->BSIM3v1SoxideTrapDensityA
       * log(MAX(((N0 + 2.0e14) / (Nl + 2.0e14)), N_MINLOG));
    T4 = model->BSIM3v1SoxideTrapDensityB * (N0 - Nl);
    T5 = model->BSIM3v1SoxideTrapDensityC * 0.5 * (N0 * N0 - Nl * Nl);

    T6 = 8.62e-5 * temp * cd * cd;
    T7 = 1.0e8 * EffFreq * pParam->BSIM3v1Sleff
       * pParam->BSIM3v1Sleff * pParam->BSIM3v1Sweff;
    T8 = model->BSIM3v1SoxideTrapDensityA + model->BSIM3v1SoxideTrapDensityB * Nl
       + model->BSIM3v1SoxideTrapDensityC * Nl * Nl;
    T9 = (Nl + 2.0e14) * (Nl + 2.0e14);

    Ssi = T1 / T2 * (T3 + T4 + T5) + T6 / T7 * DelClm * T8 / T9;
    return Ssi;
}

int
BSIM3v1Snoise (int mode, int operation, GENmodel *inModel, CKTcircuit *ckt, 
               Ndata *data, double *OnDens)
{
BSIM3v1Smodel *model = (BSIM3v1Smodel *)inModel;
BSIM3v1Sinstance *here;
struct bsim3v1sSizeDependParam *pParam;
char name[N_MXVLNTH];
double tempOnoise;
double tempInoise;
double noizDens[BSIM3v1SNSRCS];
double lnNdens[BSIM3v1SNSRCS];

double vgs, vds, Slimit;
double T1, T10, T11;
double Ssi, Swi;

int i;

    /* define the names of the noise sources */
    static char *BSIM3v1SnNames[BSIM3v1SNSRCS] =
    {   /* Note that we have to keep the order */
	".rd",              /* noise due to rd */
			    /* consistent with the index definitions */
	".rs",              /* noise due to rs */
			    /* in BSIM3v1Sdefs.h */
	".id",              /* noise due to id */
	".1overf",          /* flicker (1/f) noise */
	""                  /* total transistor noise */
    };

    for (; model != NULL; model = model->BSIM3v1SnextModel)
    {    for (here = model->BSIM3v1Sinstances; here != NULL;
	      here = here->BSIM3v1SnextInstance)
	 {    pParam = here->pParam;
	      switch (operation)
	      {  case N_OPEN:
		     /* see if we have to to produce a summary report */
		     /* if so, name all the noise generators */

		      if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0)
		      {   switch (mode)
			  {  case N_DENS:
			          for (i = 0; i < BSIM3v1SNSRCS; i++)
				  {    (void) sprintf(name, "onoise.%s%s",
					              here->BSIM3v1Sname,
						      BSIM3v1SnNames[i]);
                                       data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       (*(SPfrontEnd->IFnewUid)) (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  (IFuid) NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */
			          }
			          break;
		             case INT_NOIZ:
			          for (i = 0; i < BSIM3v1SNSRCS; i++)
				  {    (void) sprintf(name, "onoise_total.%s%s",
						      here->BSIM3v1Sname,
						      BSIM3v1SnNames[i]);
                                       data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       (*(SPfrontEnd->IFnewUid)) (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  (IFuid) NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */

			               (void) sprintf(name, "inoise_total.%s%s",
						      here->BSIM3v1Sname,
						      BSIM3v1SnNames[i]);
                                       data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       (*(SPfrontEnd->IFnewUid)) (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  (IFuid) NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */
			          }
			          break;
		          }
		      }
		      break;
	         case N_CALC:
		      switch (mode)
		      {  case N_DENS:
		              NevalSrc(&noizDens[BSIM3v1SRDNOIZ],
				       &lnNdens[BSIM3v1SRDNOIZ], ckt, THERMNOISE,
				       here->BSIM3v1SdNodePrime, here->BSIM3v1SdNode,
				       here->BSIM3v1SdrainConductance);

		              NevalSrc(&noizDens[BSIM3v1SRSNOIZ],
				       &lnNdens[BSIM3v1SRSNOIZ], ckt, THERMNOISE,
				       here->BSIM3v1SsNodePrime, here->BSIM3v1SsNode,
				       here->BSIM3v1SsourceConductance);

                              switch( model->BSIM3v1SnoiMod )
			      {  case 1:
			         case 3:
			              NevalSrc(&noizDens[BSIM3v1SIDNOIZ],
				               &lnNdens[BSIM3v1SIDNOIZ], ckt, 
					       THERMNOISE, here->BSIM3v1SdNodePrime,
				               here->BSIM3v1SsNodePrime,
                                               (2.0 / 3.0 * fabs(here->BSIM3v1Sgm
				               + here->BSIM3v1Sgds
					       + here->BSIM3v1Sgmbs)));
				      break;
			         case 2:
			         case 4:
		                      NevalSrc(&noizDens[BSIM3v1SIDNOIZ],
				               &lnNdens[BSIM3v1SIDNOIZ], ckt,
					       THERMNOISE, here->BSIM3v1SdNodePrime,
                                               here->BSIM3v1SsNodePrime,
					       (here->BSIM3v1Sueff
					       * fabs(here->BSIM3v1Sqinv
					       / (pParam->BSIM3v1Sleff
					       * pParam->BSIM3v1Sleff))));
				      break;
			      }
		              NevalSrc(&noizDens[BSIM3v1SFLNOIZ], (double*) NULL,
				       ckt, N_GAIN, here->BSIM3v1SdNodePrime,
				       here->BSIM3v1SsNodePrime, (double) 0.0);

                              switch( model->BSIM3v1SnoiMod )
			      {  case 1:
			         case 4:
			              noizDens[BSIM3v1SFLNOIZ] *= model->BSIM3v1Skf
					    * exp(model->BSIM3v1Saf
					    * log(MAX(fabs(here->BSIM3v1Scd),
					    N_MINLOG)))
					    / (pow(data->freq, model->BSIM3v1Sef)
					    * pParam->BSIM3v1Sleff
				            * pParam->BSIM3v1Sleff
					    * model->BSIM3v1Scox);
				      break;
			         case 2:
			         case 3:
			              vgs = *(ckt->CKTstates[0] + here->BSIM3v1Svgs);
		                      vds = *(ckt->CKTstates[0] + here->BSIM3v1Svds);
			              if (vds < 0.0)
			              {   vds = -vds;
				          vgs = vgs + vds;
			              }
                                      if (vgs >= here->BSIM3v1Svon + 0.1)
			              {   Ssi = StrongInversionNoiseEval_b3v1s(vgs,
					      vds, model, here, data->freq,
					      ckt->CKTtemp);
                                          noizDens[BSIM3v1SFLNOIZ] *= Ssi;
			              }
                                      else 
			              {   pParam = here->pParam;
				          T10 = model->BSIM3v1SoxideTrapDensityA
					      * 8.62e-5 * ckt->CKTtemp;
		                          T11 = pParam->BSIM3v1Sweff
					      * pParam->BSIM3v1Sleff
				              * pow(data->freq, model->BSIM3v1Sef)
				              * 4.0e36;
		                          Swi = T10 / T11 * here->BSIM3v1Scd
				              * here->BSIM3v1Scd;
                                          Slimit = StrongInversionNoiseEval_b3v1s(
				               here->BSIM3v1Svon + 0.1, vds, model,
					       here, data->freq, ckt->CKTtemp);
				          T1 = Swi + Slimit;
				          if (T1 > 0.0)
                                              noizDens[BSIM3v1SFLNOIZ] *= (Slimit
								    * Swi) / T1; 
				          else
                                              noizDens[BSIM3v1SFLNOIZ] *= 0.0;
			              }
				      break;
			      }

		              lnNdens[BSIM3v1SFLNOIZ] =
				     log(MAX(noizDens[BSIM3v1SFLNOIZ], N_MINLOG));

		              noizDens[BSIM3v1STOTNOIZ] = noizDens[BSIM3v1SRDNOIZ]
						     + noizDens[BSIM3v1SRSNOIZ]
						     + noizDens[BSIM3v1SIDNOIZ]
						     + noizDens[BSIM3v1SFLNOIZ];
		              lnNdens[BSIM3v1STOTNOIZ] = 
				     log(MAX(noizDens[BSIM3v1STOTNOIZ], N_MINLOG));

		              *OnDens += noizDens[BSIM3v1STOTNOIZ];

		              if (data->delFreq == 0.0)
			      {   /* if we haven't done any previous 
				     integration, we need to initialize our
				     "history" variables.
				    */

			          for (i = 0; i < BSIM3v1SNSRCS; i++)
				  {    here->BSIM3v1SnVar[LNLSTDENS][i] =
					     lnNdens[i];
			          }

			          /* clear out our integration variables
				     if it's the first pass
				   */
			          if (data->freq ==
				      ((NOISEAN*) ckt->CKTcurJob)->NstartFreq)
				  {   for (i = 0; i < BSIM3v1SNSRCS; i++)
				      {    here->BSIM3v1SnVar[OUTNOIZ][i] = 0.0;
				           here->BSIM3v1SnVar[INNOIZ][i] = 0.0;
			              }
			          }
		              }
			      else
			      {   /* data->delFreq != 0.0,
				     we have to integrate.
				   */
			          for (i = 0; i < BSIM3v1SNSRCS; i++)
				  {    if (i != BSIM3v1STOTNOIZ)
				       {   tempOnoise = Nintegrate(noizDens[i],
						lnNdens[i],
				                here->BSIM3v1SnVar[LNLSTDENS][i],
						data);
				           tempInoise = Nintegrate(noizDens[i]
						* data->GainSqInv, lnNdens[i]
						+ data->lnGainInv,
				                here->BSIM3v1SnVar[LNLSTDENS][i]
						+ data->lnGainInv, data);
				           here->BSIM3v1SnVar[LNLSTDENS][i] =
						lnNdens[i];
				           data->outNoiz += tempOnoise;
				           data->inNoise += tempInoise;
				           if (((NOISEAN*)
					       ckt->CKTcurJob)->NStpsSm != 0)
					   {   here->BSIM3v1SnVar[OUTNOIZ][i]
						     += tempOnoise;
				               here->BSIM3v1SnVar[OUTNOIZ][BSIM3v1STOTNOIZ]
						     += tempOnoise;
				               here->BSIM3v1SnVar[INNOIZ][i]
						     += tempInoise;
				               here->BSIM3v1SnVar[INNOIZ][BSIM3v1STOTNOIZ]
						     += tempInoise;
                                           }
			               }
			          }
		              }
		              if (data->prtSummary)
			      {   for (i = 0; i < BSIM3v1SNSRCS; i++)
				  {    /* print a summary report */
			               data->outpVector[data->outNumber++]
					     = noizDens[i];
			          }
		              }
		              break;
		         case INT_NOIZ:
			      /* already calculated, just output */
		              if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0)
			      {   for (i = 0; i < BSIM3v1SNSRCS; i++)
				  {    data->outpVector[data->outNumber++]
					     = here->BSIM3v1SnVar[OUTNOIZ][i];
			               data->outpVector[data->outNumber++]
					     = here->BSIM3v1SnVar[INNOIZ][i];
			          }
		              }
		              break;
		      }
		      break;
	         case N_CLOSE:
		      /* do nothing, the main calling routine will close */
		      return (OK);
		      break;   /* the plots */
	      }       /* switch (operation) */
	 }    /* for here */
    }    /* for model */

    return(OK);
}



