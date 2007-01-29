#include <stdlib.h>

#include <check.h>

#include <cdb.h>
#include "../cdbutils.h"


bstring testdir;

START_TEST (test_cdb1)
{
    /* unit test code */
    bstring fname=bformat("%s/test_data.cdb",testdir->data);
    bstring addr=bfromcstr("JOE@whatever.org");

    fail_unless ( lineincdb (addr,fname) == 1 );

}
END_TEST

START_TEST (test_cdb2)
{
    bstring fname=bformat("%s/test_data.cdb",testdir->data);
    bstring addr=bfromcstr("jane@whatever.org");
    fail_unless ( lineincdb (addr,fname) == 0 );
}
END_TEST

START_TEST (test_cdb3)
{
    bstring fname=bformat("%s/notexisting",testdir->data);
    bstring addr=bfromcstr("JOE@whatever.org");
    fail_unless ( lineincdb (addr,fname) == -1 );
}
END_TEST


Suite *cdb_suite(void)
{
    Suite *s=suite_create("CDB");
    TCase *cdb_core=tcase_create("Core");
    tcase_add_test(cdb_core, test_cdb1);
    tcase_add_test(cdb_core, test_cdb2);
    tcase_add_test(cdb_core, test_cdb3);
    suite_add_tcase(s,cdb_core);
    return s;

}


int
main (void)
{
    testdir=bfromcstr(getenv("TDIR"));
    int number_failed;
    Suite *s=cdb_suite();
    SRunner *sr=srunner_create(s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


