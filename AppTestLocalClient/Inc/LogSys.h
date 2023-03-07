#if !defined(_LOGSYS_H_)
#define _LOGSYS_H_

#include <stdint.h>
#include <syslog.h>

#define L_EMERG       (/*0*/LOG_EMERG)
#define L_ALERT       (/*1*/LOG_ALERT)
#define L_CRIT        (/*2*/LOG_CRIT)
#define L_ERR         (/*3*/LOG_ERR)
#define L_WARNING     (/*4*/LOG_WARNING)
#define L_NOTICE      (/*5*/LOG_NOTICE)
#define L_INFO        (/*6*/LOG_INFO)
#define L_DEBUG       (/*7*/LOG_DEBUG)
#define LL_ALL        (0xFF)

#define Log_P(l, f, ...)        Log_PrintfL(l, __LINE__, f,  ##__VA_ARGS__)
#define Log_EMERG(f, ...)       Log_PrintfFL(L_EMERG, __FILE__, __LINE__, f,  ##__VA_ARGS__)
#define Log_ALERT(f, ...)       Log_PrintfFL(L_ALERT, __FILE__, __LINE__, f,  ##__VA_ARGS__)
#define Log_CRIT(f, ...)        Log_PrintfFL(L_CRIT, __FILE__, __LINE__, f,  ##__VA_ARGS__)
#define Log_ERR(f, ...)         Log_PrintfFL(L_ERR, __FILE__, __LINE__, f,  ##__VA_ARGS__)
#define Log_WARNING(f, ...)     Log_PrintfL(L_WARNING, __LINE__, f,  ##__VA_ARGS__)
#define Log_NOTICE(f, ...)      Log_PrintfL(L_NOTICE, __LINE__, f,  ##__VA_ARGS__)
#define Log_INFO(f, ...)        Log_PrintfL(L_INFO, __LINE__, f,  ##__VA_ARGS__)
#define Log_DEBUG(f, ...)       Log_PrintfFL(L_DEBUG, __FILE__, __LINE__, f,  ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

int Log_Init(const char* name);
int Log_Deinit(void);
int Log_SetLevel(uint8_t lev);
int Log_Printf(uint8_t Lev, const char* fmt, ...);
int Log_PrintfL(uint8_t Lev, uint32_t line, const char* fmt, ...);
int Log_PrintfFL(uint8_t Lev, const char* fname, uint32_t line, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // _LOGSYS_H_
