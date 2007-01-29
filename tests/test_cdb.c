#include <check.h>

#include <cdb.h>
#include "../cdbutils.h"

START_TEST (test_cdb)
{
    /* unit test code */
    bstring fname=bfromcstr("test_data.cdb");
    bstring addr=bfromcstr("JOE@whatever.org");

    fail_unless ( lineincdb (addr,fname) == 1 );

    addr=bfromcstr("jane@whatever.org");

    fail_unless ( lineincdb (addr,fname) == 0 );

    fname=bfromcstr("notexisting");

    fail_unless ( lineincdb (addr,fname) == -1 );

}
END_TEST

int
main (void)
{
    return 0;
}


