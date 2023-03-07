/**
@file SocketCom.c
@brief SocketCom functions to help with socket usage
@author Ricardo A T <ratriches@yahoo.com.br>
@since xx/03/2023
@version 1.0.0
Notas e Alteracoes:
@note XX/XX/XXXX - XXX(X):    - XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

**/

// #include <iostream> // debug
// #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>         // std::chrono::seconds
#include <thread>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "DHComp.h"
#include <Ls_Aux.h>
#include "Ls_LogSys.h"
#include "SocketCom.h"

/*############# Definicoes locais ###########################################*/

#define TOUT_WAIT_COM_ms    (10000) // ms 
#define TOUT_WAIT_CONT_ms   (5) // ms

/** struct LocalSrvSec_s */
struct LocalSrvSec_s {
	int status = -1;
	pthread_t _thid = 0;
	LmyMutexW_ts* m_Rec = nullptr;
	bool loop = false;
	SockCallBacks_ts _Cbs;
	uint8_t* buf = nullptr;
	uint32_t lenbuf = 0;
	std::string _path;
	int sockfd = -1;
	int confd = -1;
	uint32_t waitmore = TOUT_WAIT_CONT_ms;
};

/** struct LocalCliSec_s */
struct LocalCliSec_s {
	int status = -1;
	pthread_t _thid = 0;
	LmyMutexW_ts* m_Rec = nullptr;
	bool loop = false;
	SockCallBacks_ts _Cbs;
	uint8_t* buf = nullptr;
	uint32_t lenbuf = 0;
	std::string _path;
	int sockfd = -1;
	uint32_t waitmore = TOUT_WAIT_CONT_ms;
};

/** struct InetlSrvSec_s */
struct InetSrvSec_s {
	int status = -1;
	pthread_t _thid = 0;
	LmyMutexW_ts* m_Rec = nullptr;
	bool loop = false;
	SockCallBacks_ts _Cbs;
	uint8_t* buf = nullptr;
	uint32_t lenbuf = 0;
	uint16_t _Port;
	int sockfd = -1;
	int confd = -1;
	uint32_t waitmore = TOUT_WAIT_CONT_ms;
};

/** struct InetCliSec_s */
struct InetCliSec_s {
	int status = -1;
	pthread_t _thid = 0;
	LmyMutexW_ts* m_Rec = nullptr;
	bool loop = false;
	SockCallBacks_ts _Cbs;
	uint8_t* buf = nullptr;
	uint32_t lenbuf = 0;
	std::string _HostIp;
	uint16_t _Port;
	int sockfd = -1;
	uint32_t waitmore = TOUT_WAIT_CONT_ms;
};


/*############# Variaveis Locais ############################################*/

/*############# Prototipos Locais ###########################################*/

/*############# Implementacao Local #########################################*/

/*------------------------------------------------------------------------*//**
@fn			static int CreateLocalSock(void)
*//*-------------------------------------------------------------------------*/
static int CreateLocalSock(void) {
	int sock;

	/* fd for the socket */
	sock = socket(AF_LOCAL,  		/* AF_LOCAL */
								SOCK_STREAM,  /*  */
								0);           /*  */
	if (sock < 0) {
		LibLog_ERR("socket Local fail"); /* terminate */
		return -1;
	}

	return sock;
}

/*------------------------------------------------------------------------*//**
@fn			static int CreateInetSock(void)
*//*-------------------------------------------------------------------------*/
static int CreateInetSock(void) {
	int sock;

	/* fd for the socket */
	sock = socket(AF_INET,  		/* AF_INET */
								SOCK_STREAM,  /*  */
								0);           /*  */
	if (sock < 0) {
		LibLog_ERR("socket Inet fail"); /* terminate */
		return -1;
	}

	return sock;
}

/*------------------------------------------------------------------------*//**
@fn			static int makeServerSocketLocal(const char* path)
*//*-------------------------------------------------------------------------*/
static int makeServerSocketLocal(const char* path) {
	int sockfd, ret;
	struct sockaddr_un saddr;

	LibLog_DEBUG("make_socket Local on path:%s", path);

	sockfd = CreateLocalSock();
	if (sockfd < 0) {
		return -1;
	}

	unlink(path);

	/* start with a clean address structure */
	memset(&saddr, 0, sizeof(saddr));
	saddr.sun_family = AF_LOCAL;
	strncpy(saddr.sun_path, path, sizeof(saddr.sun_path) - 1);
	if ((ret = bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr))) < 0) {
		LibLog_ERR("bind Local fail %d", ret);
		return -2;
	}

	return sockfd;
}


/*------------------------------------------------------------------------*//**
@fn			static int makeServerSocketInet(uint16_t port)
*//*-------------------------------------------------------------------------*/
static int makeServerSocketInet(uint16_t port) {
	int sockfd, ret;
	struct sockaddr_in saddr;

	LibLog_DEBUG("make_socket Inet on port:%i", port);

	sockfd = CreateInetSock();
	if (sockfd < 0) {
		return -1;
	}

	/* Give the socket a name. */
	memset(&saddr, 0, sizeof(saddr));          /* clear the bytes */
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if ((ret = bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr))) < 0) {
		LibLog_ERR("bind Inet fail %d", ret);
		return -2;
	}

	return sockfd;
}

