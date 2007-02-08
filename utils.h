#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include "bstrlib.h"


// Converts an envvar to an int
int envtoint (char *vname, int defval);

// Converts an envvar to a bstring
bstring envtostr (char *vname);

// Returns 1 if a line exactly like data is in the 
// fname file, 0 if not.
// -1 if there is an error opening the file
// It is **not** case sensitive

int lineinfile (bstring data, bstring fname);

// Log msg to stderr in my usual format
// pluginname: pid 1234 msg
// needs you to set the global bstring pluginname to the 
// right value inside your plugin

// Makes basic sanity checks for an email address:
// * Has a @
// * Has something before and after that.
// * Returns user , domain on the obvious args.
// * 0 if address is broken
// * 1 if address is ok

int checkaddr (bstring address, bstring *user, bstring *domain);

extern bstring pluginname;
void _log (bstring msg);

#endif
