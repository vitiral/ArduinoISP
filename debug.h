#ifndef __debug_h__
#define __debug_h__

#include <errno.h>
#include <string.h>

/*
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) sprintln(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif


#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) sprintln(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) sprintln(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) sprintln(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }  //log check

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

*/

#define ERR_ASSERTION 1
#define ERR_TIMEOUT 2
#define ERR_UNKNOWN 255

extern int derr;
extern char *errmsg;
char *strerrno(int err);
void printerrno();

#define sprint(M, ...) Serial.print((M), ##__VA_ARGS__)
#define sprintln(M, ...) Serial.println((M), ##__VA_ARGS__)
#define swrite(M) Serial.write((M))

#define sprintinfo() sprint(derr); swrite(' '); sprint(__FILE__); swrite(':'); sprint(__LINE__);

#ifdef NDEBUG
#define debug(M, ...)
#define log_err(M, ...)
#else
#define debug(M, ...) sprintln(""); sprint("DBG: ("); sprintinfo(); sprint(")| "); sprintln((M), ##__VA_ARGS__);
#define log_err(M, ...) sprintln(""); sprint("[ERR]("); sprintinfo(); sprint(")|"); sprint(strerrno(errno)); sprint((M), ##__VA_ARGS__); errno = 0
#endif

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }  //log check

#define assert(A) if(!(A)) {if(!derr){errno=ERR_ASSERTION; derr = 1; errmsg="";} log_err("Ass"); goto error;}

#define noerr() if(derr) goto error;

#define noerr_log() if(derr){log_err(("NE")); goto error;}

#define clearerror() errno = 0; derr = 0; errmsg = "";

#endif
