PROJECT (smtp)
INCLUDE (FindGTK)
INCLUDE_DIRECTORIES ( ${GTK_INCLUDE_DIR} )
ADD_LIBRARY ( smtp STATIC
                     libsmtp_comm.c
                     libsmtp_connection.c
                     libsmtp_data.c
                     libsmtp_error.c )