/*------------------------------------------------------------------------*//**
@fn			static int ServerInitLocal(const char* path)
*//*-------------------------------------------------------------------------*/
static int ServerInitLocal(const char* path) {
	int sockfd;

	LibLog_Deinit();
	LibLog_Init("libSockCom");
	LibLog_NOTICE("ServerInitLocal " VERSION_LIBSOCK_STR ":" DH_COMP " path:%s", path);

	sockfd = makeServerSocketLocal(path);
	if (sockfd <= 0) {
		LibLog_ERR("Erro make_socket path:%s [%d]", path, sockfd);
		return -1;
	}

	/* listen to the socket */
	if (listen(sockfd, 1) < 0) { /* listen for clients, up to MaxConnects */
		LibLog_ERR("Erro listen Local socketfd:%d", sockfd);
		return -10;
	}

	LibLog_INFO("Listening sockfd:%d on path:%s", sockfd, path);

	return sockfd;
}

/*------------------------------------------------------------------------*//**
@fn			static int ServerInitInet(uint16_t sPort_)
*//*-------------------------------------------------------------------------*/
static int ServerInitInet(uint16_t sPort_) {
	int sockfd;

	LibLog_Deinit();
	LibLog_Init("libSockCom");
	LibLog_NOTICE("ServerInitInet " VERSION_LIBSOCK_STR ":" DH_COMP " port:%i", sPort_);

	sockfd = makeServerSocketInet(sPort_);
	if (sockfd <= 0) {
		LibLog_ERR("Erro make_socket port:%d [%d]", sPort_, sockfd);
		return -1;
	}

	/* listen to the socket */
	if (listen(sockfd, 1) < 0) { /* listen for clients, up to MaxConnects */
		LibLog_ERR("Erro listen Inet socketfd:%d", sockfd);
		return -10;
	}

	LibLog_INFO("Listening sockfd:%d on port:%i for client", sockfd, sPort_);

	return sockfd;
}

/*------------------------------------------------------------------------*//**
@fn			static int ServerLocalWaitClient(int serverSockfd, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
static int ServerLocalWaitClient(int serverSockfd, uint32_t Toutms) {
	int clientfd;
	struct sockaddr_un caddr;		/* client address */
	socklen_t len = sizeof(caddr);  /* address length could change */
	fd_set rfds;
	struct timeval tv;
	int retS;

	if (serverSockfd <= 0) return -1000;

	LibLog_DEBUG("wait connect to Local serverSockfd:%d, Toutms:%ds", serverSockfd, Toutms);

	FD_ZERO(&rfds);
	FD_SET(serverSockfd, &rfds);

	tv.tv_sec = Toutms / 1000;
	tv.tv_usec = (Toutms % 1000) * 1000;

	retS = select(serverSockfd + 1, &rfds, NULL, NULL, &tv);
	if (retS <= 0) {
		// printf("sock select %d ret:%d\n", serverSockfd, retS);
		return retS;
	}

	clientfd = accept(serverSockfd, (struct sockaddr*)&caddr, &len);  /* accept blocks */
	if (clientfd <= 0) {
		LibLog_ERR("FAIL Local accept"); /* don't terminated, though there's a problem */
		return -1001;
	}

	// LibLog_INFO("Local serverSockfd:%d connect to clientfd:%d", serverSockfd, clientfd);
	return clientfd;
}

/*------------------------------------------------------------------------*//**
@fn			static int ServerInetWaitClient(int serverSockfd, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
static int ServerInetWaitClient(int serverSockfd, uint32_t Toutms) {
	int clientfd;
	struct sockaddr_in caddr;		/* client address */
	socklen_t len = sizeof(caddr);  /* address length could change */
	fd_set rfds;
	struct timeval tv;
	int retS;

	if (serverSockfd <= 0) return -1000;

	LibLog_DEBUG("wait connect to Inet serverSockfd:%d, Toutms:%ds", serverSockfd, Toutms);

	FD_ZERO(&rfds);
	FD_SET(serverSockfd, &rfds);

	tv.tv_sec = Toutms / 1000;
	tv.tv_usec = (Toutms % 1000) * 1000;

	retS = select(serverSockfd + 1, &rfds, NULL, NULL, &tv);
	if (retS <= 0) {
		//printf("sock select ret:%d\n", retS);
		return retS;
	}

	clientfd = accept(serverSockfd, (struct sockaddr*)&caddr, &len);  /* accept blocks */
	if (clientfd <= 0) {
		LibLog_ERR("FAIL Inet accept"); /* don't terminated, though there's a problem */
		return -1001;
	}
	
	// LibLog_INFO("Inet serverSockfd:%d connect to clientfd:%d", serverSockfd, clientfd);
	return clientfd;
}

/*------------------------------------------------------------------------*//**
@fn			static int ServerEndClient(int* clientfd)
*//*-------------------------------------------------------------------------*/
static int ServerEndClient(int* clientfd) {
	if ((*clientfd) > 0) {
		// LibLog_WARNING("Server End Connection clientfd:%d", (*clientfd));
		// shutdown((*clientfd), SHUT_RDWR);
		close((*clientfd)); /* break connection */
		(*clientfd) = -1;
	}

	return 0;
}

