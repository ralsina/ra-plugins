/*
 * Support e-mail addresses aliases like gmail:
 *   vapier+suckit@wh0rd.org   -> vapier@wh0rd.org
 *   vapier+spamsite@wh0rd.org -> vapier@wh0rd.org
 *
 * Copyright 2013 Mike Frysinger <vapier@gentoo.org>
 * Licensed under the GPL-2 or later.
 */

/*
 * By default, we use + as the alias splitter.
 * If you set the PLUSALIASESCHARS env var, you can select many
 * different ones.  So if you set that to "+!#", then everything
 * after the first one of those encountered will be chopped:
 *   vapier!moo@wh0rd.org          -> vapier@wh0rd.org
 *   vapier+cow@wh0rd.org          -> vapier@wh0rd.org
 *   vapier!moo+cow#fart@wh0rd.org -> vapier@wh0rd.org
 */

#include "utils.h"

int
main ()
{
  pluginname = bfromcstr ("plusaliases");

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
  bstring split = envtostr ("PLUSALIASESCHARS");
  if (!split)
    split = bfromcstr ("+");

  /* Do the actual username munging.  */
  int changed = 0;
  int i;
  for (i = 0; i < blength (split); ++i)
    {
      int p = bstrchr (username, bchar (split, i));
      if (BSTR_ERR != p)
        {
          changed = 1;
          bdelete (username, p, blength (username));
        }
    }

  /* Only issue the updated username if we modified it.  */
  if (changed)
    printf ("C%s@%s\n", username->data, domain->data);

  exit (0);
}
