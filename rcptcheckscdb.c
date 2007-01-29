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
#include "cdbutils.h"

int
main ()
{
  bstring pluginname = bfromcstr ("rcptcheckscdb");

  int wrongrcptcount = envtoint ("WRONGRCPTCOUNT", 0);
  int chkuser_rcptlimit = envtoint ("CHKUSER_RCPTLIMIT", 10000);
  int chkuser_wrongrcptlimit = envtoint ("CHKUSER_WRONGRCPTLIMIT", 10000);
  int smtprcptcount = envtoint ("SMTPRCPTCOUNT", 0);
  bstring ctrlfname = envtostr ("CHKUSER_WLFILE");
  bstring addrfname = envtostr ("CHKUSER_ADDRFILE");

  //Reset wrongrcptcount on new mail 
  if (wrongrcptcount > smtprcptcount)
    wrongrcptcount = 0;

  bstring smtprcptto = envtostr ("SMTPRCPTTO");
  btolower (smtprcptto);

  struct bstrList *pieces = bsplit (smtprcptto, '@');
  if (pieces->qty != 2)
    {
      printf ("E511 Invalid address (#5.1.1 - rcptchecksfile)\n");
      _log (bformat ("511 Invalid address (%s)", smtprcptto->data));
      exit (0);
    }
  bstring username = pieces->entry[0];
  bstring domain = pieces->entry[1];


  // If there are more than $CHKUSER_RCPTLIMIT RCPTs accepted
  // fail with error

  if (chkuser_rcptlimit <= smtprcptcount)
    {
      printf
        ("E571 sorry, reached maximum number of recipients for one session (#5.7.1 - rcptchecksfile)\n");
      _log (bformat
            ("571 sorry, reached maximum number of recipients for one session (%d of %d - %s)",
             smtprcptcount, chkuser_rcptlimit, smtprcptto->data));
      exit (0);
    }


  /*******************************************************************/
  /* Domain whitelisting                                             */
  /*******************************************************************/

  // Check if the domain is in our whitelist

  int iswhitelisted = 0;
  if (ctrlfname->slen != 0)
    {
      iswhitelisted = lineincdb (domain, ctrlfname);
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
      _log (bformat
            ("Whitelisted recipient (%s@%s number %d of %d )",
             username->data, domain->data, smtprcptcount, chkuser_rcptlimit));
      exit (0);
    }



  /*******************************************************************/
  /* Domain locality                                                 */
  /*******************************************************************/

  // If it is for one of our RCPTHOSTS, then we will check if the user
  // is in our list of addresses.

  int islocal = lineinfile (domain,
                            bfromcstr ("/var/qmail/control/rcpthosts"));
  if (islocal == -1)
    {
      // Well, maybe we have no rcpthosts. No check possible.
      // Log anyway, because this plugin is usseless in this case!

      _log (bfromcstr ("error opening rcpthosts"));
      exit (0);
    }

  // Since it is accepted here, check if the user exists.

  /*******************************************************************/
  /*  Address checking                                               */
  /*******************************************************************/

  int valid = 1;
  if (islocal == 1)
    {
      valid = lineincdb (smtprcptto, addrfname);
      if (valid == -1)
        {
          // eror opening file, can't do anything,
          // accept mail, but log error
          _log (bformat ("Error opening address list file (%s)",
                         addrfname->data));
          exit (0);
        }
    }


  /*******************************************************************/
  /* Decision and closing                                            */
  /*******************************************************************/

  //Now we should have the following
  //
  // valid = 1 if the user exists or if it's not local
  // valid = 0 if the user is local and does not exist
  //
  // username and domain with the right data

  // We keep the number of failed RCPTs on WRONGRCPTCOUNT

  if (valid == 0)               //Not valid, not relaying, we don't like it
    {
      wrongrcptcount = wrongrcptcount + 1;
      printf ("SWRONGRCPTCOUNT=%d\n", wrongrcptcount);
    }

  // Over $CHKUSER_WRONGRCPTLIMIT we don't care if it's valid or not, by policy
  if (wrongrcptcount > chkuser_wrongrcptlimit)
    {
      printf
        ("E571 sorry, you are violating our security policies (#5.7.1 - rcptchecksfile)\n");
      _log (bformat
            ("571 sorry, you are violating our security policies (%d of %d - %s@%s)",
             wrongrcptcount, chkuser_wrongrcptlimit, username->data,
             domain->data));
      exit (0);
    }

  //Finally, if it's not valid, but we are within limits, plain doesn't exist
  if (valid == 0)
    {
      printf
        ("E511 sorry, no mailbox here by that name (#5.1.1 - rcptchecksfile)\n");
      _log (bformat ("511 sorry, no mailbox here by that name (%s@%s)",
                     username->data, domain->data));
      exit (0);
    }
  _log (bformat ("Accepted recipient (%s@%s number %d of %d )",
                 username->data, domain->data,
                 smtprcptcount, chkuser_rcptlimit));
  exit (0);
}
