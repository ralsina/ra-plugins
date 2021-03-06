cmake_minimum_required(VERSION 2.6.2)
if(POLICY CMP0015)
   cmake_policy(SET CMP0015 OLD)
endif(POLICY CMP0015)
PROJECT (ra-plugins)
SET (VERSION "0.2.10" )
# SET (CMAKE_VERBOSE_MAKEFILE ON)

####################################################################
## INSTALLATION DIRECTORIES
##
## IMPORTANT: If you are a packager, or simply want to install 
## this in different directories, you only need to edit these
## definitions
####################################################################

SET (QMAIL_PLUGIN_DIR /var/qmail/plugins)
SET (DOC_DIR /usr/share/doc/ra-plugins-${VERSION} )
SET (MAN_DIR /usr/man/man8)
SET (BIN_DIR /usr/bin)

####################################################################
## Search for libraries and headers
####################################################################

INCLUDE (CheckLibraryExists)

FIND_LIBRARY(LIB_SPF2 spf2)

FIND_LIBRARY(LIB_TDB tdb)

FIND_LIBRARY(LIB_CDB cdb)

FIND_LIBRARY(LIB_CHECK check)

FIND_LIBRARY(LIB_MYSQL mysqlclient PATHS /usr/lib/mysql)

FIND_LIBRARY(LIB_VPOPMAIL vpopmail PATHS /home/vpopmail/lib)

FIND_PATH(VPOPMAIL_INCLUDE_DIR vpopmail.h /usr/include/ 
          /usr/local/include/
          /home/vpopmail/include)

INCLUDE (FindGTK)

LINK_DIRECTORIES ( smtp udns )

###################################################################
## Test suite
###################################################################

ADD_CUSTOM_TARGET ( check )
SET ( TESTDIR ../tests )

IF (LIB_CHECK)

  FOREACH (TEST test_utils test_dns)
    ADD_CUSTOM_TARGET ( DO_${TEST} DEPENDS tests/${TEST}
    COMMAND TDIR=${TESTDIR} tests/${TEST}
    )
    ADD_DEPENDENCIES ( check DO_${TEST} )
  ENDFOREACH (TEST test_cdb test_utils )
  
  IF (LIB_CDB)
    ADD_CUSTOM_TARGET ( DO_test_cdb DEPENDS tests/test_cdb
    COMMAND TDIR=${TESTDIR} tests/test_cdb
    )
    ADD_DEPENDENCIES ( check DO_test_cdb )
  ENDIF (LIB_CDB)
  
  ADD_DEPENDENCIES ( check udns smtp )

ENDIF (LIB_CHECK)

####################################################################
## Targets for pre-distributuion
##
## manpages: Builds man pages
##
####################################################################

## This is pretty evil, but I want the .8 files to
## be generated always in-place in the sources
## directory, then copied here.
##
## Since those files should always be shipped along
## with the sources...

ADD_CUSTOM_TARGET ( manpages ALL )
FILE (GLOB MANSOURCES *.man.txt)
FOREACH ( MANSRC ${MANSOURCES} )
  GET_FILENAME_COMPONENT ( MANDST ${MANSRC} NAME_WE )
  GET_FILENAME_COMPONENT ( MANPTH ${MANSRC} PATH )
  
  ADD_CUSTOM_TARGET ( ${MANDST}_8 DEPENDS ${MANPTH}/${MANDST}.8_ 
    COMMAND cp ${MANPTH}/${MANDST}.8_ ${MANDST}.8
    COMMAND gzip -f ${MANDST}.8  )
  ADD_DEPENDENCIES ( manpages ${MANDST}_8 )
  
  ADD_CUSTOM_COMMAND(
    OUTPUT ${MANPTH}/${MANDST}.8_
    COMMAND LANG=C txt2man -t ${MANDST} 
                      < ${MANSRC} > ${MANPTH}/${MANDST}.8_
    DEPENDS ${MANSRC} )
ENDFOREACH ( MANSRC ${MANSOURCES} )

####################################################################
## Build included copies of udns and libsmtp
####################################################################

ADD_SUBDIRECTORY( udns )
ADD_SUBDIRECTORY( smtp )
ADD_SUBDIRECTORY( tests )

####################################################################
## The Better String Library
####################################################################

SET (BSTRLIB_SRCS bstrlib.c)

