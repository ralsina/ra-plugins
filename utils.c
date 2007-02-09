/*w
[@utils.c@]

Miscelaneous functions that are used by several plugins but
do not require any external dependencies beyond [[#bstrlib|bstrlib]].

See also [[#utils.h|utils.h]]

*/


#include "utils.h"

#include <sys/types.h>
#include <unistd.h>

/*w
[@envtoint@]

== envtoint ==

Convert an environment variable to an int.

If the variable is not defined, returns defval.

 */
int
envtoint (char *vname, int defval)
{
  bstring envvar_;
  int retval = defval;

  envvar_ = bfromcstr (getenv (vname));
  if (envvar_)
    {
      retval = atoi (envvar_->data);
      bdestroy (envvar_);
    }
  return retval;
}

/*w
 [@envtostr@]

== envtostr ==

 Convert an environment variable to a bstring
 Returns null if the variable is empty (careful!)

 */

bstring
envtostr (char *vname)
{
  bstring retval;
  retval = bfromcstr (getenv (vname));
  return retval;
}

/*w [@lineinfile@]

== lineinfile ==

Returns 1 if a line exactly like data is in the
 fname file, 0 if not.
 -1 if there is an error opening the file
 It is **not** case sensitive

*/

int
lineinfile (bstring data, bstring fname)
{
  int retval = 0;
  FILE *f = fopen (fname->data, "r");
  if (!f)
    {
      // Can't open file.
      return -1;
    }

  bstring line;
  for (;;)
    {
      line = bgets ((bNgetc) fgetc, f, '\n');
      if (!line)
        {
          // End of stream
          break;
        }
      //You may lack a \n at the last line :-P
      if (line->slen > 0 && line->data[line->slen - 1] == '\n')
        {
          btrunc (line, line->slen - 1);
        }
      if (0 == bstricmp (data, line))
        {
          retval = 1;
          break;
        }
      bdestroy (line);
    }
  fclose (f);
  return retval;
}

/*w [@pluginname@]
 Name of the plugin, used by [[#_log|_log]] to identify who is logging.
 */

bstring pluginname = 0;

/*w
 [@_log@]

== _log ==

Log msg to stderr in my usual format
%%

pluginname: pid 1234 msg

%%

 You need to set the global bstring pluginname to the
 right value inside your plugin

*/

void
_log (bstring msg)
{
  /*w Avoid stupid segfault (this bug was there for months */

  if (!pluginname)
    {
      pluginname = bfromcstr ("");
    }

  /*w
     The log format is:

     pluginname: pid XXXX - message

     The pid used is qmail-smtpd's so you can easily group all the log
     entries relating to the same SMTP connection.

   */

  bstring buffer =
    bformat ("%s: pid %d - %s\n", pluginname->data, getppid (), msg->data);
  fprintf (stderr, "%s", buffer->data);
  bdestroy (buffer);
}

/*w
[@checkaddr@]

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

*/

int
checkaddr (bstring address, bstring * user, bstring * domain)
{
  struct bstrList *pieces = bsplit (address, '@');

  /*w Two pieces and just two non-empty pieces per address ;-) */

  if (pieces->qty != 2)
    {
      return 0;
    }
  else if (pieces->entry[0] == 0 ||
           pieces->entry[0]->slen == 0 ||
           pieces->entry[1] == 0 || pieces->entry[1]->slen == 0)
    {
      return 0;
    }

  /*w Return copies of the strings and destroy the list,
     which destroys the originals */

  user[0] = bstrcpy (pieces->entry[0]);
  domain[0] = bstrcpy (pieces->entry[1]);
  bstrListDestroy (pieces);

  /*w There probably are other, nicer tests that should be
     performed. I am all ears if you think of one */

  return 1;
}

/*w The following functions are trivial stuff, but which is used in many
 different plugins, so it's here to avoid duplication */

/*w [@block_permanent@]

== block_permanent ==

Give a 553 (permanent) SMTP error with the given message, and log
the same.

*/

void
block_permanent (const char *message)
{
  printf ("E553 sorry, %s (#5.7.1)\n", message);
  _log (bformat ("blocked with: %s", message));
  exit (0);
}

/*w [@block_temporary@]

== block_temporary ==

Give a 451 (temporary) SMTP error with the given message, and log
the same.

*/


void
block_temporary (const char *message)
{
  printf ("E451 %s (#4.3.0)\n", message);
  _log (bformat ("temporary failure: %s", message));
  exit (0);
}

/*w [@ignore_auth_users@]

== ignore_auth_users ==

If the user is authenticated, exit the plugin.

Keep in mind that if you use this, and your plugin needs
to do cleanup before exit, you should use atexit() to register your
chores.

 */

void
ignore_auth_users (void)
{
  if (envtostr ("SMTPAUTHUSER"))
    {
      _log (bfromcstr ("No checks performed, because user is authenticated"));
      exit (0);
    }
}
