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


#include <errno.h>
#include "djbdns/dns.h"
#include "utils.h"

#include <glib.h>
#include "smtp/libsmtp.h"

#include <tdb.h>
#include <time.h>
#include <fcntl.h>


bstring smtpmailfrom;
bstring buffer;
TDB_CONTEXT *db = NULL;

void init_cache ();
time_t check_cache (bstring address);
void cache (bstring address);

int compar1 (const void *vp0, const void *vp1);
void accept ();
void block_permanent (const char *message);
void block_temporary (const char *message);
int checkSender (bstring server, bstring address);

int
main ()
{
  pluginname = bfromcstr ("checkback");
  bstring buffer;
  int cachetime = envtoint ("CHECKBACK_CACHETIME", 86400);


  if (envtostr ("SMTPAUTHUSER"))
    {
      _log (bfromcstr ("Authenticated user, no checking."));
      accept ();
    }

  // Get the address and domain of the sender

  smtpmailfrom = envtostr ("SMTPMAILFROM");
  if (!smtpmailfrom)
    {
      block_permanent ("no MAIL FROM envelope header has been sent.");
    }


  struct bstrList *pieces = bsplit (smtpmailfrom, '@');
  if (pieces->qty != 2)
    {
      buffer =
        bformat ("invalid mail address in MAIL FROM envelope header: %s",
                 smtpmailfrom->data);
      block_permanent (buffer->data);
    }
  bstring username = pieces->entry[0];
  bstring domain = pieces->entry[1];

  if (domain->slen == 0 || username->slen == 0)
    {
      buffer =
        bformat ("invalid mail address in MAIL FROM envelope header: %s",
                 smtpmailfrom->data);
      block_permanent (buffer->data);
    }

  _log (bformat ("Checking sender (%s)", smtpmailfrom->data));

  // See if the domain is whitelisted

  int r = lineinfile (smtpmailfrom, bfromcstr ("/var/qmail/control/cbwldom"));
  if (r == -1)
    {
      // Not fatal, maybe it simply doesn't exist
      // so it's not even an error.
      //_log (bfromcstr("Error opening /var/qmail/control/cbwldom"));
    }
  else if (r == 1)
    {
      _log (bfromcstr("Whitelisted domain"));
      exit (0);
    }

  // See if the address is whitelisted

  r = lineinfile (smtpmailfrom, bfromcstr ("/var/qmail/control/cbwladdr"));
  if (r == -1)
    {
      // Not fatal, maybe it simply doesn't exist
      // so it's not even an error.
      //_log (bfromcstr("Error opening /var/qmail/control/cbwladdr"));
    }
  else if (r == 1)
    {
      _log (bfromcstr("Whitelisted address"));
      exit (0);
    }

  // See if it's a known good sender

  init_cache ();
  r = check_cache (smtpmailfrom);
  if (r != -1 && cachetime > r)
    {
      _log (bformat
            ("Sender in cache (%s - %d seconds)", smtpmailfrom->data, r));
      accept ();
    }


  // Get the MX records for the sender's domain

  stralloc out = { 0 };
  stralloc fqdn = { 0 };
  stralloc_copys (&fqdn, domain->data);

  if (dns_mx (&out, &fqdn) < 0 || out.len == 0) // No MX record, or error
    {
      if ((errno == ECONNREFUSED) || (errno == EAGAIN))
        block_temporary ("DNS temporary failure.");

      // check for A record instead of MX record
      else if (dns_ip4 (&out, &fqdn) < 0 || out.len == 0)       // No A record, or error
        {
          if ((errno == ECONNREFUSED) || (errno == EAGAIN))
            block_temporary ("DNS temporary failure.");
          else
            {
              buffer =
                bformat ("your envelope sender domain must exist: %s",
                         domain->data);
              block_permanent (buffer->data);
            }
        }

      else                      //Check against sending domain's A
        {
          int r = checkSender (domain, smtpmailfrom);
          if (r == 0)
            {
              cache (smtpmailfrom);
              accept ();
            }
          if (r == 1)
            {
              block_permanent ("Could not verify sender");
            }
          if (r == 2)
            {
              block_temporary ("Couldn't verify sender, come back later");
            }
          if (r == 3)
            {
              block_permanent ("Sender's server doesn't accept my mail");
            }
        }
    }

  else                          //Check against sending domain's MXs 
    {
      int pos = 0;
      int mxcount = 0;
      bstring mxs[20];          // should be more than enough
      while (pos < out.len)
        {
          // The first two bytes are a distance
          int dist = out.s[pos] * 256 + out.s[pos + 1];

          // From pos+2 until the next \0 is the name of the MX
          int l = strlen (out.s + pos + 2);
          bstring mx = bformat ("%08d%s", dist, out.s + pos + 2);

          mxs[mxcount] = mx;
          mxcount++;
          if (mxcount > 19)     //just paranoic
            break;

          // Move forward the length of the distance (2), plus
          // the length of the MX name, plus the terminating NULL
          pos = pos + l + 3;
        }
      qsort (mxs, mxcount, sizeof (bstring), compar1);
      int j = 0;
      int r;
      int istemperr = 0;
      for (j = 0; j < mxcount; j++)
        {
          r = checkSender (bmidstr (mxs[j], 8, 1000), smtpmailfrom);
          if (r == 2)           // Temp. error, try next MX
            {
              istemperr = 1;
              continue;
            }
          if (r == 1)
            {
              block_permanent ("Could not verify sender");
            }
          if (r == 3)           // Error on MAIL FROM, could be many things
            {
              block_permanent ("Sender's server doesn't accept my mail");
            }
          cache (smtpmailfrom);
          accept ();
        }
      // We ran out of MXs
      if (istemperr == 1)       // At least one error was temporary
        {
          block_temporary ("Couldn't verify sender, come back later");
        }
      else                      // All errors were permanent
        {
          block_permanent ("Could not verify sender");
        }
    }
  cache (smtpmailfrom);
  accept ();
}


