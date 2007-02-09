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
#include "dnsutils.h"

int
main (void)
{
  pluginname = bfromcstr ("mfdnschecks");

  bstring from = envtostr ("SMTPMAILFROM");
  if (!from)
    {
      block_permanent ("no MAIL FROM envelope header has been sent.");
    }

  // Empty SMTPMAILFROM happens on bounces. The original plugin had them blocked,
  // but then you don't get bounces, and some servers will not even accept your
  // mail later. So, empty SMTPMAILFROM is A-OK.

  if (from->slen == 0)
    exit (0);

  // If it is not empty, but has no @ or no domain part or no username part,
  // we don't like it.

  bstring username, domain;

  if (0 == checkaddr (from, &username, &domain))
    {
      block_permanent (bformat
                       ("invalid mail address in MAIL FROM envelope header: %s",
                        from->data)->data);
    }


  struct bstrList *list;
  int r = mailservers (domain, &list);
  if (r == -1)
    {
      block_temporary ("DNS temporary failure.");
    }
  else if (r == -2 || r == 0)
    {
      block_permanent (bformat
                       ("your envelope sender domain must exist: %s",
                        domain->data)->data);
    }

  _log (bformat ("OK %s", from->data));
  exit (0);
}
