
#ifndef LIB_SMTP_H

#define LIB_SMTP_H

#ifndef __G_LIB_H__
  #include <glib.h>
#endif


/* These flags show what the server can do */

#define LIBSMTP_HAS_TLS	1
#define LIBSMTP_HAS_8BIT	2
#define LIBSMTP_HAS_AUTH	4
#define LIBSMTP_HAS_PIPELINING	8
#define LIBSMTP_HAS_SIZE	16
#define LIBSMTP_HAS_DSN		32
#define LIBSMTP_HAS_ETRN	64
#define LIBSMTP_HAS_ENHANCEDSTATUSCODES	128

/* Recipient types for libsmtp_add_recipient */

#define LIBSMTP_REC_MAX	2

#define LIBSMTP_REC_TO	0
#define LIBSMTP_REC_CC	1
#define LIBSMTP_REC_BCC	2

/* SMTP transaction stages */

#define LIBSMTP_NOCONNECT_STAGE	0
#define LIBSMTP_CONNECT_STAGE	1
#define LIBSMTP_GREET_STAGE	2
#define LIBSMTP_HELLO_STAGE	3

#define LIBSMTP_SENDER_STAGE	16
#define LIBSMTP_RECIPIENT_STAGE	17
#define LIBSMTP_DATA_STAGE	18
#define LIBSMTP_HEADERS_STAGE	19
#define LIBSMTP_MIMEHEADERS_STAGE	20
#define LIBSMTP_BODY_STAGE	21

#define LIBSMTP_FINISHED_STAGE	128
#define LIBSMTP_QUIT_STAGE	256

/* Module types */

#define LIBSMTP_BODY_MODULE	0
#define LIBSMTP_MIME_MODULE	1
#define LIBSMTP_HEADER_MODULE	2
#define LIBSMTP_DIALOGUE_MODULE	3

/* These are the error definitions */

/* Error codes below 1024 are fatal errors - the socket will be closed */
#define LIBSMTP_NOERR		0
#define LIBSMTP_SOCKETNOCREATE	1
#define LIBSMTP_HOSTNOTFOUND	2
#define LIBSMTP_CONNECTERR	3
#define LIBSMTP_ERRORREADFATAL	4
#define LIBSMTP_NOTWELCOME	5
#define LIBSMTP_WHATSMYHOSTNAME	6
#define LIBSMTP_ERRORSENDFATAL	7
#define LIBSMTP_WONTACCEPTSENDER	8
#define LIBSMTP_REJECTBODY	9
#define LIBSMTP_WONTACCEPTDATA	10

/* Codes >= 1024 are errors that are not fatal to the whole SMTP session */
#define LIBSMTP_ERRORREAD	1024
#define LIBSMTP_ERRORSEND	1025
#define LIBSMTP_BADARGS		1026
#define LIBSMTP_WONTACCEPTREC	1027
#define LIBSMTP_BADSTAGE	1028
#define LIBSMTP_REJECTQUIT	1029

/* Codes > 2048 are MIME errors and are defined in libsmtp_mime.h */

#define LIBSMTP_UNDEFERR	10000 /* ErrorCode was undefined!! */
/* This structure defines one libsmtp session */

struct libsmtp_session_struct {
  int serverflags;	/* Server capability flags */
  int socket;		/* socket handle */

  GString *From;	/* From address */
  GList *To;		/* All recipients addresses */
  GList *CC;		/* All Carbon Copy recipients addresses */
  GList *BCC;		/* All Blind Carbon Copy recipients addresses */
  int NumFailedTo;	/* number of rejected recipients */
  int NumFailedCC;	/* number of rejected CC recipients */
  int NumFailedBCC;	/* number of rejected BCC recipients */
  GList *ToResponse;	/* List of failed recipients containing the response for
  			   each failure */
  GList *CCResponse;	/* The same for CC recipients */
  GList *BCCResponse;	/* And for BCC recipients */

  GString *Subject;	/* Mail subject */
  GString *LastResponse;	/* Last SMTP response string from server */
  int LastResponseCode;	/* Last SMTP response code from server */
  int ErrorCode;	/* Internal libsmtp error code from last error */
  GString *ErrorModule;	/* Module were error was caused */
  int Stage;		/* SMTP transfer stage */

  unsigned int DialogueSent;	/* Number of SMTP dialogue lines sent */
  unsigned int DialogueBytes;	/* Bytes of SMTP dialogue data sent */
  unsigned int HeadersSent;  	/* Number of header lines sent */
  unsigned int HeaderBytes;	/* Bytes of header data sent */
  unsigned long int BodyBytes;	/* Bytes of body data sent */
  
  #ifdef WITH_MIME
    GNode *Parts;		/* N-Tree of body parts (MIME stuff) */
    int NumParts;		/* Number of body parts */
    struct libsmtp_part_struct *PartNow;	/* Part we are sending now */
    GNode *PartNowNode;		/* Node of the part we are just sending */
  #endif
};

struct libsmtp_session_struct *libsmtp_session_initialize (void);

int libsmtp_connect (char *, unsigned int, unsigned int, struct libsmtp_session_struct *);

int libsmtp_errno(struct libsmtp_session_struct *);

const char *libsmtp_strerr (struct libsmtp_session_struct *);

int libsmtp_add_recipient (int, char *, struct libsmtp_session_struct *);

int libsmtp_set_environment (char *, char *, unsigned int, struct libsmtp_session_struct *);

int libsmtp_dialogue_send (char *, struct libsmtp_session_struct *);

int libsmtp_dialogue (struct libsmtp_session_struct *);

int libsmtp_header_send (char *, struct libsmtp_session_struct *);

int libsmtp_headers (struct libsmtp_session_struct *);

int libsmtp_body_send_raw (char *, unsigned long int, struct libsmtp_session_struct *);

int libsmtp_body_end (struct libsmtp_session_struct *);

int libsmtp_quit (struct libsmtp_session_struct *);

int libsmtp_close (struct libsmtp_session_struct *);

int libsmtp_free (struct libsmtp_session_struct *);

/* internal functions */

int libsmtp_int_send (GString *, struct libsmtp_session_struct *, int);

int libsmtp_int_read (GString *, struct libsmtp_session_struct *, int);

int libsmtp_int_send_body (char *, unsigned long int, struct libsmtp_session_struct *);

#endif  /* LIB_SMTP_H */
