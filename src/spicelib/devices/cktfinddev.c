/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "ngspice/config.h"
#include "ngspice/cktdefs.h"
#include "ngspice/devdefs.h"
#include "ngspice/sperror.h"
#include "string.h"


GENinstance *
CKTfndDev(CKTcircuit *ckt, int *type, GENinstance **fast, IFuid name)
{
    GENinstance *here;

    /* we know the device instance `fast' */
    if (fast && *fast) {
        if (type)
            *type = (*fast)->GENmodPtr->GENmodType;
        return *fast;
    }

    here = nghash_find(ckt->DEVnameHash, name);

    if (here) {

        if (fast)
            *fast = here;

        if (type)
            *type = here->GENmodPtr->GENmodType;

        return here;
    }

    return NULL;
}
