/*
 * Make e-mail addresses "dot blind" like gmail:
 *   vap.ier@wh0rd.org     -> vapier@wh0rd.org
 *   vap.i....er@wh0rd.org -> vapier@wh0rd.org
 *
 * Copyright 2013 Mike Frysinger <vapier@gentoo.org>
 * Licensed under the GPL-2 or later.
 */

/*
 * By default, we only strip out dots from the username.
 * If you set the DOTBLINDSTRIP env var, you can strip out all
 * chars listed there.
 */

#include "utils.h"

int
main ()
{
  pluginname = bfromcstr ("dotblind");

  /* Make sure SMTPRCPTTO is sane first.  */
  bstring smtprcptto = envtostr ("SMTPRCPTTO");
  if (!smtprcptto)
    {
      _log (bfromcstr ("error: no SMTPRCPTTO"));
      exit (0);
    }

  /* Split out the username/domain so we can mung the username.  */
  bstring username, domain;
  if (0 == checkaddr (smtprcptto, &username, &domain))
    {
      _log (bfromcstr ("error: no valid SMTPRCPTTO"));
      exit (0);
    }

  /* Get the chars to split on.  */
  bstring strip = envtostr ("DOTBLINDSTRIP");
  if (!strip)
    strip = bfromcstr (".");

  /* Do the actual username munging.  */
  int changed = 0;
  int i, j;
  for (i = 0; i < blength (strip); ++i)
    for (j = 0; j < blength (username); ++j)
      while (bchar (strip, i) == bchar (username, j))
        {
          changed = 1;
          bdelete (username, j, 1);
        }

  /* Only issue the updated username if we modified it.  */
  if (changed)
    printf ("C%s@%s\n", username->data, domain->data);

  exit (0);
}
