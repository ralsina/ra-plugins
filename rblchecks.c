/*
* Copyright (C) 2003-2004 Perolo Silantico <per.sil@gmx.it>
* Copyright (C) 2006 Roberto Alsina <ralsina@kde.org>
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

#include "utils.h"

int check_rbl (bstring lookup_addr, const char *rbl);



int
main (int argc, char *argv[])
{
  pluginname = bfromcstr ("rblchecks");

  unsigned long address;
  bstring ip = envtostr ("TCPREMOTEIP");

  if (!ip)                      //Somehow we are running in a bad situation
    {
      printf ("D\n");
      _log (bfromcstr ("no TCPREMOTEIP"));
      exit (0);
    }



  address = inet_addr (ip->data);
  bstring addr = bformat ("%lu.%lu.%lu.%lu",
                          (address & 0xff000000) >> 24,
                          (address & 0x00ff0000) >> 16,
                          (address & 0x0000ff00) >> 8,
                          (address & 0x000000ff) >> 0);
  //If authenticated, don't check at all
  if (envtostr ("SMTPAUTHUSER"))
    {
      _log (bfromcstr ("No checks performed, because user is authenticated"));
      exit (0);
    }

  bstring rbl = envtostr ("RBLSERVERS");
  if (!rbl)
    rbl = bfromcstr ("bl.spamcop.net");
  struct bstrList *list = bsplit (rbl, ':');
  int i = 0;
  for (; i < list->qty; i++)
    {
      bstring serv = list->entry[i];
      if (serv->slen == 0)
        continue;
      if (check_rbl (addr, serv->data))
        {
          printf
            ("R541 Your IP (%s) is blocked, more information at http://%s\n",
             ip->data, serv->data);
          _log (bformat ("541 Blocked by %s (%s)", serv->data, ip->data));
          exit (0);
        }
    }

  // No RBL issues
  _log (bformat ("Accepted %s", ip->data));
  exit (0);
}

int
check_rbl (bstring lookup_addr, const char *rbl)
{
  struct addrinfo *ai = NULL;
  bstring lookupname = bformat ("%s.%s", lookup_addr->data, rbl);
  if (getaddrinfo (lookupname->data, NULL, NULL, &ai))
    {
      if (ai)
        freeaddrinfo (ai);
      return 0;
    }
  freeaddrinfo (ai);
  return 1;
}
