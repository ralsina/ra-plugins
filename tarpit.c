/*w [@tarpit.c@]
 * Copyright (C) 2004 Pawel Foremski <pjf@asn.pl>
 * Copyright © 2006 Roberto Alsina <ralsina@kde.org>
 *
 * For any questions please contact Roberto Alsina, because
 * this version is heavily modified from Mr. Foremski's original.
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

  pluginname = bfromcstr ("tarpit");

  int tc, rc;

  tc = envtoint ("TARPITCOUNT", 50);
  rc = envtoint ("SMTPRCPTCOUNTALL", -1);

  if (rc == -1)
    {
      _log (bfromcstr ("can't get number of envelope recipients"));
      exit (0);
    }

  if (rc < tc)
    exit (0);                   /* under limit */
  if (rc == tc)
    _log (bfromcstr ("started tarpitting"));

  bstring tdelay = envtostr ("TARPITDELAY");
  if (!tdelay)
    tdelay = bfromcstr ("2");

  switch (*(tdelay->data))
    {
    case 'N':                  /* NORMAL */
      sleep ((rc - tc + 1) * 2);
      break;
    case 'M':                  /* MEDIUM */
      sleep ((rc - tc + 1) * 5);
      break;
    case 'H':                  /* HARD */
      sleep ((rc - tc + 1) * (rc - tc + 1));
      break;
    default:
      sleep (atoi (tdelay->data));
      break;
    }

  exit (0);
}
