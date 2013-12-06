/*

 LICENSE: This code is released under the GNU GPL v3 or greater. For more info see:
 <http://www.gnu.org/licenses/gpl.html>
 Copyright: Garrett Berg cloudformdesign.com garrett@cloudformdesign.com
 Loosly based on code from the fantastic "Learn C the Hard Way",  Zed A. Shaw
 http://c.learncodethehardway.org/book/ex20.html
 
 Arduino Debug Library
 This is the arduino debug library, meant to make debugging and error 
 handling easier.
 
 Intended Use:
 There are many times where you want to do error checking and simultaniously provide debugging.
 This library helps significantly. The intended use is to make an assertion for your failure
 point, and then in the "error:" goto define errno and derr to be what your values are.
 
 You then use noerr_log() to follow the stack trace down, so you can see what called what,
 while simultaniously doing the necessary error checking.
 
 Example:
 void print_user_int(){
 int user_int = get_user_int(); // expects to get an integer from the user
 noerr_log();  // if there was an error, log it and go to error:
 Serial.println(user_int);
 return;
 error:
 return;
 }
 
 int get_user_int(){
   while(!Serial.available());
   char c = Serial.read();
   // asserts that the character is an integer character (0-9), if not print
   // out helpful message
   assert_raise(isint(c), ERR_TYPE); 
   return char_to_int(c);
 error:
   return -1;
 }
 
 
 Preprossessor Definitions
 #define DEBUG ::
 define DEBUG before importing this module to print things, otherwise error checking, etc will be silent.

 
 Global Variables:
 errno :: from "errno.h". Specifys error type.
 derr  :: debug error, specifys error type. Kept separate for printing convienience
 
 Macro Overview:
 assert(A) ::
 Assert that the value is true. If it is not true, then derr and errno = ERR_ASSERT
 Also logs error with message "AS"
 Note: requires "error:" defined for goto
 
 raise(E) ::
 Raises the error given, logging it.
 Note: requires "error:" defined for goto
 
 assert_raise(A, E) ::
 if A is false, raise(E)
 Note: requires "error:" defined for goto
 
 noerr() ::
 same as assert(!derr)
 Note: requires "error:" defined for goto
 
 noerr_log() ::
 same as noerr() but provides a log as well with message "NE"
 Note: requires "error:" defined for goto
 
 clearerror() ::
 clears error specifiers (derr = 0; errno = 0)
 
 debug(M, ...) ::
 prints out: [DBG](file:line)| M
 
 log_err(M, ...) ::
 prints out [ERR](derr:file:line)|strerrno(errno) M
 
*/

#ifndef __debug_h__
#define __debug_h__

#include <errno.h>
#include <string.h>

#define ERR_BASE          1 // Base error
#define ERR_TIMEOUT       2 // TimeoutErr
#define ERR_SERIAL        3 // SerialErr
#define ERR_SPI           4 // Spierr
#define ERR_I2C           5 // I2cErr
#define ERR_COMMUNICATION 6 // ComErr
#define ERR_CONFIG        7 // ConfigErr
#define ERR_PIN           8 // PinErr
#define ERR_INPUT         9 // InputErr

#define ERR_TYPE          20 // TypeErr
#define ERR_VALUE         21 // ValueErr
#define ERR_ASSERT        22 // AssertErr

#define ERR_EMPTY   254  //nothing printed
#define ERR_UNKNOWN 255  //unknown

extern int derr;
extern char *errmsg;

#define sprint(M, ...) Serial.print((M), ##__VA_ARGS__)
#define sprintln(M, ...) Serial.println((M), ##__VA_ARGS__)
#define swrite(M) Serial.write((M))


#ifdef DEBUG
void DBG_printerrno();
//void DBG_sprintinfo(char *file, unsigned int line);
void dbg_start_debug(char *file, unsigned int line);
void dbg_logme(char *file, unsigned int line);

#define debug(M, ...) //dbg_start_debug(__FILE__, __LINE__); sprintln((M), ##__VA_ARGS__);

#define log_err() dbg_logme(__FILE__, __LINE__); //sprintln((M), ##__VA_ARGS__); errno = 0
#else
#define debug(M, ...)
#define log_err()
#endif

#define assert(A) if(!(A)) {errno=ERR_ASSERT; derr = ERR_ASSERT; errmsg=""; log_err(""); goto error;}

#define raise(E) errno = (E); derr = (E); errmsg = ""; log_err(); goto error;

#define assert_raise(A, E) if(!(A)) {raise((E))}

#define noerr() if(derr) goto error;

#define noerr_log() if(derr){log_err(); goto error;}

#define clearerror() errno = 0; derr = 0; errmsg = "";

#endif

