/**
@file SocketCom.h
@brief SocketCom functions to help with socket usage
@author Ricardo A T <ratriches@yahoo.com.br>
@since xx/03/2023
@version 1.0.0
Notas e Alteracoes:
@note XX/XX/XXXX - XXX(X):    - XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

**/

#ifndef SOCKETCOM_H_
#define SOCKETCOM_H_

#include <stdint.h> // uint*_t
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

/*############# Definicoes Globais ##########################################*/
#define VERSION_LIBSOCK_STR			"LS2.0.0"	/**< Versao da lib */

typedef int (*onConnect_tf)(void);
typedef int (*onDisconnect_tf)(int code);
typedef int (*onReceive_tf)(const uint8_t* data, uint32_t len);

typedef struct  {
  onConnect_tf      onConnect = nullptr;
  onDisconnect_tf   onDisconnect = nullptr;
  onReceive_tf      onReceive = nullptr;
} SockCallBacks_ts;

typedef struct LocalSrvSec_s LServerSec_ts;
typedef struct LocalCliSec_s LClientSec_ts;
typedef struct InetSrvSec_s IServerSec_ts;
typedef struct InetCliSec_s IClientSec_ts;

typedef enum {
  LS_Cfg_WaitMore = 0x01,
} LS_CfgParam_te;

/*############# Prototipos Globais ##########################################*/

// ################ LS_LSrv
/*------------------------------------------------------------------------*//**
 @fn			LServerSec_ts* LS_LSrvCreateSock(std::string path, SockCallBacks_ts Cbs, uint32_t lenbuf)
*//*-------------------------------------------------------------------------*/
LServerSec_ts* LS_LSrvCreateSock(std::string path, SockCallBacks_ts Cbs, uint32_t lenbuf);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LSrvDeleteSock(LServerSec_ts* sec, bool DeinitLog)
*//*-------------------------------------------------------------------------*/
int LS_LSrvDeleteSock(LServerSec_ts* sec, bool DeinitLog = false);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LSrvSendSock(LServerSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
int LS_LSrvSendSock(LServerSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LSrvWaitRecSock(LServerSec_ts* sec, uint32_t tpoms)
*//*-------------------------------------------------------------------------*/
int LS_LSrvWaitRecSock(LServerSec_ts* sec, uint32_t tpoms);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LSrvGetStatusSock(LServerSec_ts* sec)
*//*-------------------------------------------------------------------------*/
int LS_LSrvGetStatusSock(LServerSec_ts* sec);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LSrvSetParam(LServerSec_ts* sec, LS_CfgParam_te par, int val)
*//*-------------------------------------------------------------------------*/
int LS_LSrvSetParam(LServerSec_ts* sec, LS_CfgParam_te par, int val);


// ################ LS_LCLi
/*------------------------------------------------------------------------*//**
 @fn			LClientSec_ts* LS_LCLiCreateSock(std::string path, SockCallBacks_ts Cbs, uint32_t lenbuf)
*//*-------------------------------------------------------------------------*/
LClientSec_ts* LS_LCLiCreateSock(std::string path, SockCallBacks_ts Cbs, uint32_t lenbuf);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LCLiDeleteSock(LClientSec_ts* sec, bool DeinitLog)
*//*-------------------------------------------------------------------------*/
int LS_LCLiDeleteSock(LClientSec_ts* sec, bool DeinitLog = false);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LCLiSendSock(LClientSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
int LS_LCLiSendSock(LClientSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LCLiWaitRecSock(LClientSec_ts* sec, uint32_t tpoms)
*//*-------------------------------------------------------------------------*/
int LS_LCLiWaitRecSock(LClientSec_ts* sec, uint32_t tpoms);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LCliGetStatusSock(LClientSec_ts* sec)
*//*-------------------------------------------------------------------------*/
int LS_LCliGetStatusSock(LClientSec_ts* sec);

/*------------------------------------------------------------------------*//**
 @fn			int LS_LCliSetParam(LClientSec_ts* sec, LS_CfgParam_te par, int val)
*//*-------------------------------------------------------------------------*/
int LS_LCliSetParam(LClientSec_ts* sec, LS_CfgParam_te par, int val);


// ################ LS_ISrv
/*------------------------------------------------------------------------*//**
 @fn			IServerSec_ts* LS_ISrvCreateSock(uint16_t Port, SockCallBacks_ts Cbs, uint32_t lenbuf)
*//*-------------------------------------------------------------------------*/
IServerSec_ts* LS_ISrvCreateSock(uint16_t Port, SockCallBacks_ts Cbs, uint32_t lenbuf);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ISrvDeleteSock(IServerSec_ts* sec, bool DeinitLog)
*//*-------------------------------------------------------------------------*/
int LS_ISrvDeleteSock(IServerSec_ts* sec, bool DeinitLog = false);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ISrvSendSock(IServerSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
int LS_ISrvSendSock(IServerSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ISrvWaitRecSock(IServerSec_ts* sec, uint32_t tpoms)
*//*-------------------------------------------------------------------------*/
int LS_ISrvWaitRecSock(IServerSec_ts* sec, uint32_t tpoms);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ISrvGetStatusSock(IServerSec_ts* sec)
*//*-------------------------------------------------------------------------*/
int LS_ISrvGetStatusSock(IServerSec_ts* sec);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ISrvSetParam(IServerSec_ts* sec, LS_CfgParam_te par, int val)
*//*-------------------------------------------------------------------------*/
int LS_ISrvSetParam(IServerSec_ts* sec, LS_CfgParam_te par, int val);


// ################ LS_ICLi
/*------------------------------------------------------------------------*//**
 @fn			IClientSec_ts* LS_ICLiCreateSock(std::string HostIp, uint16_t HostPort, SockCallBacks_ts Cbs, uint32_t lenbuf)
*//*-------------------------------------------------------------------------*/
IClientSec_ts* LS_ICLiCreateSock(std::string HostIp, uint16_t HostPort, SockCallBacks_ts Cbs, uint32_t lenbuf);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ICLiDeleteSock(IClientSec_ts* sec, bool DeinitLog)
*//*-------------------------------------------------------------------------*/
int LS_ICLiDeleteSock(IClientSec_ts* sec, bool DeinitLog = false);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ICLiSendSock(IClientSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
int LS_ICLiSendSock(IClientSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ICLiWaitRecSock(IClientSec_ts* sec, uint32_t tpoms)
*//*-------------------------------------------------------------------------*/
int LS_ICLiWaitRecSock(IClientSec_ts* sec, uint32_t tpoms);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ICliGetStatusSock(IClientSec_ts* sec)
*//*-------------------------------------------------------------------------*/
int LS_ICliGetStatusSock(IClientSec_ts* sec);

/*------------------------------------------------------------------------*//**
 @fn			int LS_ICliSetParam(IClientSec_ts* sec, LS_CfgParam_te par, int val)
*//*-------------------------------------------------------------------------*/
int LS_ICliSetParam(IClientSec_ts* sec, LS_CfgParam_te par, int val);


// ################
/*------------------------------------------------------------------------*//**
 @fn			int LS_SetLogLevels(uint8_t lv)
*//*-------------------------------------------------------------------------*/
int LS_SetLogLevels(uint8_t lv);

#ifdef __cplusplus
}
#endif

#endif // SOCKETCOM_H_
