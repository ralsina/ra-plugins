/*w [@cdbutils.c@] */
#include <cdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cdbutils.h"

int
lineincdb (bstring data, bstring fname)
{
  int retval = 1;
  cdbi_t vlen;
  int fd;
  fd = open (fname->data, O_RDONLY);    /* open a CDB file */
  if (fd == -1)
    {
      retval = -1;
    }

  else
    {
      btolower (data);
      if (cdb_seek (fd, data->data, data->slen, &vlen) <= 0)
        {
          retval = 0;
        }
      close (fd);
    }
  return retval;
}
