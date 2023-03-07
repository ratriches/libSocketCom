// #define USING_THREAD

#include <string.h>
#include <mutex>
#ifndef USING_THREAD
#include <unistd.h>
#include <chrono> 
#endif // USING_THREAD

#include <Ls_LogSys.h>

#include "Ls_Aux.h"

using namespace std;

/*############# Definicoes locais ###########################################*/

struct LmyMutexW_s {
#ifdef USING_THREAD
  mutex m_wait;
  mutex m_thw;
  pthread_t tWId = 0;
#else // USING_THREAD
  timed_mutex m_wait;
#endif // USING_THREAD
  uint32_t tout = 1;
};

/*############# Variaveis Locais ############################################*/

/*############# Prototipos Locais ###########################################*/

/*############# Implementacao Local #########################################*/

#ifdef USING_THREAD
/*------------------------------------------------------------------------*//**
 @fn			static void Aux_Millisleep(int ms)
*//*-------------------------------------------------------------------------*/
static void Aux_Millisleep(int ms) {
  if (ms > 0) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = ms * 1000;
    select(0, NULL, NULL, NULL, &tv);
  }
}

/*------------------------------------------------------------------------*//**
 @fn			static void* LmyWait_ThFcn(void* arg)
*//*-------------------------------------------------------------------------*/
static void* LmyWait_ThFcn(void* arg) {
  LmyMutexW_ts* m_ = (LmyMutexW_ts*)arg;

  pthread_detach(pthread_self());
  int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  if (s != 0) { LibLog_CRIT("L pthread_setcancelstate: FAIL %d, %X", s, m_); }
  s = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  if (s != 0) { LibLog_CRIT("L pthread_setcanceltype: FAIL %d, %X", s, m_); }

  m_->m_thw.lock();
  Aux_Millisleep(m_->tout);
  m_->m_wait.unlock();
  m_->tWId = 0;

  return nullptr;
  // pthread_exit(nullptr);
}
#endif // USING_THREAD

/*############# Implementacao Exportada #####################################*/

/*------------------------------------------------------------------------*//**
 @fn			int LmyMutexTryLock(LmyMutexW_ts* m_)
*//*-------------------------------------------------------------------------*/
int LmyMutexTryLock(LmyMutexW_ts* m_) {
  return m_->m_wait.try_lock();
}

/*------------------------------------------------------------------------*//**
 @fn			int LmyMutexWaitms(uint32_t tms, bool CrlI, LmyMutexW_ts* m_)
*//*-------------------------------------------------------------------------*/
int LmyMutexWaitms(uint32_t tms, bool CrlI, LmyMutexW_ts* m_) {
  if (tms <= 0) return -1; // "timeout"
  if (m_ == nullptr) return -10;

  if (CrlI) m_->m_wait.try_lock();

  m_->tout = tms;
#ifdef USING_THREAD
  m_->m_thw.try_lock();
  int ret;
  if ((ret = pthread_create(&m_->tWId, nullptr, &LmyWait_ThFcn, m_)) == 0) {
    pthread_yield();
  } else {
    LibLog_CRIT("L myMutexWaitms: FAIL %X, r:%d, th:%d, to:%d", 
             m_, ret, m_->tWId, m_->tout);
    Aux_Millisleep(m_->tout); // fake timeout
    return -20; // ferrou
  }
  m_->m_thw.unlock();
  m_->m_wait.lock();
  if (m_->tWId) {
    pthread_cancel(m_->tWId);
    return 0; // unlocked before timeout
  }
#else // USING_THREAD
  if (m_->m_wait.try_lock_for(std::chrono::milliseconds(m_->tout))) return 0;
#endif // USING_THREAD

  return -1; // timeout

}

/*------------------------------------------------------------------------*//**
 @fn			void LmyMutexUnlock(LmyMutexW_ts* m_)
*//*-------------------------------------------------------------------------*/
void LmyMutexUnlock(LmyMutexW_ts* m_) {
  if (m_ == nullptr) return;

  m_->m_wait.unlock();
}

/*------------------------------------------------------------------------*//**
 @fn			LmyMutexW_ts* LmyMutexNew(void)
*//*-------------------------------------------------------------------------*/
LmyMutexW_ts* LmyMutexNew(void) {
  return new LmyMutexW_ts;
}

/*------------------------------------------------------------------------*//**
 @fn			void LmyMutexDel(LmyMutexW_ts** m_)
*//*-------------------------------------------------------------------------*/
void LmyMutexDel(LmyMutexW_ts** m_) {
  if (m_) {
    if (*m_) {
#ifdef USING_THREAD
      if ((*m_)->tWId) {
        pthread_cancel((*m_)->tWId);
        (*m_)->tWId = 0;
      }
#endif // USING_THREAD
      (*m_)->m_wait.unlock();
      delete (*m_);
    }
    (*m_) = nullptr;
  }
}
