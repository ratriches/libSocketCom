

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <atomic>

#include <time.h>

// #include <thread>
#include <mutex>
#include <SocketCom.h>

#include "LogSys.h"

/*############# Definicoes locais ###########################################*/

#define TAM_WORKBUFF		  (1024)	
#define TOUT_WAIT_COM_ms	(500) // ms

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
  printf("C onConnect\n");
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int onDisconnect(int code)
*//*-------------------------------------------------------------------------*/
int onDisconnect(int code) {
  printf("C onDisconnect: %d\n", code);
  return 0;
}

/*------------------------------------------------------------------------*//**
 @fn			int onReceive(const uint8_t* data, uint32_t len)
*//*-------------------------------------------------------------------------*/
int onReceive(const uint8_t* data, uint32_t len) {
  printf("C onReceive: %d, '%s'\n", len, data);
  m_Rec.unlock();
  return 1; // libera LS_LCLiWaitRecSock
}

/*############# Implementacao Exportada #####################################*/
/*------------------------------------------------------------------------*//**
 @fn			int main(int argc, char** argv)
*//*-------------------------------------------------------------------------*/
int main(int argc, char** argv) {
  int cnt, ret = 0;
  uint8_t WorkBuf[TAM_WORKBUFF];
  char path[32] = "/tmp/mysock";


  SockCallBacks_ts Cbs;
  Cbs.onConnect = onConnect;
  Cbs.onDisconnect = onDisconnect;
  Cbs.onReceive = onReceive;

  if (argc >= 2) {
    strncpy(path, argv[1], sizeof(path) - 1);
  }

  fprintf(stdout, "AppTestLocalClient, At path %s\n", path);
  Log_Init("AppTestLocalClient");
  Log_SetLevel(LL_ALL);
  LS_SetLogLevels(LL_ALL);

  if (signal(SIGINT, handler_SIGINT) == SIG_ERR) {
    Log_ERR("Nao foi possivel capturar SIGINT");
  }


  LClientSec_ts* LCli = LS_LCLiCreateSock(path, Cbs, sizeof(WorkBuf));
  printf("LS_LCLiCreateSock\n");

  cnt = 0;
  ThreadsLoop = true;
  m_Rec.try_lock();
  while (ThreadsLoop) {
    Aux_Millisleep(2000);
    ret = sprintf((char*)WorkBuf, "C teste: %d", cnt++);
    ret = LS_LCLiSendSock(LCli, WorkBuf, ret, 500);
    printf("sent %d\n", ret);
  }

  printf("LS_LCLiDeleteSock\n");
  LS_LCLiDeleteSock(LCli);
  Aux_Millisleep(2000);
  printf("AppTestLocalClient End\n");
  Aux_Millisleep(2000);

  return 0;
}
