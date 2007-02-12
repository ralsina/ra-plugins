/*w [@senderchecksfile.c@]
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

int
main ()
{
  pluginname = bfromcstr ("senderchecksfile");
  bstring ctrlfname = envtostr ("SENDERCHK_WLFILE");
  bstring addrfname = envtostr ("SENDERCHK_ADDRFILE");
  bstring smtpmailfrom = envtostr ("SMTPMAILFROM");

  bstring username, domain;

  if (0 == checkaddr (smtpmailfrom, &username, &domain))
    {
      printf ("E501 Invalid address\n");
      _log (bformat ("501 Invalid address (%s)", smtpmailfrom->data));
      exit (0);
    }

  /*******************************************************************/
  /* Domain whitelisting                                             */
  /*******************************************************************/

  // Check if the domain is in our whitelist

  int iswhitelisted = 0;
  if (ctrlfname->slen != 0)
    {
      iswhitelisted = lineinfile (domain, ctrlfname);
      if (iswhitelisted == -1)
        {
          // Can't open whitelist file.
          // Not a lethal problem, but log it anyway.
          _log (bformat ("Couldn't open whitelist file (%s)",
                         ctrlfname->data));
          iswhitelisted = 0;
        }
    }

  if (iswhitelisted)            // No checks necessary
    {
      _log (bformat ("Whitelisted sender (%s@%s)",
                     username->data, domain->data));
      exit (0);
    }


  /*******************************************************************/
  /* Domain locality                                                 */
  /*******************************************************************/

  // If it is for one of our RCPTHOSTS, then we will check if the user
  // is in our list of addresses.

  int islocal =
    lineinfile (domain, bfromcstr ("/var/qmail/control/rcpthosts"));

  if (islocal == -1)
    {
      // Well, maybe we have no rcpthosts. No check possible.
      // Log anyway, because this plugin is usseless in this case!

      _log (bfromcstr ("error opening rcpthosts"));
      exit (0);
    }

  /*******************************************************************/
  /*  Address checking                                               */
  /*******************************************************************/

  int valid = 1;
  if (islocal == 1)
    {
      valid = lineinfile (smtpmailfrom, addrfname);
      if (valid == -1)
        {
          // eror opening file, can't do anything,
          // accept mail, but log error
          _log (bformat ("Error opening address list file (%s)",
                         addrfname->data));
          exit (0);
        }
    }

  //Now we should have the following
  //
  // valid = 1 if the user exists or if it's not local
  // valid = 0 if the user is local and does not exist
  //
  // user_passwd containing the data for the real user (alias already followed)
  // username and domain with the right data


  //Finally, if it's not valid, but we are within limits, plain doesn't exist
  if (valid == 0)
    {
      printf
        ("E511 sorry, no mailbox here by that name (#5.1.1 - senderchecks)\n");
      _log (bformat ("511 sorry, no mailbox here by that name (%s@%s)",
                     username->data, domain->data));
      exit (0);
    }
  _log (bformat ("Accepted sender (%s@%s)", username->data, domain->data));
  exit (0);
}
