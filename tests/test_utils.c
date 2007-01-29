#include <stdlib.h>

#include <check.h>

#include "../utils.h"


bstring testdir;

START_TEST (test_utils1)
{
  /* Positive match */
  bstring fname = bformat ("%s/test_data.txt", testdir->data);
  bstring addr = bfromcstr ("JOE@whatever.org");

  fail_unless (lineinfile (addr, fname) == 1);

}
END_TEST

START_TEST (test_utils2)
{
  /* Negative match */
  bstring fname = bformat ("%s/test_data.txt", testdir->data);
  bstring addr = bfromcstr ("jane@whatever.org");
  fail_unless (lineinfile (addr, fname) == 0);
}
END_TEST

START_TEST (test_utils3)
{
  /* Missing file */
  bstring fname = bformat ("%s/notexisting", testdir->data);
  bstring addr = bfromcstr ("JOE@whatever.org");
  fail_unless (lineinfile (addr, fname) == -1);
}
END_TEST

START_TEST (test_utils4)
{
  /* Positive match on last line without a EOL */
  bstring fname = bformat ("%s/notexisting", testdir->data);
  bstring addr = bfromcstr ("JACK@whatever.org");
  fail_unless (lineinfile (addr, fname) == -1);
}
END_TEST

Suite * utils_suite (void)
{
  Suite *s = suite_create ("UTILS");
  TCase *utils_core = tcase_create ("Core");
  tcase_add_test (utils_core, test_utils1);
  tcase_add_test (utils_core, test_utils2);
  tcase_add_test (utils_core, test_utils3);
  tcase_add_test (utils_core, test_utils4);
  suite_add_tcase (s, utils_core);
  return s;

}


int
main (void)
{
  testdir = bfromcstr (getenv ("TDIR"));
  int number_failed;
  Suite *s = utils_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