/*------------------------------------------------------------------------*//**
@fn			static int ServerEnd(int* serverfd, int* clientfd)
*//*-------------------------------------------------------------------------*/
static int ServerEnd(int* serverfd, int* clientfd) {
	ServerEndClient(clientfd);

	if ((*serverfd) > 0) {
		// shutdown((*clientfd), SHUT_RDWR);
		close((*serverfd));
		(*serverfd) = -1;
		return  0;
	}

	return -1;
}


/*------------------------------------------------------------------------*//**
@fn			static int ClientEndCom(int *sockfd)
*//*-------------------------------------------------------------------------*/
static int ClientEndCom(int* sockfd) {
	if ((*sockfd) > 0) {
		// LibLog_INFO("Client End sockfd:%d", (*sockfd));
		// shutdown((*sockfd), SHUT_RDWR);
		close((*sockfd)); /* close the connection */
		(*sockfd) = -1;
	}
	
	return 0;
}


/*------------------------------------------------------------------------*//**
 @fn			static int SocketSendData(int sockfd, const uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
static int SocketSendData(int sockfd, const uint8_t* data, uint32_t len, uint32_t Toutms) {
	fd_set fdW;
	struct timeval tv;
	int retS, retSnd;

	if (sockfd <= 0) return -10;
	if (len == 0) return -11;
	if (Toutms == 0) return -12;

	FD_ZERO(&fdW);
	FD_SET(sockfd, &fdW);

	tv.tv_sec = Toutms / 1000;
	tv.tv_usec = (Toutms % 1000) * 1000;

	retS = select(sockfd + 1, NULL, &fdW, NULL, &tv);
	LibLog_DEBUG("select fd:%d; ret:%d; Toutms:%d", sockfd, retS, Toutms);
	if (retS == -1) {
		LibLog_ERR("send select() fail");
		return -20;
	} else if (retS) {
		// LibLog_DEBUG("Data is available now. ret:%d", retS);
		/* FD_ISSET(0, &rfds) will be true. */
	} else {
		// printf("No data\n");
		return 0; // timeout
	}

	retSnd = send(sockfd, data, len, MSG_NOSIGNAL);
	LibLog_DEBUG("send fd:%d; ret:%d; len:%d", sockfd, retSnd, len);
	if ((retSnd <= 0) && (retS > 0)) {
		retSnd = -1;
	}
	return retSnd;
}

/*------------------------------------------------------------------------*//**
@fn			static int SocketRecData(int sockfd, uint8_t* data, uint32_t maxlen, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
static int SocketRecData(int sockfd, uint8_t* data, uint32_t maxlen, uint32_t Toutms) {
	fd_set fdr;
	struct timeval tv;
	int retS, retRec;

	if (sockfd <= 0) return -10;
	if (maxlen == 0) return -11;
	if (Toutms == 0) return -12;

	FD_ZERO(&fdr);
	FD_SET(sockfd, &fdr);

	tv.tv_sec = Toutms / 1000;
	tv.tv_usec = (Toutms % 1000) * 1000;

	retS = select(sockfd + 1, &fdr, NULL, NULL, &tv);
	LibLog_DEBUG("select fd:%d; rec:%d; Toutms:%d", sockfd, retS, Toutms);
	if (retS == -1) {
		LibLog_ERR("rec select() fail");
		return -20;
	} else if (retS) {
		// LibLog_DEBUG("Data is available now. ret:%d", retS);
		/* FD_ISSET(0, &rfds) will be true. */
	} else {
		// printf("No data\n");
		return 0; // timeout
	}

	retRec = recv(sockfd, data, maxlen, 0);
	LibLog_DEBUG("recv fd:%d; retS:%d, retRec:%d, maxlen:%d", sockfd, retS, retRec, maxlen);
	if ((retRec <= 0) && (retS > 0)) {
		retRec = -1;
	}

	return retRec;
}