####################################################################
## authlogger plugin
## requires nothing
####################################################################

ADD_EXECUTABLE (authlogger authlogger.c utils.c ${BSTRLIB_SRCS})
INSTALL (TARGETS authlogger DESTINATION ${QMAIL_PLUGIN_DIR})

####################################################################
## localmail plugin
## requires nothing
####################################################################

ADD_EXECUTABLE (localmail localmail.c utils.c ${BSTRLIB_SRCS})
INSTALL (TARGETS localmail DESTINATION ${QMAIL_PLUGIN_DIR})

####################################################################
## msa plugin
## requires nothing
####################################################################

ADD_EXECUTABLE (msa msa.c utils.c ${BSTRLIB_SRCS})
INSTALL (TARGETS msa DESTINATION ${QMAIL_PLUGIN_DIR})

####################################################################
## rblchecks plugin
## requires libudns
####################################################################

ADD_EXECUTABLE ( rblchecks rblchecks.c utils.c dnsutils.c
                 ${BSTRLIB_SRCS})
TARGET_LINK_LIBRARIES (rblchecks libudns.a)
INSTALL (TARGETS rblchecks DESTINATION ${QMAIL_PLUGIN_DIR})
ADD_DEPENDENCIES (rblchecks udns)

####################################################################
## senderchecks plugin
## requires vpopmail
####################################################################

IF (LIB_VPOPMAIL AND LIB_MYSQL)
  ADD_EXECUTABLE (senderchecks senderchecks.c utils.c
                  vpoputils.c 
                  ${BSTRLIB_SRCS})
  SET_TARGET_PROPERTIES ( senderchecks PROPERTIES SKIP_BUILD_RPATH TRUE )
  TARGET_LINK_LIBRARIES (senderchecks ${LIB_VPOPMAIL} ${LIB_MYSQL})
  INSTALL (TARGETS senderchecks DESTINATION ${QMAIL_PLUGIN_DIR})
ENDIF (LIB_VPOPMAIL AND LIB_MYSQL)

####################################################################
## senderchecksfile plugin
## requires nothing
####################################################################

ADD_EXECUTABLE (senderchecksfile senderchecksfile.c utils.c 
                ${BSTRLIB_SRCS})
INSTALL (TARGETS senderchecksfile DESTINATION ${QMAIL_PLUGIN_DIR})

####################################################################
## spfchecks plugin
## requires nothing
####################################################################

IF (LIB_SPF2)
  ADD_EXECUTABLE (spfchecks spfchecks.c utils.c ${BSTRLIB_SRCS})
  TARGET_LINK_LIBRARIES (spfchecks ${LIB_SPF2})
  INSTALL (TARGETS spfchecks DESTINATION ${QMAIL_PLUGIN_DIR})
ENDIF (LIB_SPF2)

####################################################################
## tarpit plugin
## requires nothing
####################################################################

ADD_EXECUTABLE (tarpit tarpit.c utils.c ${BSTRLIB_SRCS})
INSTALL (TARGETS tarpit DESTINATION ${QMAIL_PLUGIN_DIR})

####################################################################
## dotblind plugin
## requires nothing
####################################################################

ADD_EXECUTABLE (dotblind dotblind.c utils.c ${BSTRLIB_SRCS})
INSTALL (TARGETS dotblind DESTINATION ${QMAIL_PLUGIN_DIR})

####################################################################
## plusaliases plugin
## requires nothing
####################################################################

ADD_EXECUTABLE (plusaliases plusaliases.c utils.c ${BSTRLIB_SRCS})
INSTALL (TARGETS plusaliases DESTINATION ${QMAIL_PLUGIN_DIR})

###################################################################
## ipthrottle plugin
## requires librelay (part of relayd)
###################################################################

FIND_LIBRARY(LIB_RELAY relay)
IF (LIB_RELAY)
  ADD_EXECUTABLE (ipthrottle ipthrottle.c utils.c ${BSTRLIB_SRCS})
  TARGET_LINK_LIBRARIES (ipthrottle relay)
  INSTALL (TARGETS ipthrottle DESTINATION ${QMAIL_PLUGIN_DIR})
ENDIF(LIB_RELAY)

###################################################################
## userthrottle plugin
## requires librelay (part of relayd)
###################################################################

