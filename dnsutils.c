#include "dnsutils.h"

int
checkrbl (bstring lookup_addr, const char *rbl)
{
    struct dns_rr_a4  *a4;
    struct in_addr addr;
    dns_init(NULL,1);

    if (dns_pton(AF_INET,lookup_addr->data, &addr) > 0)
    {
        a4=dns_resolve_a4dnsbl(NULL,&addr,rbl);

        if (!a4)
        {
            return 0;
        }
    }
    return 1;
}

bstring
describerbl (bstring lookup_addr, const char *rbl)
{
    struct dns_rr_txt  *txt;
    struct in_addr addr;
    dns_init(NULL,1);

    if (dns_pton(AF_INET,lookup_addr->data, &addr) > 0)
    {
        txt=dns_resolve_a4dnsbl_txt(NULL,&addr,rbl);

        if (txt)
        {
            return bfromcstr(txt->dnstxt_txt->txt);
        }
    }
    return 0;
}