/*------------------------------------------------------------------------*//**
@fn			static void* LServer_ThFcn(void* arg)
*//*-------------------------------------------------------------------------*/
static void* LServer_ThFcn(void* arg) {
	LServerSec_ts* sec = (LServerSec_ts*)arg;
	const char* path = sec->_path.c_str();

	LibLog_DEBUG("LServer_ThFcn IN");
	pthread_detach(pthread_self());

	sec->loop = true;
	while ((sec->loop) && (sec->sockfd > 0)) {
		sec->status = 1;
		if (sec->confd <= 0) {
			while ((sec->confd = ServerLocalWaitClient(sec->sockfd, TOUT_WAIT_COM_ms)) <= 0) { // TODO: aumentar tempo
				std::this_thread::sleep_for(std::chrono::milliseconds(100)); // protecao
				if ((!sec->loop) ||
						(sec->confd <= -1000)) { // falha do select ou accept
					sec->status = -1000;
					goto END_TH;
				}
			}
			if (sec->_Cbs.onConnect) sec->_Cbs.onConnect();

			LibLog_INFO("ServerLocal '%s' connected; sockfd:%d,fd:%d", path, sec->sockfd, sec->confd);
		}
		sec->status = 2;

		uint32_t toutRec = TOUT_WAIT_COM_ms;
		uint32_t RecIdx = 0;
		int cnt = 0;
		memset(sec->buf, 0, sec->lenbuf);

		while (((sec->lenbuf - 1) > RecIdx) &&
					 ((cnt = SocketRecData(sec->confd, &sec->buf[RecIdx], sec->lenbuf - 1 - RecIdx, toutRec)) > 0)) {
			RecIdx += static_cast<unsigned int>(cnt);
			// Log_DEBUG("RecIdx [%d]; cnt [%d]", RecIdx, cnt);
			toutRec = sec->waitmore;//TOUT_WAIT_CONT_ms; // apos receber primeira parte reduz timeout
		};

		if (RecIdx > 0) {
			LmyMutexTryLock(sec->m_Rec); // limpa possivel liberacao anterior
			if (sec->_Cbs.onReceive(sec->buf, RecIdx) > 0) {
				LmyMutexUnlock(sec->m_Rec);
			}
			// Log_DEBUG("<%d;H%s;", RecIdx, SprintHex(&WorkBufTh[0], RecIdx));
		} else {
			if (cnt < 0) { // client error
				// sec->status = -1005;
				LibLog_WARNING("ServerEndClient: '%s' sockfd:%d,fd:%d", path, sec->sockfd, sec->confd);
				ServerEndClient(&sec->confd);
				if (sec->_Cbs.onDisconnect) sec->_Cbs.onDisconnect(-200);
			}
		}
	}

	sec->status = -1010;

END_TH:
	LibLog_DEBUG("LServer_ThFcn END %d", sec->status);
	sec->_thid = 0;
	return nullptr;
}

