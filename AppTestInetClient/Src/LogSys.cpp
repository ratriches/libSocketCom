
#include <string>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#include "LogSys.h"

/*############# Definicoes locais ###########################################*/

#define TMPBUF_LENGHT   ((4*1024)-1)			/**< Tamanho do buffer temporatio de log */


/*############# Variaveis Locais ############################################*/
// NOTE: from syslog.h
// #define	LOG_EMERG	0	/* system is unusable */
// #define	LOG_ALERT	1	/* action must be taken immediately */
// #define	LOG_CRIT	2	/* critical conditions */
// #define	LOG_ERR		3	/* error conditions */
// #define	LOG_WARNING	4	/* warning conditions */
// #define	LOG_NOTICE	5	/* normal but significant condition */
// #define	LOG_INFO	6	/* informational */
// #define	LOG_DEBUG	7	/* debug-level messages */

static int cfgLv = 0;
static char logChar[] = "MACEWNID"; // sequencia de defines acima
static char TmpBuf[TMPBUF_LENGHT+1];			/**< Buffer de log */
static bool init_ = true;
  
/*############# Implementacao Local #########################################*/
/**
 * @return milliseconds
 */
static uint32_t get_UpTime() {
  struct timespec spec;
  if (clock_gettime(1, &spec) == -1) { /* 1 is CLOCK_MONOTONIC */
    return 0;
  }

  return spec.tv_sec * 1000 + spec.tv_nsec / 1e6;
}


/*############# Implementacao Exportada #####################################*/
/*------------------------------------------------------------------------*//**
 @fn			int Log_Init(const char* name)
*//*-------------------------------------------------------------------------*/
int Log_Init(const char* name) {
  if (init_) {
    openlog(name, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    init_ = false;
  } else {
    printf("Log_Init Ops: %s!\n", name);
  }
  // printf("Log_Init: %s!\n", name);
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int Log_Deinit(void)
*//*-------------------------------------------------------------------------*/
int Log_Deinit(void) {
  closelog();
  init_ = true;
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int Log_SetLevel(uint8_t lev)
*//*-------------------------------------------------------------------------*/
int Log_SetLevel(uint8_t lev) { // map bit
  cfgLv = lev;
  // setlogmask(LOG_UPTO (LOG_DEBUG));
  setlogmask(lev);
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int Log_Printf(uint8_t Lev, const char* fmt, ...)
*//*-------------------------------------------------------------------------*/
int Log_Printf(uint8_t Lev, const char* fmt, ...) {
  va_list args;
  
  if (init_) return 0;
  if ((cfgLv & (1<<Lev) ) == 0) {
    return 0;
  }
  
  va_start(args, fmt);
  vsnprintf(TmpBuf, sizeof(TmpBuf)-1, fmt, args);
  //printf(TmpBuf);
  syslog(Lev, "%c[T%d] %s", logChar[Lev], get_UpTime(), TmpBuf);

  va_end(args);
  return 1;
}

/*------------------------------------------------------------------------*//**
 @fn			int Log_PrintfL(uint8_t Lev, uint32_t line, const char* fmt, ...)
*//*-------------------------------------------------------------------------*/
int Log_PrintfL(uint8_t Lev, uint32_t line, const char* fmt, ...) {
  va_list args;
  
  if (init_) return 0;
  if ((cfgLv & (1<<Lev) ) == 0) {
    return 0;
  }
  
  va_start(args, fmt);
  vsnprintf(TmpBuf, sizeof(TmpBuf)-1, fmt, args);
  //printf(TmpBuf);
  syslog(Lev, "%c[L%dT%d] %s", logChar[Lev], line, get_UpTime(), TmpBuf);

  va_end(args);
  return 1;
}

/*------------------------------------------------------------------------*//**
 @fn			int Log_PrintfFL(uint8_t Lev, const char* fname, uint32_t line, const char* fmt, ...)
*//*-------------------------------------------------------------------------*/
int Log_PrintfFL(uint8_t Lev, const char* fname, uint32_t line, const char* fmt, ...) {
  va_list args;
  
  if (init_) return 0;
  if ((cfgLv & (1<<Lev) ) == 0) {
    return 0;
  }
  
  const char * f = fname;
  std::string f1 = fname;
  std::string::size_type n = f1.rfind('/');
  if (n != std::string::npos) {
    f = &f1.c_str()[n+1];
  }

  va_start(args, fmt);
  vsnprintf(TmpBuf, sizeof(TmpBuf)-1, fmt, args);
  //printf(TmpBuf);
  syslog(Lev, "%c[%sL%dT%d] %s", logChar[Lev], f, line, get_UpTime(), TmpBuf);

  va_end(args);
  return 1;
}

