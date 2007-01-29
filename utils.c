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

bstring pluginname;

void
_log (bstring msg)
{
  bstring buffer =
    bformat ("%s: pid %d - %s\n", pluginname->data, getppid (), msg->data);
  fprintf (stderr, buffer->data);
  bdestroy (buffer);
}
