#ifndef DNS_UTILS_H
#define DNS_UTILS_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include "udns/udns.h"
#include "bstrlib.h"

int checkrbl (bstring lookup_addr, const char *rbl);

#endif
