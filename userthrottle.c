/*w [@userthrottle.c@]
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

#include <sys/types.h>
#include <relay.h>
#include "utils.h"

/*

Plugin to connect to relayd, a connection throttle mechanism.
This works on MAIL, and checks SMTPAUTHUSER against the
relayd.

The idea being that you can only start N sessions over
a given period of time.

*/

int
main ()
{
  pluginname = bfromcstr ("userthrottle");

  bstring authuser = envtostr ("SMTPAUTHUSER");

  if (!authuser)  // not authenticated, not our business
    {
      exit (0);
    }

  if (relayd_open () < 0)
    {
      _log (bfromcstr ("Can't connect to relayd"));
      exit (0);
    }

  int result = relayd_check_ip (authuser->data);

  if (result == 0)              // Exceeding current rate for IP
    {
      printf
        ("R421 Too many connections. Please reduce connections per minute.\n");
      _log (bformat ("Rate exceeded per user (%s)", authuser->data));
    }
  relayd_ack ("", authuser->data);
  relayd_commit ();
  relayd_close ();
  exit (0);
}
