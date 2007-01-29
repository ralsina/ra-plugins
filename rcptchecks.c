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

// Ugly globals ;-)

bstring username = 0;
bstring domain = 0;
bstring smtprcptto = 0;
int smtprcptcount = 0;
int chkuser_rcptlimit = 0;
int wrongrcptcount = 0;
int chkuser_wrongrcptlimit = 0;

// Convenience functions exiting with different
// reasons. They are not necessary, but this way the main code looks
// cleaner and is easier to follow.

void accept ();
void nomailbox ();
void policy ();
void toomanyrcpts ();
void overquota ();

int
main ()
{
  // Read configuration
  pluginname = bfromcstr ("rcptchecks");
  wrongrcptcount = envtoint ("WRONGRCPTCOUNT", 0);
  int chkuser_mbxquota = envtoint ("CHKUSER_MBXQUOTA", 100);
  chkuser_rcptlimit = envtoint ("CHKUSER_RCPTLIMIT", 10000);
  chkuser_wrongrcptlimit = envtoint ("CHKUSER_WRONGRCPTLIMIT", 10000);
  smtprcptcount = envtoint ("SMTPRCPTCOUNT", 0);
  bstring ctrlfname = envtostr ("CHKUSER_WLFILE");

  //Reset wrongrcptcount on new mail 
  if (wrongrcptcount > smtprcptcount)
    wrongrcptcount = 0;

  smtprcptto = envtostr ("SMTPRCPTTO");

  struct bstrList *pieces = bsplit (smtprcptto, '@');
  if (pieces->qty != 2)
    {
      printf ("E511 Invalid address (#5.1.1 - rcptchecks)\n");
      _log (bformat ("511 Invalid address (%s)", smtprcptto->data));
      exit (0);
    }
  username = pieces->entry[0];
  domain = pieces->entry[1];

  // Over $CHKUSER_WRONGRCPTLIMIT we don't care if it's valid or not, by policy
  // so we fail with error

  if (wrongrcptcount > chkuser_wrongrcptlimit)
    {
      policy ();
    }

  // If there are more than $CHKUSER_RCPTLIMIT RCPTs accepted
  // fail with error

  if (chkuser_rcptlimit <= smtprcptcount)
    {
      toomanyrcpts ();
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
      _log (bformat ("Whitelisted recipient (%s@%s number %d of %d )",
                     username->data, domain->data, smtprcptcount,
                     chkuser_rcptlimit));
      accept ();
    }


  if (isDomainVirtual (domain)) //If it is not, not our problem.
    {
      if (isSubAddress (username, domain))
        {
          // We can't check quotas on subaddresses
          accept ();
        }
      else if (isUser (username, domain))       //It's a user
        {
          // Check the quota
          if (chkuser_mbxquota <= quotaUsage (username, domain))
            {
              overquota ();
            }

        }
      else if (isAlias (username, domain))
        {
          // TODO
          // See where it points to
          // Check if the real address is local
          // See if the real address is over quota
          accept ();
        }
      else                      // Doesn't exist
        {
          nomailbox ();
        }
    }
  else
    {
      // We are probably a secondary MX, or
      // it's a non-vpopmail domain
      // in any case, not our problem
      accept ();
    }
}


// Convenience functions to avoid crowding the code above 

void
accept ()
{
  _log (bformat ("Accepted recipient (%s@%s number %d of %d )",
                 username->data, domain->data, smtprcptcount,
                 chkuser_rcptlimit));
  exit (0);
}

void
nomailbox ()
{
  // Update the counter
  wrongrcptcount = wrongrcptcount + 1;
  printf ("SWRONGRCPTCOUNT=%d\n", wrongrcptcount);

  // And reject the address
  printf ("E511 sorry, no mailbox here by that name (#5.1.1 - rcptchecks)\n");
  _log (bformat
        ("511 sorry, no mailbox here by that name (%s@%s)",
         username->data, domain->data));
  exit (0);
}

void
policy ()
{
  printf
    ("E571 sorry, you are violating our security policies (#5.7.1 - rcptchecks)\n");
  _log (bformat
        ("571 sorry, you are violating our security policies (%d of %d - %s@%s)",
         wrongrcptcount, chkuser_wrongrcptlimit, username->data,
         domain->data));
  exit (0);
}

void
toomanyrcpts ()
{
  printf
    ("E571 sorry, reached maximum number of recipients for one session (#5.7.1 - rcptchecks)\n");
  _log (bformat
        ("571 sorry, reached maximum number of recipients for one session (%d of %d - %s)",
         smtprcptcount, chkuser_rcptlimit, smtprcptto->data));
  exit (0);

}

void
overquota ()
{
  printf ("E522 sorry, recipient mailbox is full (#5.2.2 - rcptchecks)\n");
  _log (bformat ("522 sorry, recipient mailbox is full (%s@%s)",
                 username->data, domain->data));
  exit (0);
}
