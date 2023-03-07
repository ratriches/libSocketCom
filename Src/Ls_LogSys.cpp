
#include <time.h>
#include <stdio.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <string.h>

#include "Ls_LogSys.h"


/**
 * NOTE: Logsys alterado para trabalhar com lib dinamica.
 * necessario pois esta usa o mesmo espaco de memoria da app,
 * ou seja, para o syslog eh o mesmo processo
 */

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
static char Name[32] = {0};
  
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
int LibLog_Init(const char* name) {
  // NOTE: a app eh que inicia o log
  // if (init_) {
  //   openlog(name, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    init_ = false;
  // } else {
  //   printf("Log_Init Ops: %s!\n", name);
  // }
  strncpy(Name, name, sizeof(Name) - 1);
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int LibLog_Deinit(void)
*//*-------------------------------------------------------------------------*/
int LibLog_Deinit(void) {
  // closelog();
  //printf("LibLog_Deinit\n");
  init_ = true;
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int LibLog_SetLevel(uint8_t lev)
*//*-------------------------------------------------------------------------*/
int LibLog_SetLevel(uint8_t lev) { // map bit
  int cfgLv_ = cfgLv;
  cfgLv = lev;
  return cfgLv_;
}

/*------------------------------------------------------------------------*//**
 @fn			int LibLog_Printf(uint8_t Lev, const char* fmt, ...)
*//*-------------------------------------------------------------------------*/
int LibLog_Printf(uint8_t Lev, const char* fmt, ...) {
  va_list args;
  
  if (init_) return 0;
  if ((cfgLv & (1<<Lev) ) == 0) {
    return 0;
  }
  
  va_start(args, fmt);
  vsnprintf(TmpBuf, sizeof(TmpBuf)-1, fmt, args);
  //printf(TmpBuf);
  syslog(Lev, "%c[T%d] (%s) %s", logChar[Lev], get_UpTime(), Name, TmpBuf);

  va_end(args);
  return 1;
}

/*------------------------------------------------------------------------*//**
 @fn			int LibLog_Printf(uint8_t Lev, uint32_t line, const char* fmt, ...)
*//*-------------------------------------------------------------------------*/
int LibLog_PrintfL(uint8_t Lev, uint32_t line, const char* fmt, ...) {
  va_list args;
  
  if (init_) return 0;
  if ((cfgLv & (1<<Lev) ) == 0) {
    return 0;
  }
  
  va_start(args, fmt);
  vsnprintf(TmpBuf, sizeof(TmpBuf)-1, fmt, args);
  //printf(TmpBuf);
  syslog(Lev, "%c[L%dT%d] (%s) %s", logChar[Lev], line, get_UpTime(), Name, TmpBuf);

  va_end(args);
  return 1;
}

