/*w
[@utils.h@]

Miscelaneous functions that are used by several plugins but
do not require any external dependencies beyond [[#bstrlib|bstrlib]].

See also [[#utils.c|utils.c]].

*/

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include "bstrlib.h"

/*w

== envtoint ==

Convert an environment variable to an int.

If the variable is not defined, returns defval.

See [[#envtoint|implementation]]

*/

int envtoint (char *vname, int defval);


/*w

== envtostr ==

Convert an environment variable to a bstring
 Returns null if the variable is empty (careful!)

 See [[#envtostr|implementation]]
 */

bstring envtostr (char *vname);


/*w

== lineinfile ==

Returns 1 if a line exactly like data is in the
 fname file, 0 if not.
 -1 if there is an error opening the file
 It is **not** case sensitive

 See [[#lineinfile|implementation]]
*/

int lineinfile (bstring data, bstring fname);

/*w

== _log ==

Log msg to stderr in my usual format
%%

pluginname: pid 1234 msg

%%

 You need to set the global bstring pluginname to the
 right value inside your plugin

 See [[#_log|implementation]]

*/

extern bstring pluginname;
void _log (bstring msg);

/*w
== checkaddr ==

 Makes basic sanity checks for an email address:

 - Has a @

 - Has something before and after that.

 - Returns user , domain on the obvious args.

 - Return 0 if address is broken

 - Return 1 if address is ok

Since this is rather cheap, it should be used whenever a plugin
expects an email address to be given, for example when processing
SMTPMAILFROM or SMTPRCPTTO.

The arguments user and domain are modified to contain
the user and domain pieces of the address.

You should not allocate them, but you are responsible for
destroying them after you used them.

See [[#checkaddr|implementation]]
*/

int checkaddr (bstring address, bstring * user, bstring * domain);

/*w The following functions are trivial stuff, but which is used in many
 different plugins, so it's here to avoid duplication */

/*w

== block_permanent ==

Give a 553 (permanent) SMTP error with the given message, and log
the same.

*/


void block_permanent (const char *message);

/*w

== block_temporary ==

Give a 451 (temporary) SMTP error with the given message, and log
the same.

*/


void block_temporary (const char *message);

/*w

== ignore_auth_users ==

If the user is authenticated, exit the plugin.

Keep in mind that if you use this, and your plugin needs
to do cleanup before exit, you should use atexit() to register your
chores.

 */

void ignore_auth_users (void);


#endif
