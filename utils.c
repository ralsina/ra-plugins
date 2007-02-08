#include "utils.h"

#include <sys/types.h>
#include <unistd.h>

// Converts an envvar to an int
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
  //printf ("%s=%d\n",vname,retval);
  return retval;
}

// Converts an envvar to a bstring
// Returns null if the variable is empty (careful!)
//
bstring
envtostr (char *vname)
{
  bstring retval;
  retval = bfromcstr (getenv (vname));
  return retval;
}

// Returns 1 if a line exactly like data is in the 
// fname file, 0 if not.
// -1 if there is an error opening the file
// It is **not** case sensitive

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

bstring pluginname = 0;

void
_log (bstring msg)
{
  if (!pluginname)
    {
      pluginname = bfromcstr ("");
    }
  bstring buffer =
    bformat ("%s: pid %d - %s\n", pluginname->data, getppid (), msg->data);
  fprintf (stderr, "%s", buffer->data);
  bdestroy (buffer);
}

int
checkaddr (bstring address, bstring * user, bstring * domain)
{
  struct bstrList *pieces = bsplit (address, '@');
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

  user[0] = bstrcpy (pieces->entry[0]);
  domain[0] = bstrcpy (pieces->entry[1]);
  bstrListDestroy (pieces);
  return 1;
}

int
checkrbl (bstring lookup_addr, const char *rbl)
{
  struct addrinfo *ai = NULL;
  bstring lookupname = bformat ("%s.%s", lookup_addr->data, rbl);
  if (getaddrinfo (lookupname->data, NULL, NULL, &ai))
    {
      if (ai)
        freeaddrinfo (ai);
      return 0;
    }
  freeaddrinfo (ai);
  return 1;
}

