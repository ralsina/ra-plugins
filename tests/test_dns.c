#include <stdlib.h>

#include <check.h>

#include "../dnsutils.h"


bstring testdir;

START_TEST (test_mailservers_a)
{
  struct bstrList *list;
  int r = mailservers (bfromcstr ("planetkde.org"), &list);
  fail_unless (r == 1 && biseq (list->entry[0], bfromcstr ("67.18.167.98")));
}

END_TEST
START_TEST (test_mailservers_mx)
{
  struct bstrList *list;
  int r = mailservers (bfromcstr ("netmanagers.com.ar"), &list);
  fail_unless (r == 1
               && biseq (list->entry[0], bfromcstr ("mx1.netmanagers.com.ar")));

}

END_TEST
START_TEST (test_mailservers_unknown)
{
  struct bstrList *list;
  int r = mailservers (bfromcstr ("idont.exist"), &list);
  fail_unless (r == -2);

}
END_TEST Suite *
dns_suite (void)
{
  Suite *s = suite_create ("DNS");
  TCase *dns_core = tcase_create ("Core");
  tcase_add_test (dns_core, test_mailservers_a);
  tcase_add_test (dns_core, test_mailservers_mx);
  tcase_add_test (dns_core, test_mailservers_unknown);
  suite_add_tcase (s, dns_core);
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
  Suite *s = dns_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
