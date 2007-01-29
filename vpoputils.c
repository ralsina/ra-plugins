#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vauth.h>
#include <vpopmail.h>


// Utility functions to deal with vpopmail
// 1 if domain is local, 0 if not (or error)

int
isDomainVirtual (bstring domain)
{
  // Check if the sender domain is in virtualdomains. 
  // We only check for domain.tld:domain.tld
  // because that is how vpopmail does it, and this code is for
  // vpopmail only

  int islocal = 0;

  bstring vrcptdom = bformat ("%s:%s", domain->data, domain->data);

  islocal =
    lineinfile (vrcptdom, bfromcstr ("/var/qmail/control/virtualdomains"));

  if (islocal == -1)
    {
      // Well, maybe we have no virtual domains. No check possible.
      // Log it, because this test is usseless in this case!

      _log (bfromcstr ("error opening virtualdomains"));
      return 0;
    }

  return islocal;

}

// Check if .qmail file exists - mlac
// return 1 if found, 0 if not
int
isSubAddress (bstring username, bstring domain)
{
  // dots in .qmail files names should bo replaced with ':'
  bstring fixedUsername;
  if ((fixedUsername = bstrcpy (username)) != NULL)
    {
      bfindreplace (fixedUsername, bfromcstr ("."), bfromcstr (":"), 0);
      char cRealName[256];
      char cDomainDir[256];
      uid_t Uid;
      gid_t Gid;
      strcpy (cRealName, domain->data);
      if (vget_assign (cRealName, cDomainDir, sizeof (cDomainDir), &Uid, &Gid)
          != NULL)
        {
          char cDotQmailFile[256];
          sprintf (cDotQmailFile, "%s/.qmail-%s", cDomainDir,
                   fixedUsername->data);
          struct stat st;
          if (stat (cDotQmailFile, &st) == 0)
            {
              return 1;
            }
          else
            {
              // no such .qmail
              return 0;
            }
        }
    }
  return 0;
}


int
isAlias (bstring username, bstring domain)
{
  bstring dest = bfromcstr (valias_select (username->data, domain->data));
  if (dest == NULL)
    {
      return 0;
    }
    bdestroy (dest);
  return 1;
}

int
isUser (bstring username, bstring domain)
{
  struct vqpasswd *user_passwd = NULL;
  user_passwd = vauth_getpw (username->data, domain->data);
  if (user_passwd == NULL)
    {
      return 0;
    }
  return 1;
}

// This is defined in libvpopmail.a but it's not on the headers.
// If anyone knows the "right" way to get quota usage info, I
// would like to know it.

int vmaildir_readquota (const char *dir, const char *quota);


int
quotaUsage (bstring username, bstring domain)
{
  struct vqpasswd *user_passwd = NULL;
  user_passwd = vauth_getpw (username->data, domain->data);
  if (strcasecmp (user_passwd->pw_shell, "NOQUOTA") != 0)       //He has a quota
    {
      bstring path;
      bcatcstr (path = bfromcstr (user_passwd->pw_dir), "/Maildir");
      char *q;
      vmaildir_readquota (path->data, q);
      return atoi (q);
    }
  else
    return 0;                   // He has no quota, so he has 0% usage

}
