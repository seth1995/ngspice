/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#ifndef CCCS
#define CCCS

#include "ngspice/ifsim.h"
#include "ngspice/gendefs.h"
#include "ngspice/complex.h"
#include "ngspice/cktdefs.h"

    /* structures used to describe Current Controlled Current Sources */

/* information needed for each instance */

typedef struct sCCCSinstance {
    struct sCCCSmodel *CCCSmodPtr;  /* backpointer to model */
    struct sCCCSinstance *CCCSnextInstance;  /* pointer to next instance of 
                                             *current model*/
    IFuid CCCSname; /* pointer to character string naming this instance */
    int CCCSstate; /* not used */

    const int CCCSposNode; /* number of positive node of source */
    const int CCCSnegNode; /* number of negative node of source */
    int CCCScontBranch;    /* number of branch eq of controlling source */

    char *CCCScontName; /* pointer to name of controlling instance */

    double CCCScoeff;   /* coefficient */

    double CCCSmValue;  /* Parallel multiplier */

    double *CCCSposContBrPtr;  /* pointer to sparse matrix element at 
                                     *(positive node, control branch eq)*/
    double *CCCSnegContBrPtr;  /* pointer to sparse matrix element at 
                                     *(negative node, control branch eq)*/
    unsigned CCCScoeffGiven :1 ;   /* flag to indicate coeff given */
    unsigned CCCSmGiven     :1 ;  /* flag to indicate multiplier given */

    int  CCCSsenParmNo;   /* parameter # for sensitivity use;
            set equal to  0 if not a design parameter*/

} CCCSinstance ;

/* per model data */

typedef struct sCCCSmodel {       /* model structure for a source */
    int CCCSmodType;    /* type index of this device type */
    struct sCCCSmodel *CCCSnextModel;   /* pointer to next possible model 
                                         *in linked list */
    CCCSinstance * CCCSinstances;       /* pointer to list of instances 
                                             that have this model */
    IFuid CCCSmodName;       /* pointer to character string naming this model */

    /* --- end of generic struct GENmodel --- */

} CCCSmodel;

/* device parameters */
#define CCCS_GAIN 1
#define CCCS_CONTROL 2
#define CCCS_POS_NODE 3
#define CCCS_NEG_NODE 4
#define CCCS_CONT_BR 5
#define CCCS_GAIN_SENS 6
#define CCCS_CURRENT 7
#define CCCS_POWER 8
#define CCCS_VOLTS 9
#define CCCS_M 10

/* model parameters */

/* device questions */
#define CCCS_QUEST_SENS_REAL         201
#define CCCS_QUEST_SENS_IMAG         202
#define CCCS_QUEST_SENS_MAG      203
#define CCCS_QUEST_SENS_PH       204
#define CCCS_QUEST_SENS_CPLX         205
#define CCCS_QUEST_SENS_DC       206

/* model questions */

#include "cccsext.h"

#endif /*CCCS*/