/*------------------------------------------------------------------------*//**
@fn			static void* LClient_ThFcn(void* arg)
*//*-------------------------------------------------------------------------*/
static void* LClient_ThFcn(void* arg) {
	LClientSec_ts* sec = (LClientSec_ts*)arg;
	const char* path = sec->_path.c_str();

	LibLog_DEBUG("LClient_ThFcn IN");
	pthread_detach(pthread_self());

	sec->loop = true;
	while (sec->loop) {
		sec->status = 1;
		if (sec->sockfd < 0) {
			int sock = CreateLocalSock();
			if (sock < 0) {
				sec->status = -11;
				break;
			}

			struct sockaddr_un saddr;
			memset(&saddr, 0, sizeof(saddr));
			saddr.sun_family = AF_LOCAL;
			strncpy(saddr.sun_path, path, sizeof(saddr.sun_path) - 1);

			while ((sec->loop) &&
						 (connect(sock, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			sec->sockfd = sock;

			if (sec->_Cbs.onConnect) sec->_Cbs.onConnect();
			LibLog_INFO("LS_CLiLocal '%s' connected; fd:%d", path, sec->sockfd);
		}
		sec->status = 2;

		uint32_t toutRec = TOUT_WAIT_COM_ms;
		uint32_t RecIdx = 0;
		int cnt = 0;
		memset(sec->buf, 0, sec->lenbuf);

		while (((sec->lenbuf - 1) > RecIdx) &&
					 ((cnt = SocketRecData(sec->sockfd, &sec->buf[RecIdx], sec->lenbuf - 1 - RecIdx, toutRec)) > 0)) {
			RecIdx += static_cast<unsigned int>(cnt);
			// LibLog_DEBUG("RecIdx [%d]; cnt [%d]", RecIdx, cnt);
			toutRec = sec->waitmore;//TOUT_WAIT_CONT_ms; // apos receber primeira parte reduz timeout
		};

		if (RecIdx > 0) {
			if (sec->_Cbs.onReceive) {
				LmyMutexTryLock(sec->m_Rec); // limpa possivel liberacao anterior
				if (sec->_Cbs.onReceive(sec->buf, RecIdx) > 0) {
					LmyMutexUnlock(sec->m_Rec);
				}
			}
			// LibLog_DEBUG("<%d;H%s;", RecIdx, SprintHex(&WorkBufTh[0], RecIdx));
		} else {
			if (cnt < 0) { // client error
				if (sec->_Cbs.onDisconnect) sec->_Cbs.onDisconnect(-200);
				if (sec->sockfd) {
					LibLog_WARNING("CliSocket close: '%s' fd:%d", path, sec->sockfd);
					close(sec->sockfd); /* close the connection */
					sec->sockfd = -1;
				}
			}
		}
	}
	LibLog_DEBUG("LClient_ThFcn END");

	sec->_thid = 0;
	return nullptr;
}

/*------------------------------------------------------------------------*//**
@fn			static void* IServer_ThFcn(void* arg)
*//*-------------------------------------------------------------------------*/
static void* IServer_ThFcn(void* arg) {
	IServerSec_ts* sec = (IServerSec_ts*)arg;

	LibLog_DEBUG("IServer_ThFcn IN");
	pthread_detach(pthread_self());

	sec->loop = true;
	while ((sec->loop) && (sec->sockfd > 0)) {
		sec->status = 1;
		if (sec->confd <= 0) {
			while ((sec->confd = ServerInetWaitClient(sec->sockfd, TOUT_WAIT_COM_ms)) <= 0) { // TODO: aumentar tempo
				std::this_thread::sleep_for(std::chrono::milliseconds(100)); // protecao
				if ((!sec->loop) ||
						(sec->confd <= -1000)) { // falha do select ou accept
					sec->status = -1000;
					goto END_TH;
				}
			}
			if (sec->_Cbs.onConnect) sec->_Cbs.onConnect();

			LibLog_INFO("ServerInet Port:%d connected; sockfd:%d,fd:%d", sec->_Port, sec->sockfd, sec->confd);
		}
		sec->status = 2;

		uint32_t toutRec = TOUT_WAIT_COM_ms;
		uint32_t RecIdx = 0;
		int cnt = 0;
		memset(sec->buf, 0, sec->lenbuf);
		while ((sec->loop) && ((sec->lenbuf - 1) > RecIdx) &&
					 ((cnt = SocketRecData(sec->confd, &sec->buf[RecIdx], sec->lenbuf - 1 - RecIdx, toutRec)) > 0)) {
			RecIdx += static_cast<unsigned int>(cnt);
			// Log_DEBUG("RecIdx [%d]; cnt [%d]", RecIdx, cnt);
			toutRec = sec->waitmore;//TOUT_WAIT_CONT_ms; // apos receber primeira parte reduz timeout
		};
		if (RecIdx > 0) {
			LmyMutexTryLock(sec->m_Rec); // limpa possivel liberacao anterior
			if (sec->_Cbs.onReceive(sec->buf, RecIdx) > 0) {
				LmyMutexUnlock(sec->m_Rec);
			}
			// Log_DEBUG("<%d;H%s;", RecIdx, SprintHex(&WorkBufTh[0], RecIdx));
		} else {
			if (cnt < 0) { // client error
				// sec->status = -1005;
				LibLog_WARNING("ServerEndClient: Port:%d sockfd:%d,fd:%d", sec->_Port, sec->sockfd, sec->confd);
				ServerEndClient(&sec->confd);
				if (sec->_Cbs.onDisconnect) sec->_Cbs.onDisconnect(-200);
			}
		}
	}

	sec->status = -1010;

END_TH:
	LibLog_DEBUG("IServer_ThFcn END %d", sec->status);
	sec->_thid = 0;
	return nullptr;
}

/*------------------------------------------------------------------------*//**
@fn			static void* IClient_ThFcn(void* arg)
*//*-------------------------------------------------------------------------*/
static void* IClient_ThFcn(void* arg) {
	IClientSec_ts* sec = (IClientSec_ts*)arg;
	
	LibLog_DEBUG("IClient_ThFcn IN");
	pthread_detach(pthread_self());

	sec->loop = true;
	while (sec->loop) {
		sec->status = 1;
		if (sec->sockfd < 0) {
			int sock = CreateInetSock();
			if (sock < 0) {
				sec->status = -11;
				break;
			}

			/* get the address of the host */
			struct hostent* hptr = gethostbyname(sec->_HostIp.c_str()); /* localhost: 127.0.0.1 */
			if (!hptr) {
				LibLog_ERR("ERROR gethostbyname %s", sec->_HostIp.c_str()); /* is hptr NULL? */
				ClientEndCom(&sock);
				sec->status = -12;
				break;
			}
			if (hptr->h_addrtype != AF_INET) {      /* versus AF_LOCAL */
				LibLog_ERR("bad address family");
				ClientEndCom(&sock);
				sec->status = -13;
				break;
			}

			/* connect to the server: configure server's address 1st */
			struct sockaddr_in saddr;
			memset(&saddr, 0, sizeof(saddr));
			saddr.sin_family = AF_INET;
			saddr.sin_addr.s_addr = ((struct in_addr*)hptr->h_addr_list[0])->s_addr;
			saddr.sin_port = htons(sec->_Port); /* port number in big-endian */

			while ((sec->loop) &&
						 (connect(sock, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			sec->sockfd = sock;

			if (sec->_Cbs.onConnect) sec->_Cbs.onConnect();
			LibLog_INFO("LS_CLiInet %s:%d connected; fd:%d", sec->_HostIp.c_str(), sec->_Port, sec->sockfd);
		}
		sec->status = 2;

		uint32_t toutRec = TOUT_WAIT_COM_ms;
		uint32_t RecIdx = 0;
		int cnt = 0;
		memset(sec->buf, 0, sec->lenbuf);

		while (((sec->lenbuf - 1) > RecIdx) &&
					 ((cnt = SocketRecData(sec->sockfd, &sec->buf[RecIdx], sec->lenbuf - 1 - RecIdx, toutRec)) > 0)) {
			RecIdx += static_cast<unsigned int>(cnt);
			// LibLog_DEBUG("RecIdx [%d]; cnt [%d]", RecIdx, cnt);
			toutRec = sec->waitmore;//TOUT_WAIT_CONT_ms; // apos receber primeira parte reduz timeout
		};

		if (RecIdx > 0) {
			if (sec->_Cbs.onReceive) {
				LmyMutexTryLock(sec->m_Rec); // limpa possivel liberacao anterior
				if (sec->_Cbs.onReceive(sec->buf, RecIdx) > 0) {
					LmyMutexUnlock(sec->m_Rec);
				}
			}
			// LibLog_DEBUG("<%d;H%s;", RecIdx, SprintHex(&WorkBufTh[0], RecIdx));
		} else {
			if (cnt < 0) { // client error
				if (sec->_Cbs.onDisconnect) sec->_Cbs.onDisconnect(-200);
				if (sec->sockfd) {
					LibLog_WARNING("CliSocket close: %s:%d fd:%d", sec->_HostIp.c_str(), sec->_Port, sec->sockfd);
					// close(sec->sockfd); /* close the connection */
					// sec->sockfd = -1;
					ClientEndCom(&(sec->sockfd));
				}
			}
		}
	}
	LibLog_DEBUG("IClient_ThFcn END");

	sec->_thid = 0;
	return nullptr;
}

/*############# Implementacao Exportada #####################################*/

/*------------------------------------------------------------------------*//**
@fn			LServerSec_ts* LS_LSrvCreateSock(std::string path, SockCallBacks_ts Cbs, uint32_t lenbuf)
*//*-------------------------------------------------------------------------*/
LServerSec_ts* LS_LSrvCreateSock(std::string path, SockCallBacks_ts Cbs, uint32_t lenbuf) {
	LServerSec_ts* sec = new LServerSec_ts;

	if (!sec) return nullptr;
	if (path.length() < 4) return nullptr;
	sec->m_Rec = LmyMutexNew();
	LibLog_DEBUG("LS_LSrvCreateSock m_Rec:%X", sec->m_Rec);
	if (!sec->m_Rec) {
		delete sec;
		return nullptr;
	}

	if (lenbuf < 128) lenbuf = 128; // protecao
	sec->buf = new uint8_t[lenbuf];
	if (!sec->buf) {
		LmyMutexDel(&sec->m_Rec);
		delete sec;
		return nullptr;
	}

	sec->_path = path;
	sec->_Cbs = Cbs;
	sec->lenbuf = lenbuf;
	LmyMutexTryLock(sec->m_Rec); // limpa possivel liberacao anterior

	do {
		if ((sec->sockfd = ServerInitLocal(path.c_str())) < 0) {
			sec->status = -11;
			break;
		}

		sec->status = 0;
		if (pthread_create(&sec->_thid, nullptr, &LServer_ThFcn, (void*)sec) == 0) {
			pthread_yield();
		} else {
			sec->status = -12;
			break;
		}
	} while (0);

	return sec;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LSrvDeleteSock(LServerSec_ts* sec, bool DeinitLog)
*//*-------------------------------------------------------------------------*/
int LS_LSrvDeleteSock(LServerSec_ts* sec, bool DeinitLog) {
	if (!sec) return -1;
	LibLog_INFO("LS_LSrvDeleteSock at: '%s'; sockfd:%d,fd:%d", sec->_path.c_str(), sec->sockfd, sec->confd);
	sec->loop = false;
	ServerEnd(&sec->sockfd, &sec->confd);
	pthread_cancel(sec->_thid);
	if (DeinitLog) LibLog_Deinit();
	LmyMutexDel(&sec->m_Rec);
	delete[] sec->buf;
	delete sec;
	return 0;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LSrvSendSock(LServerSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
int LS_LSrvSendSock(LServerSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms) {
	if (!sec) return -50;
	if (sec->status < 0) return -51;
	return SocketSendData(sec->confd, data, len, Toutms);
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LSrvWaitRecSock(LServerSec_ts* sec, uint32_t tpoms)
*//*-------------------------------------------------------------------------*/
int LS_LSrvWaitRecSock(LServerSec_ts* sec, uint32_t tpoms) {
	
	LibLog_DEBUG("LS_Srv Rec Waiting %dms delay", tpoms);
	if (tpoms <= 0) return -2;

	return LmyMutexWaitms(tpoms, true, sec->m_Rec);
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LSrvGetStatusSock(LServerSec_ts* sec)
*//*-------------------------------------------------------------------------*/
int LS_LSrvGetStatusSock(LServerSec_ts* sec) {
	if (!sec) return -1;
	return sec->status;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LSrvSetParam(LServerSec_ts* sec, LS_CfgParam_te par, int val)
*//*-------------------------------------------------------------------------*/
int LS_LSrvSetParam(LServerSec_ts* sec, LS_CfgParam_te par, int val) {
	switch (par)
	{
	case LS_Cfg_WaitMore: {
		sec->waitmore = val;
	} break;
	
	default:
		return -1;
	}

	return 0;
}

/*------------------------------------------------------------------------*//**
@fn			LClientSec_ts* LS_LCLiCreateSock(std::string path, SockCallBacks_ts Cbs, uint32_t lenbuf)
*//*-------------------------------------------------------------------------*/
LClientSec_ts* LS_LCLiCreateSock(std::string path, SockCallBacks_ts Cbs, uint32_t lenbuf) {
	LClientSec_ts* sec = new LClientSec_ts;

	if (!sec) return nullptr;
	if (path.length() < 4) return nullptr;
	sec->m_Rec = LmyMutexNew();
	LibLog_DEBUG("LS_LCLiCreateSock m_Rec:%X", sec->m_Rec);
	if (!sec->m_Rec) {
		delete sec;
		return nullptr;
	}

	if (lenbuf < 128) lenbuf = 128; // protecao
	sec->buf = new uint8_t[lenbuf];
	if (!sec->buf) {
		LmyMutexDel(&sec->m_Rec);
		delete sec;
		return nullptr;
	}

	LibLog_Init("libSockCom");
	LibLog_INFO("LS_LCLiCreateSock: %s", path.c_str());

	sec->_path = path;
	sec->_Cbs = Cbs;
	sec->lenbuf = lenbuf;
	LmyMutexTryLock(sec->m_Rec); // limpa possivel liberacao anterior

	do {
		sec->status = 0;
		if (pthread_create(&sec->_thid, nullptr, &LClient_ThFcn, (void*)sec) == 0) {
			pthread_yield();
		} else {
			sec->status = -12;
			break;
		}
	} while (0);

	return sec;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LCLiDeleteSock(LClientSec_ts* sec, bool DeinitLog)
*//*-------------------------------------------------------------------------*/
int LS_LCLiDeleteSock(LClientSec_ts* sec, bool DeinitLog) {
	if (!sec) return -1;
	LibLog_INFO("LS_LCLiDeleteSock at: '%s'; fd:%d", sec->_path.c_str(), sec->sockfd);
	sec->loop = false;
	ClientEndCom(&sec->sockfd);
	pthread_cancel(sec->_thid);
	if (DeinitLog) LibLog_Deinit();
	LmyMutexDel(&sec->m_Rec);
	delete[] sec->buf;
	delete sec;
	return 0;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LCLiSendSock(LClientSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
int LS_LCLiSendSock(LClientSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms) {
	if (!sec) return -50;
	if (sec->status < 0) return -51;
	return SocketSendData(sec->sockfd, data, len, Toutms);
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LCLiWaitRecSock(LClientSec_ts* sec, uint32_t tpoms)
*//*-------------------------------------------------------------------------*/
int LS_LCLiWaitRecSock(LClientSec_ts* sec, uint32_t tpoms) {
	
	LibLog_DEBUG("LS_CLi Rec Waiting %dms delay", tpoms);
	if (tpoms <= 0) return -2;

	return LmyMutexWaitms(tpoms, true, sec->m_Rec);
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LCliGetStatusSock(LClientSec_ts* sec)
*//*-------------------------------------------------------------------------*/
int LS_LCliGetStatusSock(LClientSec_ts* sec) {
	if (!sec) return -1;
	return sec->status;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_LCliSetParam(LClientSec_ts* sec, LS_CfgParam_te par, int val)
*//*-------------------------------------------------------------------------*/
int LS_LCliSetParam(LClientSec_ts* sec, LS_CfgParam_te par, int val) {
	switch (par)
	{
	case LS_Cfg_WaitMore: {
		sec->waitmore = val;
	} break;
	
	default:
		return -1;
	}

	return 0;
}

/*------------------------------------------------------------------------*//**
@fn			IServerSec_ts* LS_ISrvCreateSock(uint16_t Port, SockCallBacks_ts Cbs, uint32_t lenbuf)
*//*-------------------------------------------------------------------------*/
IServerSec_ts* LS_ISrvCreateSock(uint16_t Port, SockCallBacks_ts Cbs, uint32_t lenbuf) {
	IServerSec_ts* sec = new IServerSec_ts;

	if (!sec) return nullptr;
	sec->m_Rec = LmyMutexNew();
	LibLog_DEBUG("LS_ISrvCreateSock m_Rec:%X", sec->m_Rec);
	if (!sec->m_Rec) {
		delete sec;
		return nullptr;
	}
	
	if (lenbuf < 128) lenbuf = 128; // protecao
	sec->buf = new uint8_t[lenbuf];
	if (!sec->buf) {
		LmyMutexDel(&sec->m_Rec);
		delete sec;
		return nullptr;
	}

	sec->_Port = Port;
	sec->_Cbs = Cbs;
	sec->lenbuf = lenbuf;
	LmyMutexTryLock(sec->m_Rec); // limpa possivel liberacao anterior

	do {
		if ((sec->sockfd = ServerInitInet(Port)) < 0) {
			sec->status = -11;
			break;
		}

		sec->status = 0;
		if (pthread_create(&sec->_thid, nullptr, &IServer_ThFcn, (void*)sec) == 0) {
			pthread_yield();
		} else {
			sec->status = -12;
			break;
		}
	} while (0);

	return sec;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ISrvDeleteSock(IServerSec_ts* sec, bool DeinitLog)
*//*-------------------------------------------------------------------------*/
int LS_ISrvDeleteSock(IServerSec_ts* sec, bool DeinitLog) {
	if (!sec) return -1;
	LibLog_INFO("LS_ISrvDeleteSock port:%d; sockfd:%d,fd:%d", sec->_Port, sec->sockfd, sec->confd);
	sec->loop = false;
	ServerEnd(&sec->sockfd, &sec->confd);
	pthread_cancel(sec->_thid);
	if (DeinitLog) LibLog_Deinit();
	LmyMutexDel(&sec->m_Rec);
	delete[] sec->buf;
	delete sec;
	return 0;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ISrvSendSock(IServerSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
int LS_ISrvSendSock(IServerSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms) {
	if (!sec) return -50;
	if (sec->status < 0) return -51;
	return SocketSendData(sec->confd, data, len, Toutms);
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ISrvWaitRecSock(IServerSec_ts* sec, uint32_t tpoms)
*//*-------------------------------------------------------------------------*/
int LS_ISrvWaitRecSock(IServerSec_ts* sec, uint32_t tpoms) {
	
	LibLog_DEBUG("LS_Srv Rec Waiting %dms delay", tpoms);
	if (tpoms <= 0) return -2;

	return LmyMutexWaitms(tpoms, true, sec->m_Rec);
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ISrvGetStatusSock(IServerSec_ts* sec)
*//*-------------------------------------------------------------------------*/
int LS_ISrvGetStatusSock(IServerSec_ts* sec) {
	if (!sec) return -1;
	return sec->status;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ISrvSetParam(IServerSec_ts* sec, LS_CfgParam_te par, int val)
*//*-------------------------------------------------------------------------*/
int LS_ISrvSetParam(IServerSec_ts* sec, LS_CfgParam_te par, int val) {
	switch (par)
	{
	case LS_Cfg_WaitMore: {
		sec->waitmore = val;
	} break;
	
	default:
		return -1;
	}

	return 0;
}

/*------------------------------------------------------------------------*//**
@fn			IClientSec_ts* LS_ICLiCreateSock(std::string HostIp, uint16_t HostPort, SockCallBacks_ts Cbs, uint32_t lenbuf)
*//*-------------------------------------------------------------------------*/
IClientSec_ts* LS_ICLiCreateSock(std::string HostIp, uint16_t HostPort, SockCallBacks_ts Cbs, uint32_t lenbuf) {
	IClientSec_ts* sec = new IClientSec_ts;

	if (!sec) return nullptr;
	if (HostIp.length() < ((1+1)+(1+1)+(1+1)+(1+1))) return nullptr;
	sec->m_Rec = LmyMutexNew();
	LibLog_DEBUG("LS_ICLiCreateSock m_Rec:%X", sec->m_Rec);
	if (!sec->m_Rec) {
		delete sec;
		return nullptr;
	}

	if (lenbuf < 128) lenbuf = 128; // protecao
	sec->buf = new uint8_t[lenbuf];
	if (!sec->buf) {
		delete sec;
		return nullptr;
	}

	LibLog_Init("libSockCom");
	LibLog_INFO("LS_ICLiCreateSock: %s:%d", HostIp.c_str(), HostPort);

	sec->_HostIp = HostIp;
	sec->_Port = HostPort;
	sec->_Cbs = Cbs;
	sec->lenbuf = lenbuf;
	LmyMutexTryLock(sec->m_Rec); // limpa possivel liberacao anterior

	do {
		sec->status = 0;
		if (pthread_create(&sec->_thid, nullptr, &IClient_ThFcn, (void*)sec) == 0) {
			pthread_yield();
		} else {
			sec->status = -12;
			break;
		}
	} while (0);

	return sec;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ICLiDeleteSock(IClientSec_ts* sec, bool DeinitLog)
*//*-------------------------------------------------------------------------*/
int LS_ICLiDeleteSock(IClientSec_ts* sec, bool DeinitLog) {
	if (!sec) return -1;
	LibLog_INFO("LS_ICLiDeleteSock %s:%d; fd:%d", sec->_HostIp.c_str(), sec->_Port, sec->sockfd);
	sec->loop = false;
	ClientEndCom(&sec->sockfd);
	pthread_cancel(sec->_thid);
	if (DeinitLog) LibLog_Deinit();
	LmyMutexDel(&sec->m_Rec);
	delete[] sec->buf;
	delete sec;
	return 0;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ICLiSendSock(IClientSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms)
*//*-------------------------------------------------------------------------*/
int LS_ICLiSendSock(IClientSec_ts* sec, uint8_t* data, uint32_t len, uint32_t Toutms) {
	if (!sec) return -50;
	if (sec->status < 0) return -51;
	return SocketSendData(sec->sockfd, data, len, Toutms);
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ICLiWaitRecSock(IClientSec_ts* sec, uint32_t tpoms)
*//*-------------------------------------------------------------------------*/
int LS_ICLiWaitRecSock(IClientSec_ts* sec, uint32_t tpoms) {
	
	LibLog_DEBUG("LS_CLi Rec Waiting %dms delay", tpoms);
	if (tpoms <= 0) return -2;

	return LmyMutexWaitms(tpoms, true, sec->m_Rec);
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ICliGetStatusSock(IClientSec_ts* sec)
*//*-------------------------------------------------------------------------*/
int LS_ICliGetStatusSock(IClientSec_ts* sec) {
	if (!sec) return -1;
	return sec->status;
}

/*------------------------------------------------------------------------*//**
@fn			int LS_ICliSetParam(IClientSec_ts* sec, LS_CfgParam_te par, int val)
*//*-------------------------------------------------------------------------*/
int LS_ICliSetParam(IClientSec_ts* sec, LS_CfgParam_te par, int val) {
	switch (par)
	{
	case LS_Cfg_WaitMore: {
		sec->waitmore = val;
	} break;
	
	default:
		return -1;
	}

	return 0;
}


/*------------------------------------------------------------------------*//**
@fn			int LS_SetLogLevels(uint8_t lv)
*//*-------------------------------------------------------------------------*/
int LS_SetLogLevels(uint8_t lv) {
	return LibLog_SetLevel(lv);
}

