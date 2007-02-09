/*w [@dnsutils.c@]
* Copyright (C) 2007 Roberto Alsina <ralsina@kde.org>
*
* For any questions please contact Roberto Alsina, because
* this version is heavily modified from the original.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*/

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
mailservers (bstring domain, struct bstrList **list)
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
              return -1;
            }
          return -2;
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

  list[0] = bsplit (tmp, '|');
  return list[0]->qty - 1;

}
