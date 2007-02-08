/*
* Copyright (C) 2003-2004 Perolo Silantico <per.sil@gmx.it>
* Copyright (C) 2006 Roberto Alsina <ralsina@kde.org>
*
* For any questions please contact Roberto Alsina, because
* this version is modified from the original.
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
main (int argc, char *argv[])
{
  pluginname = bfromcstr ("hardcoderbl");

  unsigned long address;
  bstring ip = envtostr ("TCPREMOTEIP");
  if (!ip)                      //Somehow we are running in a bad situation
    {
      printf ("D\n");
      _log (bfromcstr ("no TCPREMOTEIP"));
      exit (0);
    }

  //If authenticated, don't check at all
  if (envtostr ("SMTPAUTHUSER"))
    {
      _log (bfromcstr ("No checks performed, because user is authenticated"));
      exit (0);
    }
  if (checkrbl (ip, "bl.spamcop.net"))
    {
      printf
        ("R451 SPAMCOP\tBlocked, look at http://www.spamcop.net/bl.shtml?%s\n",
         ip->data);
      _log (bformat
            ("451 SPAMCOP\tBlocked, look at http://www.spamcop.net/bl.shtml?%s",
             ip->data));
      exit (0);
    }
  if (checkrbl (ip, "combined.njabl.org"))
    {
      printf ("R451 NJABL\tBlocked, look at http://njabl.org/lookup?%s\n",
              ip->data);
      _log (bformat
            ("451 NJABL\tBlocked, look at http://njabl.org/lookup?%s",
             ip->data));
      exit (0);
    }
  // No RBL issues
  _log (bformat ("Accepted %s", ip->data));
  exit (0);
}
