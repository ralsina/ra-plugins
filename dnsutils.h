#ifndef DNS_UTILS_H
#define DNS_UTILS_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include "bstrlib.h"
#include "udns/udns.h"

int checkrbl (bstring lookup_addr, const char *rbl);

bstring describerbl (bstring lookup_addr, const char *rbl);

int mailservers (bstring domain, struct bstrList **list);

#endif
