/*w [@localmail.c@]
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
  pluginname = bfromcstr ("localmail");

  bstring rcpt = envtostr ("SMTPRCPTTO");
  /*w No rcpt given?. Bad usage? */
  if (!rcpt)
    {
      _log (bfromcstr ("error: no SMTPRCPTTO"));
      exit (0);
    }

  bstring authuser = envtostr ("SMTPAUTHUSER");
  /*w The user is not authenticated.
     This plugin doesn't apply, exit quietly */
  if (!authuser)
    {
      exit (0);
    }

  bstring cfname = envtostr ("LOCALMAILCONF");
  if (!cfname)
    cfname = bfromcstr ("/var/qmail/control/localonly");

  int localonly = lineinfile (authuser, cfname);
  if (localonly == -1)
    {
      _log (bformat ("error opening %s", cfname->data));
      exit (0);
    }

  if (localonly)
    {
      /*w The recipient is remote, so forbid it */
      if (0 == envtoint ("SMTPRCPTHOSTSOK", 0))
        {
          printf ("E550 You are only allowed to send to local addresses.");
          _log (bformat ("blocked from %s to %s",
                         authuser->data, rcpt->data));
        }
      exit (0);
    }

  exit (0);
}
