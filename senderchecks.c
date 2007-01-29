/*
* Copyright (C) 2006 Roberto Alsina <ralsina@kde.org>
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
#include "vpoputils.h"

int
main ()
{
  pluginname = bfromcstr ("senderchecks");

  bstring ctrlfname = envtostr ("SENDERCHK_WLFILE");
  bstring smtpmailfrom = envtostr ("SMTPMAILFROM");

  // Empty sender is OK
  if (smtpmailfrom->slen == 0)
    {
      _log (bfromcstr ("Accepted empty sender"));
      exit (0);
    }

  struct bstrList *pieces = bsplit (smtpmailfrom, '@');
  if (pieces->qty != 2)
    {
      printf ("E501 Invalid address\n");
      _log (bformat ("501 Invalid address (%s)", smtpmailfrom->data));
      exit (0);
    }
  bstring username = pieces->entry[0];
  bstring domain = pieces->entry[1];

  /*******************************************************************/
  /* Domain whitelisting                                             */
  /*******************************************************************/

  // Check if the domain is in our whitelist

  if (ctrlfname->slen != 0)
    {
      int iswhitelisted = 0;
      iswhitelisted = lineinfile (domain, ctrlfname);
      if (iswhitelisted == -1)
        {
          // Can't open whitelist file.
          // Not a lethal problem, but log it anyway.
          _log (bformat ("Couldn't open whitelist file (%s)",
                         ctrlfname->data));
          iswhitelisted = 0;
        }
      else if (iswhitelisted)   // No checks necessary
        {
          _log (bformat ("Whitelisted sender (%s@%s)",
                         username->data, domain->data));
          exit (0);
        }

    }

  if (1 == isDomainVirtual (domain))
    {
      // Since it is virtual here, check if the user exists.
      // Of course, it can exist in different ways  

      // Does it exist as a regular vpopmail user?
      if (isUser (username, domain) ||
          isSubAddress (username, domain) || isAlias (username, domain))
        {
          _log (bformat
                ("Accepted sender (%s@%s)", username->data, domain->data));
          exit (0);
        }
      else
        {
          //It's not anoything we know, it plain doesn't exist
          printf
            ("E511 sorry, no mailbox here by that name (#5.1.1 - senderchecks)\n");
          _log (bformat ("511 sorry, no mailbox here by that name (%s@%s)",
                         username->data, domain->data));
          exit (0);
        }
    }
  else
    {
      _log (bformat ("Accepting non-local sender (%s)", smtpmailfrom));
      exit (0);
    }
}
