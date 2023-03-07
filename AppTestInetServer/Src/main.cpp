

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <atomic>

#include <time.h>

#include <mutex>
#include <SocketCom.h>

#include "LogSys.h"

/*############# Definicoes locais ###########################################*/

#define TAM_WORKBUFF		  (1024)	
#define TOUT_WAIT_COM_ms	(500) // ms
#define TOUT_WAIT_CONT_ms	(5) // ms

/*############# Variaveis Acessiveis externamente ###########################*/

/*############# Variaveis Locais ############################################*/

static std::atomic_bool ThreadsLoop(true);
static std::mutex m_Rec;

/*############# Prototipos Locais ###########################################*/

/*############# Implementacao Local #########################################*/
/**
 * @return milliseconds
 */
uint32_t get_UpTime() {
  struct timespec spec;
  if (clock_gettime(1, &spec) == -1) { /* 1 is CLOCK_MONOTONIC */
    return 0;
  }

  return spec.tv_sec * 1000 + spec.tv_nsec / 1e6;
}

/*------------------------------------------------------------------------*//**
 @fn			void Aux_Millisleep(int ms)
*//*-------------------------------------------------------------------------*/
void Aux_Millisleep(int ms) {
  if (ms > 0) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = ms * 1000;
    select(0, NULL, NULL, NULL, &tv);
  }
}

/*------------------------------------------------------------------------*//**
 @fn			static void handler_SIGINT(int sig)
*//*-------------------------------------------------------------------------*/
static void handler_SIGINT(int sig) {
  Log_WARNING("CTRL+C pressionado");
  printf("CTRL+C pressionado\n");
  ThreadsLoop = false;
  m_Rec.unlock();
  Aux_Millisleep(100);
}

/*------------------------------------------------------------------------*//**
 @fn			int onConnect(void)
*//*-------------------------------------------------------------------------*/
int onConnect(void) {
  printf("S onConnect\n");
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int onDisconnect(int code)
*//*-------------------------------------------------------------------------*/
int onDisconnect(int code) {
  printf("S onDisconnect: %d\n", code);
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int onReceive(const uint8_t* data, uint32_t len)
*//*-------------------------------------------------------------------------*/
int onReceive(const uint8_t* data, uint32_t len) {
  printf("S onReceive: %d, '%s'\n", len, data);
  m_Rec.unlock();
  return 1; // libera LS_LSrvWaitRecSock
}

/*############# Implementacao Exportada #####################################*/
/*------------------------------------------------------------------------*//**
 @fn			int main(int argc, char** argv)
*//*-------------------------------------------------------------------------*/
int main(int argc, char** argv) {
  int cnt, ret;
  uint8_t WorkBuf[TAM_WORKBUFF];
  uint16_t Port = 1234;


  SockCallBacks_ts Cbs;
  Cbs.onConnect = onConnect;
  Cbs.onDisconnect = onDisconnect;
  Cbs.onReceive = onReceive;


  if (argc >= 2) {
    Port = atoi(argv[1]);
  }

  fprintf(stdout, "AppTestInetServer, At Port %d\n", Port);
  Log_Init("AppTestInetServer");
  Log_SetLevel(LL_ALL);
  LS_SetLogLevels(LL_ALL);

  if (signal(SIGINT, handler_SIGINT) == SIG_ERR) {
    Log_ERR("Nao foi possivel capturar SIGINT");
  }

  ThreadsLoop = true;
  IServerSec_ts* SrvI = nullptr;
  while (ThreadsLoop) {
    printf("LS_ISrvCreateSock 1\n");
    SrvI = LS_ISrvCreateSock(Port, Cbs, sizeof(WorkBuf));
    printf("LS_ISrvCreateSock 2\n");

    if (LS_ISrvGetStatusSock(SrvI) < 0) {
      printf("LS_ISrvGetStatusSock %d\n", LS_ISrvGetStatusSock(SrvI));
      Log_ERR("LS_ISrvGetStatusSock %d", LS_ISrvGetStatusSock(SrvI));
      LS_ISrvDeleteSock(SrvI);
      Aux_Millisleep(10000);  
      // NOTE: if 'server' crashes or finishes, it may take 'a long time' to create the socket again
      // on ubuntu tests, it took about 60s
      continue;
    }
    break;
  }
  if (!ThreadsLoop) return 0;

  printf("LS_ISrvCreateSock\n");
  Aux_Millisleep(500);

  printf("proc\n");
  m_Rec.try_lock();

  cnt = 0;
  while (ThreadsLoop) {
    printf("wait rec\n");
    m_Rec.lock();
    // printf("rec\n");
    ret = sprintf((char*)WorkBuf, "S teste: %d", cnt++);
    Aux_Millisleep(300);
    ret = LS_ISrvSendSock(SrvI, WorkBuf, ret, 500);
    printf("sent %d\n", ret);
  }


  LS_SetLogLevels(LL_ALL); // apenas para registar "tudo" da desconexao
  LS_ISrvDeleteSock(SrvI);
  printf("LS_ISrvDeleteSock\n");
  Aux_Millisleep(500);
  printf("AppTestInetServer End\n");

  return 0;
}
