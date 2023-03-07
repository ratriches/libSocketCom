#if !defined(_AUX_H_)
#define _AUX_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct LmyMutexW_s LmyMutexW_ts;

#ifdef __cplusplus
extern "C" {
#endif

  int LmyMutexTryLock(LmyMutexW_ts* m_) ;
  int LmyMutexWaitms(uint32_t tms, bool CrlI, LmyMutexW_ts* m_);
  void LmyMutexUnlock(LmyMutexW_ts* m_) ;
  LmyMutexW_ts* LmyMutexNew(void);
  void LmyMutexDel(LmyMutexW_ts** m_);

#ifdef __cplusplus
}
#endif

#endif // _AUX_H_
