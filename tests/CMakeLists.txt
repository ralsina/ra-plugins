PROJECT (tests)
FIND_LIBRARY (LIB_CHECK check)
LINK_DIRECTORIES ( ../smtp ../udns )


IF (LIB_CHECK)

IF (LIB_CDB)
ADD_EXECUTABLE ( test_cdb test_cdb.c ../cdbutils.c ../bstrlib.c )
TARGET_LINK_LIBRARIES (test_cdb ${LIB_CDB} ${LIB_CHECK})
ENDIF (LIB_CDB)

ADD_EXECUTABLE ( test_utils test_utils.c ../utils.c ../bstrlib.c )
TARGET_LINK_LIBRARIES (test_utils ${LIB_CHECK})

ADD_EXECUTABLE ( test_dns test_dns.c ../dnsutils.c ../utils.c ../bstrlib.c )
TARGET_LINK_LIBRARIES (test_dns libudns.a ${LIB_CHECK} )

ENDIF (LIB_CHECK)
