#include <stdlib.h>

#include <check.h>

#include "../utils.h"


bstring testdir;

START_TEST (test_lineinfile)
{
  /* Positive match */
  bstring fname;
  bstring addr;

  fname = bformat ("%s/test_data.txt", testdir->data);
  addr = bfromcstr ("JOE@whatever.org");

  fail_unless (lineinfile (addr, fname) == 1, "Positive match fails");

  /* Negative match */
  addr = bfromcstr ("jane@whatever.org");
  fail_unless (lineinfile (addr, fname) == 0, "Negative match fails");

  /* Positive match on last line without a EOL */
  addr = bfromcstr ("JACK@whatever.org");
  fail_unless (lineinfile (addr, fname) == 1, "Last line without EOL fails");

  /* Missing file */
  fname = bformat ("%s/notexisting", testdir->data);
  addr = bfromcstr ("JOE@whatever.org");
  fail_unless (lineinfile (addr, fname) == -1, "Missing file fails");

}

END_TEST
START_TEST (test_checkaddr)
{
  /* Check email addresses validity && splitting */
  bstring addr, user, domain;
  int r;
  addr = bfromcstr ("addr@domain.com");
  r = checkaddr (addr, &user, &domain);
  fail_unless (r == 1 &&
               (biseq (user, bfromcstr ("addr")) == 1) &&
               (biseq (domain, bfromcstr ("domain.com")) == 1),
               "Checkaddr test 1");
  bdestroy (addr);
  bdestroy (user);
  bdestroy (domain);

  addr = bfromcstr ("addr@");

  r = checkaddr (addr, &user, &domain);
  fail_unless (r == 0, "Checkaddr test 2");
  bdestroy (addr);
  bdestroy (user);
  bdestroy (domain);


  addr = bfromcstr ("addr");
  r = checkaddr (addr, &user, &domain);
  fail_unless (r == 0, "Checkaddr test 3");
  bdestroy (addr);
  bdestroy (user);
  bdestroy (domain);


  addr = bfromcstr ("@domain.com");
  r = checkaddr (addr, &user, &domain);
  fail_unless (r == 0, "Checkaddr test 4");

  bdestroy (addr);
  bdestroy (user);
  bdestroy (domain);
}
END_TEST Suite *
utils_suite (void)
{
  Suite *s = suite_create ("UTILS");
  TCase *utils_core = tcase_create ("Core");
  tcase_add_test (utils_core, test_lineinfile);
  tcase_add_test (utils_core, test_checkaddr);
  suite_add_tcase (s, utils_core);
  return s;

}


int
main (void)
{
  testdir = bfromcstr (getenv ("TDIR"));
  if (!testdir)
    {
      testdir = bfromcstr (".");
    }
  int number_failed;
  Suite *s = utils_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
