#include <stdlib.h>

#include <check.h>

#include "../dnsutils.h"


bstring testdir;

START_TEST (test_mailservers)
{
}
END_TEST

Suite *
dns_suite (void)
{
  Suite *s = suite_create ("DNS");
  TCase *dns_core = tcase_create ("Core");
  tcase_add_test (dns_core, test_mailservers);
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
