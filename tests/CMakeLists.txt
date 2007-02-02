PROJECT (tests)
FIND_LIBRARY (LIB_CHECK check)

IF (LIB_CHECK)

IF (LIB_CDB)
ADD_EXECUTABLE ( test_cdb test_cdb.c ../cdbutils.c ../bstrlib.c )
TARGET_LINK_LIBRARIES (test_cdb ${LIB_CDB} ${LIB_CHECK})
ENDIF (LIB_CDB)

ADD_EXECUTABLE ( test_utils test_utils.c ../utils.c ../bstrlib.c )
TARGET_LINK_LIBRARIES (test_utils ${LIB_CHECK})

ENDIF (LIB_CHECK)
