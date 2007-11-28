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

#include <sys/types.h>
#include <sys/stat.h>
#include <relay.h>
#include "utils.h"

/*

Plugin to connect to relayd, a connection throttle mechanism.
This works on HELO, and checks TCPREMOTEIP against the
relayd.

The idea being that you can only start N sessions over
a given period of time.

*/

int
    main ()
{
    pluginname = bfromcstr ("ipthrottle");

    bstring remoteip = envtostr ("TCPREMOTEIP");
    bstring ipsvdir = envtostr ("IPSVDIR");

    if (!remoteip)      //Should not happen
    {
        _log (bfromcstr ("511 No IP address"));
        exit (0);
    }

    if (relayd_open () < 0)
    {
        _log (bfromcstr ("Can't connect to relayd"));
        exit (0);
    }

    int result = relayd_check_ip (remoteip->data);

    if (result == 0)              // Exceeding current rate for IP
    {
        printf
            ("R421 Too many connections. Please reduce connections per minute. Blocking for 2 minutes.\n");
        _log (bformat ("Rate exceeded per IP (%s)", remoteip->data));
        // If IPSVDIR is set, and there is no ipsvd file for this IP, block it.
        if (ipsvdir && ipsvdir->data)
        {
          bstring path=bformat("%s/%s",ipsvdir->data,remoteip->data);
          struct stat st;
          if (0==stat(path->data,&st))
          {
            _log (bformat ("Blocked %s", remoteip->data));
            int f=open (path->data);
            close(f);
            chmod (path->data,S_IWUSR);
          }
        }
    }
    relayd_ack ("", remoteip->data);
    relayd_commit ();
    relayd_close ();
    exit (0);
}
