cmake_minimum_required(VERSION 2.0)
PROJECT (smtp)
INCLUDE (FindGTK)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB_PKG glib-2.0)
INCLUDE_DIRECTORIES ( ${GTK_INCLUDE_DIR} ${GLIB_PKG_INCLUDE_DIRS})

ADD_LIBRARY ( smtp STATIC
                     libsmtp_comm.c
                     libsmtp_connection.c
                     libsmtp_data.c
                     libsmtp_error.c )
