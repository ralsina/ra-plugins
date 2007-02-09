#ifndef VPOP_UTILS_H
#define VPOP_UTILS_H

#include "bstrlib.h"

// Utility functions to deal with vpopmail
// 1 if domain is local, 0 if not

int isDomainVirtual (bstring domain);
int isSubAddress (bstring username, bstring domain);
int isAlias (bstring username, bstring domain);
int isUser (bstring username, bstring domain);
int quotaUsage (bstring username, bstring domain);

#endif
