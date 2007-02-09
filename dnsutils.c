#include "dnsutils.h"

int
checkrbl (bstring lookup_addr, const char *rbl)
{
  struct dns_rr_a4 *a4;
  struct in_addr addr;
  dns_init (NULL, 1);

  if (dns_pton (AF_INET, lookup_addr->data, &addr) > 0)
    {
      a4 = dns_resolve_a4dnsbl (NULL, &addr, rbl);

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
  struct dns_rr_txt *txt;
  struct in_addr addr;
  dns_init (NULL, 1);

  if (dns_pton (AF_INET, lookup_addr->data, &addr) > 0)
    {
      txt = dns_resolve_a4dnsbl_txt (NULL, &addr, rbl);

      if (txt)
        {
          return bfromcstr (txt->dnstxt_txt->txt);
        }
    }
  return 0;
}


int
mailservers (bstring domain, struct bstrList *list)
{
  struct dns_rr_mx *mx;
  struct dns_rr_a4 *a4;
  int i;
  char buffer[1024];

  dns_init (NULL, 1);

  bstring tmp = bfromcstr ("");

  mx = dns_resolve_mx (NULL, domain->data, 0);
  if (!mx)                      // No MX
    {
      if (dns_status (NULL) == DNS_E_TEMPFAIL)
        {
          return -1;
        }
      a4 = dns_resolve_a4 (NULL, domain->data, 0);
      if (!a4)                  // No A record, or error
        {
          if (dns_status (NULL) == DNS_E_TEMPFAIL)
            {
              block_temporary ("DNS temporary failure.");
            }
          return -1;
        }
      else                      // don't have a MX, but have A
        {
          for (i = 0; i < a4->dnsa4_nrr; i++)
            {
              dns_ntop (AF_INET, &(a4->dnsa4_addr[i]), buffer, 1024);
              tmp = bformat ("%s%s|", tmp->data, buffer);
            }
        }
    }
  else                          // Have MX
    {
      for (i = 0; i < mx->dnsmx_nrr; i++)
        {
          tmp = bformat ("%s%s|", tmp->data, mx->dnsmx_mx[i].name);
        }
    }

  list = bsplit (tmp, '|');
  return list->qty - 1;

}
