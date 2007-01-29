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

#include <tdb.h>
#include <time.h>
#include <fcntl.h>


// Open cache DB file, do initialization, etc.

TDB_CONTEXT *db = NULL;
int cachetime = 86400;
int counter = 0;

void
init_cache ()
{
  bstring cfname = envtostr ("CHECKBACK_CACHE");
  pluginname = bfromcstr ("checkbackpurge");

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

int
traverse (TDB_CONTEXT * db, TDB_DATA key, TDB_DATA data, void *state)
{
  if (time (NULL) - (*((time_t *) data.dptr)) > cachetime)
    {
      int r = tdb_delete (db, key);
      if (r == -1)
        {
          _log (bfromcstr ("Error deleting item"));
        }
      counter++;
    }
}

int
main ()
{
  cachetime = envtoint ("CHECKBACK_CACHETIME", 86400);
  init_cache ();
  int c = tdb_traverse (db, traverse, NULL);
  _log (bformat ("Traversed %d items, deleted %d items\n", c, counter));

}
