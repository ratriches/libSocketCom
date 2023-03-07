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

#define LibLog_P(l, f, ...)        LibLog_PrintfL(l, __LINE__, f,  ##__VA_ARGS__)
#define LibLog_EMERG(f, ...)       LibLog_PrintfL(L_EMERG, __LINE__, f,  ##__VA_ARGS__)
#define LibLog_ALERT(f, ...)       LibLog_PrintfL(L_ALERT, __LINE__, f,  ##__VA_ARGS__)
#define LibLog_CRIT(f, ...)        LibLog_PrintfL(L_CRIT, __LINE__, f,  ##__VA_ARGS__)
#define LibLog_ERR(f, ...)         LibLog_PrintfL(L_ERR, __LINE__, f,  ##__VA_ARGS__)
#define LibLog_WARNING(f, ...)     LibLog_PrintfL(L_WARNING, __LINE__, f,  ##__VA_ARGS__)
#define LibLog_NOTICE(f, ...)      LibLog_PrintfL(L_NOTICE, __LINE__, f,  ##__VA_ARGS__)
#define LibLog_INFO(f, ...)        LibLog_PrintfL(L_INFO, __LINE__, f,  ##__VA_ARGS__)
#define LibLog_DEBUG(f, ...)       LibLog_PrintfL(L_DEBUG, __LINE__, f,  ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

int LibLog_Init(const char* name);
int LibLog_Deinit(void);
int LibLog_SetLevel(uint8_t lev);
int LibLog_Printf(uint8_t Lev, const char* fmt, ...);
int LibLog_PrintfL(uint8_t Lev, uint32_t line, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // _LOGSYS_H_
