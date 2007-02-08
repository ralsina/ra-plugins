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

#include "udns/udns.h"
#include "utils.h"

void block_permanent (const bstring message);
void block_temporary (const bstring message);

int
main (void)
{
  pluginname = bfromcstr ("mfdnschecks");
  dns_init (NULL, 1);

  bstring from = envtostr ("SMTPMAILFROM");
  if (!from)
    {
      block_permanent (bfromcstr
                       ("no MAIL FROM envelope header has been sent."));
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
                        from->data));
    }

  /* make query */

  // check for MX records

  struct dns_rr_mx *mx = dns_resolve_mx (NULL, domain->data, 0);

  if (!mx)                      // No MX record, or error
    {
      if (dns_status (NULL) == DNS_E_TEMPFAIL)
        {
          block_temporary (bfromcstr ("DNS temporary failure."));
        }

      // check for A record instead of MX record
      else
        {
          struct dns_rr_a4 *a4 = dns_resolve_a4 (NULL, domain->data, 0);
          if (!a4)              // No A record, or error
            {
              if (dns_status (NULL) == DNS_E_TEMPFAIL)
                {
                  block_temporary (bfromcstr ("DNS temporary failure."));
                }
              else
                {
                  block_permanent (bformat
                                   ("your envelope sender domain must exist: %s",
                                    domain->data));
                }
            }
        }

    }
  _log (bformat ("OK %s", from->data));
  exit (0);
}

void
block_permanent (const bstring message)
{
  printf ("E553 sorry, %s (#5.7.1)\n", message->data);
  _log (bformat ("blocked with: %s", message->data));
  exit (0);
}

void
block_temporary (const bstring message)
{
  printf ("E451 %s (#4.3.0)\n", message->data);
  _log (bformat ("temporary failure: %s", message->data));
  exit (0);
}
