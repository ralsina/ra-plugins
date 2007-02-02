/*
* Copyright (C) 2006 Roberto Alsina <ralsina@kde.org>
*
* Large portions copied and/or adapted from spf_example
* written by Wayne Schlitt <wayne@midwestcs.com> and placed
* by him in the public domain.
*
* Please don't bother Mr. Schlitt with questions, they should
* be addressed to Roberto Alsina instead.
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


#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <spf2/spf.h>

#include "utils.h"

int
main ()
{
  pluginname = bfromcstr ("spfchecks");

  SPF_server_t *spf_server = NULL;
  SPF_request_t *spf_request = NULL;
  SPF_response_t *spf_response = NULL;
  SPF_response_t *spf_response_2mx = NULL;

  bstring opt_ip = envtostr ("TCPREMOTEIP");
  bstring opt_sender = envtostr ("SMTPMAILFROM");
  bstring opt_helo = envtostr ("HELOHOST");
  bstring opt_rcpt_to = envtostr ("SMTPRCPTTO");


  //If authenticated, don't check at all
  if (envtostr ("SMTPAUTHUSER"))
    {
      _log (bfromcstr ("No checks performed, because user is authenticated"));
      exit (0);
    }


  /* BOUNCELEVEL is our threshold of annoyance.

     Levels above 2 are surely too much.
     Level 1 is probably too much.
     Level 0 should be OK.
     Level -1 is useful for testing.

     -1 means never reject a message. Useful for testing
     0 means reject on FAIL: the server says this mail is definitely coming from the wrong place
     1 means reject on SOFTFAIL: it's wrong, but the server is in transition, so please be nice
     2 means reject on NEUTRAL: It may be ok, it may not be, but we reject. This breaks the spec.
     3 means reject on NONE: We are **demanding** SPF, kids. Probably breaks the spec too.
     4 means reject on ERROR (TEMPERROR): There is probably a DNS error? This breaks the spec too.
     5 means reject on UNKNOWN (PERMERROR): Oh, go to hell already! This setting breaks the spec too.
   */

  int bouncelevel = envtoint ("BOUNCELEVEL", -1);

  if ((!opt_ip) || opt_ip->slen == 0
      || ( ((!opt_helo) || opt_helo->slen == 0) && opt_sender->slen == 0))
    {
      _log (bfromcstr ("error, missing information."));
      exit (0);
    }

  /*
   * set up the SPF server
   *
   * Configurations contain malloc'd data so must be
   * destroyed when you are finished.
   */

  spf_server = SPF_server_new (SPF_DNS_CACHE, 0);

  if (spf_server == NULL)
    {
      _log (bfromcstr ("error, can't initialize SPF library"));
      exit (0);
    }

  /*
   * Create a new request.
   *
   * The SPF request contains all the data needed to process
   * the SPF check. Requests are malloc'd so it must be
   * destroyed when you are finished with it.
   */

  spf_request = SPF_request_new (spf_server);

  /* The domain name of the receiving MTA will default to gethostname(), but we have
     the actual SMTP greeting name here */
  /* Of course this was commented in the example because the damn function
     does not exist. Sure, it used to, in some form, but it doesn't now.

     if (SPF_request_set_rec_dom (spf_request, opt_name->data))
     {
     fprintf (stderr,
     "spfchecks: pid %d - error setting receiving domain name (%s).\n",
     ppid, opt_name->data);
     exit (0);
     } */

  /*
   * record the IP address of the client (sending) MTA.
   *
   * There are other SPF_set_ip*() functionx if you have a structure
   * instead of a string.
   */

  if (SPF_request_set_ipv4_str (spf_request, opt_ip->data))
    {
      _log (bformat ("error, invalid IP address (%s).", opt_ip->data));
      exit (0);
    }

  /*
   * record the HELO domain name of the client (sending) MTA from
   * the SMTP HELO or EHLO commands
   *
   * This domain name will be used if the envelope from address is
   * null (e.g. MAIL FROM:<>).  This happens when a bounce is being
   * sent and, in effect, it is the client MTA that is sending the
   * message.
   */

  if (SPF_request_set_helo_dom (spf_request, opt_helo->data))
    {
      _log (bformat ("error, invalid HELO domain (%s).", opt_helo->data));
      exit (0);
    }
  /*
   * record the envelope from email address from the SMTP MAIL FROM:
   * command.
   */

  if (SPF_request_set_env_from (spf_request, opt_sender->data))
    {
      _log (bformat
            ("error, invalid envelope from address (%s).", opt_sender->data));
      exit (0);
    }

  /*
   * now that we have all the information, see what the result of
   * the SPF check is.
   */

  if (SPF_request_query_mailfrom (spf_request, &spf_response))
    {
      _log (bfromcstr ("error querying MAIL-FROM"));
    }

  /*
   * If the sender MAIL FROM check failed, then for each SMTP RCPT TO
   * command, the mail might have come from a secondary MX for that
   * domain.
   *
   * Note that most MTAs will also check the RCPT TO command to make sure
   * that it is ok to accept. This SPF check won't give a free pass
   * to all secondary MXes from all domains, just the one specified by
   * the rcpt_to address. It is assumed that the MTA checks (at some
   * point) that we are also a valid primary or secondary for the domain.
   */
  if (SPF_response_result (spf_response) != SPF_RESULT_PASS)
    {
      SPF_request_query_rcptto (spf_request, &spf_response_2mx,
                                opt_rcpt_to->data);
      /*
       * We might now have a PASS if the mail came from a client which
       * is a secondary MX from the domain specified in opt_rcpt_to.
       *
       * If not, then the RCPT TO: address must have been a domain for
       * which the client is not a secondary MX, AND the MAIL FROM: domain
       * doesn't doesn't return 'pass' from SPF_result()
       */
      if (SPF_response_result (spf_response_2mx) == SPF_RESULT_PASS)
        {
          // All is good
          _log (bfromcstr ("accepted from secondary MX"));
          exit (0);
        }

    }

  bstring smtp_comment =
    bfromcstr (SPF_response_get_smtp_comment (spf_response));
  bstring header_comment =
    bfromcstr (SPF_response_get_smtp_comment (spf_response));
  bstring comment = header_comment;
  if (!header_comment)
    {
      comment = smtp_comment;
      header_comment = bfromcstr ("");
    }
  if (!smtp_comment)
    {
      smtp_comment = bfromcstr ("");
    }
  if (!comment)
    comment = bfromcstr ("No comments");

  switch (SPF_response_result (spf_response))
    {
    case SPF_RESULT_PASS:
      _log (bformat ("SPF_RESULT_PASS, %s", comment->data));
      exit (0);
      break;
    case SPF_RESULT_FAIL:
      if (bouncelevel > -1)
        {
          printf ("E550 %s\n", smtp_comment->data);
          _log (bformat ("Rejecting on SPF_RESULT_FAIL, %s", comment->data));
        }
      else
        {
          _log (bformat ("Accepting on SPF_RESULT_FAIL, %s", comment->data));
        }
      exit (0);
      break;
    case SPF_RESULT_SOFTFAIL:
      if (bouncelevel > 0)
        {
          printf ("E550 %s\n", smtp_comment->data);
          _log (bformat
                ("Rejecting on SPF_RESULT_SOFTFAIL, %s", comment->data));
        }
      else
        {
          _log (bformat
                ("Accepting on SPF_RESULT_SOFTFAIL, %s", comment->data));
        }
      exit (0);
      break;
    case SPF_RESULT_NEUTRAL:
      if (bouncelevel > 1)
        {
          printf ("E550 Message rejected by server\n");
          _log (bformat
                ("Rejecting on SPF_RESULT_NEUTRAL, %s", comment->data));
        }
      else
        {
          _log (bformat
                ("Accepting on SPF_RESULT_NEUTRAL, %s", comment->data));
        }
      exit (0);
      break;
    case SPF_RESULT_NONE:
      if (bouncelevel > 2)
        {
          printf ("E550 Message rejected by server\n");
          _log (bformat ("Rejecting on SPF_RESULT_NONE, %s", comment->data));
        }
      else
        {
          _log (bformat ("Accepting on SPF_RESULT_NONE, %s", comment->data));
        }
      exit (0);
      break;
    case SPF_RESULT_TEMPERROR:
      if (bouncelevel > 3)
        {
          printf ("E550 Message rejected by server\n");
          _log (bformat
                ("Rejecting on SPF_RESULT_TEMPERROR, %s", comment->data));
        }
      else
        {
          _log (bformat
                ("Accepting on SPF_RESULT_TEMPERROR, %s", comment->data));
        }
      exit (0);
      break;
    case SPF_RESULT_PERMERROR:
      if (bouncelevel > 4)
        {
          printf ("E550 Message rejected by server\n");
          _log (bformat
                ("Rejecting on SPF_RESULT_PERMERROR, %s", comment->data));
        }
      else
        {
          _log (bformat
                ("Accepting on SPF_RESULT_PERMERROR, %s", comment->data));
        }
      exit (0);
      break;
    case SPF_RESULT_INVALID:   //This should not happen
    default:
      _log (bformat ("SPF_RESULT_INVALID, %s", comment->data));
      exit (0);
      break;
    };
  //There is no point bothering to free memory, since the process
  //ends here and memory is freed anyhow. Don't be anal.
  exit (0);
}