// Returns: 0 if it validates
//          1 if the server rejects
//          2 if it's a temporary error
//          3 if it's an error sending MAIl FROM: <> 

int
checkSender (bstring server, bstring address)
{
  _log (bformat ("Testing server %s for address %s", server->data,
                 address->data));
  struct libsmtp_session_struct *session = libsmtp_session_initialize ();
  int r;
  int retval = 99;
  int smtpresp;
  r = libsmtp_add_recipient (LIBSMTP_REC_TO, address->data, session);
  r = libsmtp_set_environment ("<>", "CheckBack SMTP Test", 0, session);
  // Open connection, gives the EHLO
  r = libsmtp_connect (server->data, 25, 0, session);
  if (r == 0)                   // Connected
    {
      // See if it's not RFC-ignorant
      r = libsmtp_dialogue_send ("MAIL FROM: <>\r\n", session);
      if (r == 0)               // FROM sent successfully
        {
          smtpresp = session->LastResponseCode;
          if (smtpresp < 299)   // From ok
            {
              bstring buffer = bformat ("RCPT TO: <%s>\r\n", address->data);
              r = libsmtp_dialogue_send (buffer->data, session);
              if (r == 0)       // RCPT sent successfully
                {
                  smtpresp = session->LastResponseCode;
                  if (smtpresp < 299)   // RCPT ok
                    {
                      retval = 0;       // We like it
                    }
                  else          // RCPT not ok
                    {
                      _log (bformat
                            ("Server rejected our RCPT TO: %s with code %d",
                             address->data, smtpresp));
                      retval = 1;       // We really don't like it
                    }
                }               // Error sending RCPT TO
            }
          else                  // From not ok
            {
              _log (bformat
                    ("Server rejected our MAIL FROM: <> with code %d",
                     smtpresp));
              retval = 3;       // We really don't like it
            }
        }                       // Error sending MAIL FROM 
    }                           // Error connecting

  // Now, if we got here with a retval of 99, that
  // means we had some sort of network error
  // and we should just return temporary error.
  // I miss exceptions!

  if (retval == 99)
    {
      _log (bformat ("Server %s gave error %d: %s", server->data,
                     r, libsmtp_strerr (session)));
      retval = 2;
    }
  else                          // We know what to return, and the connection is alive
    {
      // Be polite, but don't care about errors
      r = libsmtp_dialogue_send ("QUIT \r\n", session);
    }
  libsmtp_free (session);
  return retval;
}


int
compar1 (const void *vp0, const void *vp1)
{
  const bstring *ip0 = (const bstring *) vp0;
  const bstring *ip1 = (const bstring *) vp1;
  return (bstricmp (*ip0, *ip1));
}

void
accept ()
{
  if (db)
    tdb_close (db);
  _log (bformat ("OK %s", smtpmailfrom->data));
  exit (0);
}


void
block_permanent (const char *message)
{
  printf ("E553 sorry, %s (#5.7.1)\n", message);
  _log (bformat ("blocked with: %s", message));
  if (db)
    tdb_close (db);
  exit (0);
}


void
block_temporary (const char *message)
{
  printf ("E451 %s\n", message);
  _log (bformat ("temporary failure: %s", message));
  if (db)
    tdb_close (db);
  exit (0);
}


// Open cache DB file, do initialization, etc.


void
init_cache ()
{
  bstring cfname = envtostr ("CHECKBACK_CACHE");
  if (!cfname)
    {
      cfname = bfromcstr ("/var/qmail/control/checkbackcache.tdb");
    }
  db = tdb_open (cfname->data, 0, 0, O_RDWR | O_CREAT, 0600);
  if (!db)
    {
      _log (bformat ("Error opening DB (%s)", cfname->data));
      exit (0);
    }
}


// Checks if the address given is in the cache.
//
// Returns -1 if it is not there
//
// Returns a positive number of seconds (the age of the entry)
// if the address is in the cache.

time_t
check_cache (bstring address)
{
  TDB_DATA key;
  key.dptr = address->data;
  key.dsize = address->slen;
  TDB_DATA data = tdb_fetch (db, key);
  if (!data.dptr)               // not in cache
    {
      return -1;
    }
  return time (NULL) - (*((time_t *) data.dptr));
  // I don't bother cleaning because this program is exiting very soon
}

// Add an address to the cache

void
cache (bstring address)
{
  TDB_DATA key;
  key.dptr = address->data;
  key.dsize = address->slen;
  TDB_DATA data;
  data.dptr = malloc (sizeof (time_t));
  data.dsize = sizeof (time_t);
  time ((time_t *) data.dptr);
  if (-1 == tdb_store (db, key, data, TDB_REPLACE))
    {
      _log (bfromcstr ("Error saving data to cache"));
      tdb_close (db);
      exit (0);
    }
  // I don't bother cleaning because this program is exiting very soon
}
