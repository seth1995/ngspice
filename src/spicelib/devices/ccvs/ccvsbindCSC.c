/**********
Author: 2013 Francesco Lannutti
**********/

#include "ngspice/ngspice.h"
#include "ngspice/cktdefs.h"
#include "ccvsdefs.h"
#include "ngspice/sperror.h"

#include <stdlib.h>

static
int
BindCompare (const void *a, const void *b)
{
    BindElement *A, *B ;
    A = (BindElement *)a ;
    B = (BindElement *)b ;

    return ((int)(A->Sparse - B->Sparse)) ;
}

int
CCVSbindCSC (GENmodel *inModel, CKTcircuit *ckt)
{
    CCVSmodel *model = (CCVSmodel *)inModel ;
    CCVSinstance *here ;
    double *i ;
    BindElement *matched, *BindStruct ;
    size_t nz ;

    BindStruct = ckt->CKTmatrix->CKTbindStruct ;
    nz = (size_t)ckt->CKTmatrix->CKTklunz ;

    /* loop through all the CCVS models */
    for ( ; model != NULL ; model = model->CCVSnextModel)
    {
        /* loop through all the instances of the model */
        for (here = model->CCVSinstances ; here != NULL ; here = here->CCVSnextInstance)
        {
            if ((here-> CCVSposNode != 0) && (here-> CCVSbranch != 0))
            {
                i = here->CCVSposIbrPtr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CCVSposIbrptrStructPtr = matched ;
                here->CCVSposIbrPtr = matched->CSC ;
            }

            if ((here-> CCVSnegNode != 0) && (here-> CCVSbranch != 0))
            {
                i = here->CCVSnegIbrPtr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CCVSnegIbrptrStructPtr = matched ;
                here->CCVSnegIbrPtr = matched->CSC ;
            }

            if ((here-> CCVSbranch != 0) && (here-> CCVSnegNode != 0))
            {
                i = here->CCVSibrNegPtr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CCVSibrNegptrStructPtr = matched ;
                here->CCVSibrNegPtr = matched->CSC ;
            }

            if ((here-> CCVSbranch != 0) && (here-> CCVSposNode != 0))
            {
                i = here->CCVSibrPosPtr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CCVSibrPosptrStructPtr = matched ;
                here->CCVSibrPosPtr = matched->CSC ;
            }

            if ((here-> CCVSbranch != 0) && (here-> CCVScontBranch != 0))
            {
                i = here->CCVSibrContBrPtr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CCVSibrContBrptrStructPtr = matched ;
                here->CCVSibrContBrPtr = matched->CSC ;
            }

        }
    }

    return (OK) ;
}

int
CCVSbindCSCComplex (GENmodel *inModel, CKTcircuit *ckt)
{
    CCVSmodel *model = (CCVSmodel *)inModel ;
    CCVSinstance *here ;

    NG_IGNORE (ckt) ;

    /* loop through all the CCVS models */
    for ( ; model != NULL ; model = model->CCVSnextModel)
    {
        /* loop through all the instances of the model */
        for (here = model->CCVSinstances ; here != NULL ; here = here->CCVSnextInstance)
        {
            if ((here-> CCVSposNode != 0) && (here-> CCVSbranch != 0))
                here->CCVSposIbrPtr = here->CCVSposIbrptrStructPtr->CSC_Complex ;

            if ((here-> CCVSnegNode != 0) && (here-> CCVSbranch != 0))
                here->CCVSnegIbrPtr = here->CCVSnegIbrptrStructPtr->CSC_Complex ;

            if ((here-> CCVSbranch != 0) && (here-> CCVSnegNode != 0))
                here->CCVSibrNegPtr = here->CCVSibrNegptrStructPtr->CSC_Complex ;

            if ((here-> CCVSbranch != 0) && (here-> CCVSposNode != 0))
                here->CCVSibrPosPtr = here->CCVSibrPosptrStructPtr->CSC_Complex ;

            if ((here-> CCVSbranch != 0) && (here-> CCVScontBranch != 0))
                here->CCVSibrContBrPtr = here->CCVSibrContBrptrStructPtr->CSC_Complex ;

        }
    }

    return (OK) ;
}

int
CCVSbindCSCComplexToReal (GENmodel *inModel, CKTcircuit *ckt)
{
    CCVSmodel *model = (CCVSmodel *)inModel ;
    CCVSinstance *here ;

    NG_IGNORE (ckt) ;

    /* loop through all the CCVS models */
    for ( ; model != NULL ; model = model->CCVSnextModel)
    {
        /* loop through all the instances of the model */
        for (here = model->CCVSinstances ; here != NULL ; here = here->CCVSnextInstance)
        {
            if ((here-> CCVSposNode != 0) && (here-> CCVSbranch != 0))
                here->CCVSposIbrPtr = here->CCVSposIbrptrStructPtr->CSC ;

            if ((here-> CCVSnegNode != 0) && (here-> CCVSbranch != 0))
                here->CCVSnegIbrPtr = here->CCVSnegIbrptrStructPtr->CSC ;

            if ((here-> CCVSbranch != 0) && (here-> CCVSnegNode != 0))
                here->CCVSibrNegPtr = here->CCVSibrNegptrStructPtr->CSC ;

            if ((here-> CCVSbranch != 0) && (here-> CCVSposNode != 0))
                here->CCVSibrPosPtr = here->CCVSibrPosptrStructPtr->CSC ;

            if ((here-> CCVSbranch != 0) && (here-> CCVScontBranch != 0))
                here->CCVSibrContBrPtr = here->CCVSibrContBrptrStructPtr->CSC ;

        }
    }

    return (OK) ;
}