FIND_LIBRARY(LIB_RELAY relay)
IF (LIB_RELAY)
  ADD_EXECUTABLE (userthrottle userthrottle.c utils.c ${BSTRLIB_SRCS})
  TARGET_LINK_LIBRARIES (userthrottle relay)
  INSTALL (TARGETS userthrottle DESTINATION ${QMAIL_PLUGIN_DIR})
ENDIF(LIB_RELAY)

####################################################################
## checkback plugin
## requires udns, libsmtp, glib
####################################################################

IF ( GTK_FOUND AND LIB_TDB )
  INCLUDE_DIRECTORIES ( ${GTK_INCLUDE_DIR} )
  ADD_EXECUTABLE ( checkback checkback.c utils.c dnsutils.c 
                   ${BSTRLIB_SRCS})
  SET_TARGET_PROPERTIES ( checkback PROPERTIES SKIP_BUILD_RPATH TRUE )
  TARGET_LINK_LIBRARIES ( checkback libudns.a libsmtp.a glib tdb)
  ADD_DEPENDENCIES ( checkback udns smtp )
  INSTALL ( TARGETS checkback DESTINATION ${QMAIL_PLUGIN_DIR})
  
# Helper to purge cache
  ADD_EXECUTABLE ( checkbackpurge checkbackpurge.c utils.c ${BSTRLIB_SRCS})
  SET_TARGET_PROPERTIES ( checkbackpurge PROPERTIES SKIP_BUILD_RPATH TRUE )
  TARGET_LINK_LIBRARIES ( checkbackpurge tdb)
  INSTALL ( TARGETS checkbackpurge DESTINATION ${BIN_DIR})
  
  
ENDIF ( GTK_FOUND AND LIB_TDB )

####################################################################
## mfdnschecks plugin.
## requires libudns
####################################################################
ADD_EXECUTABLE (mfdnschecks mfdnschecks.c utils.c dnsutils.c 
                ${BSTRLIB_SRCS})
SET_TARGET_PROPERTIES ( mfdnschecks PROPERTIES SKIP_BUILD_RPATH TRUE )
TARGET_LINK_LIBRARIES (mfdnschecks libudns.a)
INSTALL (TARGETS mfdnschecks DESTINATION ${QMAIL_PLUGIN_DIR})
ADD_DEPENDENCIES (mfdnschecks udns)

####################################################################
## rcptchecksfile plugin
## requires nothing
####################################################################

ADD_EXECUTABLE (rcptchecksfile rcptchecksfile.c utils.c 
                ${BSTRLIB_SRCS})
INSTALL (TARGETS rcptchecksfile DESTINATION ${QMAIL_PLUGIN_DIR})

####################################################################
## rcptcheckscdb plugin
## requires tinycdb
####################################################################

IF (LIB_CDB)
  ADD_EXECUTABLE (rcptcheckscdb rcptcheckscdb.c utils.c cdbutils.c
                  ${BSTRLIB_SRCS})
  TARGET_LINK_LIBRARIES (rcptcheckscdb ${LIB_CDB})
  INSTALL (TARGETS rcptcheckscdb DESTINATION ${QMAIL_PLUGIN_DIR})
ENDIF (LIB_CDB)

###################################################################
## rcptchecks plugin
## requires vpopmail and mysql (at least here)
###################################################################

IF (LIB_VPOPMAIL AND LIB_MYSQL)
  INCLUDE_DIRECTORIES ( ${VPOPMAIL_INCLUDE_DIR} )
  ADD_EXECUTABLE (rcptchecks rcptchecks.c utils.c vpoputils.c 
                  ${BSTRLIB_SRCS})
  SET_TARGET_PROPERTIES ( rcptchecks PROPERTIES SKIP_BUILD_RPATH TRUE )
  TARGET_LINK_LIBRARIES (rcptchecks ${LIB_VPOPMAIL} ${LIB_MYSQL})
  INSTALL (TARGETS rcptchecks DESTINATION ${QMAIL_PLUGIN_DIR})
ENDIF (LIB_VPOPMAIL AND LIB_MYSQL)

###################################################################
## Installation
###################################################################

FILE(GLOB MANPAGES *.8.gz)
INSTALL (FILES ${MANPAGES} DESTINATION ${MAN_DIR} )

INSTALL ( FILES COPYING CHANGELOG ${MANSOURCES} DESTINATION ${DOC_DIR} )

