//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/getter/fromsocket.hpp"
#include <string.h>
#include "bin_for_mexdaq_browser_common.h"
#include <stdio.h>
#include <common/common_socketbase.hpp>
#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif
#else
#include <sys/wait.h>
//#include <unistd.h>
#endif

#define HELPER_BINARY_NAME  "bindaq_browser_actual"
//#define HELPER_BINARY_NAME  "bindaq_browser.3.2.0"
//#define HELPER_BINARY_NAME  "bindaq_browser.3.2.3"
//#define HELPER_BINARY_NAME  "bindaq_browser.4.1.0"

static void SignalHandler(int){}
static ssize_t ReadPipeStatic(int a_socket, void* a_pBuffer, size_t a_bufferSize);
static int CreateAndConnectSocket(int a_nIndex, void* a_pOwner,const char* a_serverName, int a_serverPort, uint16_t* a_pEndien);
static void DisconnectStatic(const int sockets[]);
static int WriteViaTcpSocket(int a_socket, const void* a_cpBuffer, int a_nSize);

using namespace pitz::daq;


data::getter::FromSocket::FromSocket(const char* a_serverName, int a_serverPort)
	:
	getter::BaseTmp<getter::NoIndexing,engine::ByPipe>(ReadPipeStatic)
{
	m_isEndianDiffer = 0;
	if(a_serverName && (a_serverPort>0)){this->connect(a_serverName,a_serverPort);}
}


data::getter::FromSocket::~FromSocket()
{
}


int data::getter::FromSocket::connect(const char* a_serverName, int a_serverPort)
{
	if(!m_engine.IsPipesSet()){
		const int cnNumberOfPipes((int)byPipe::pipePurpose::Count);
		int vnPipes[cnNumberOfPipes];
		for(int i(0);i<cnNumberOfPipes;++i){
			vnPipes[i] = CreateAndConnectSocket(i,this,a_serverName, a_serverPort,&m_isEndianDiffer); if (vnPipes[i] < 0) { goto errorReturn; }
		}
		m_engine.SetPipes(vnPipes);
		return 0;
	errorReturn:
		DisconnectStatic(vnPipes);
		return -1;
	}
	else {
		MAKE_WARNING_THIS("Pipes are already assigned");
	}
	return 0;
}


void data::getter::FromSocket::disconnet()
{
	int	pipes[byPipe::pipePurpose::Count];
	
	DisconnectStatic(m_engine.pipes());
	memset(pipes,-1,byPipe::Base::sm_cnPipesBufferSize);
	m_engine.SetPipes(pipes);
}


int data::getter::FromSocket::GetEntriesInfo(const char* a_rootFileName)
{
	const int* cpnPipes = m_engine.pipes();
	const int cnStrSize = (int)strlen(a_rootFileName) + 1;
	int nReturn;

	SetFilter(filter::Type::FileEntriesInfo);

	if( WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr],&m_filter,sizeof(m_filter))!= sizeof(m_filter)){return -1;}
	if( WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr],&cnStrSize,4)!=4 ){return -1;}
	if( WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr],a_rootFileName,cnStrSize)!= cnStrSize){return -1;}

	nReturn = getter::NoIndexing::GetEntriesInfo(a_rootFileName);

	return nReturn;
}


int data::getter::FromSocket::GetMultipleEntries(const char* a_rootFileName, const ::std::vector< ::std::string >& a_branchNames)
{
	::std::string strAllBranches;
	const int* cpnPipes = m_engine.pipes();
	int cnStrSize = (int)strlen(a_rootFileName) + 1;
	const int cnNumberOfBranchesMin1((int)a_branchNames.size());
	int i,nReturn;

	if(cnNumberOfBranchesMin1<0){return -1;}

	SetFilter(filter::Type::MultyBranchFromFile);

	for(i=0;i<cnNumberOfBranchesMin1;++i){
		strAllBranches += a_branchNames[i].c_str();
		strAllBranches += ";";
	}
	strAllBranches += a_branchNames[cnNumberOfBranchesMin1].c_str();

	if (WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr], &m_filter, sizeof(m_filter)) != sizeof(m_filter)) { return -1; }
	if (WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr], &cnStrSize, 4) != 4) { return -1; }
	if (WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr], a_rootFileName, cnStrSize) != cnStrSize) { return -1; }
	cnStrSize = (int)strAllBranches.size();
	if (WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr], &cnStrSize, 4) != 4) { return -1; }
	if (WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr],strAllBranches.c_str(), cnStrSize) != cnStrSize) { return -1; }

	nReturn = getter::NoIndexing::GetMultipleEntries(a_rootFileName, a_branchNames);

	return nReturn;
}


int data::getter::FromSocket::GetMultipleEntriesTI(const ::std::vector< ::std::string >& a_branchNames, int a_startTime, int a_endTime)
{
	::std::string strAllBranches;
	const int* cpnPipes = m_engine.pipes();
	int cnStrSize;
	const int cnNumberOfBranchesMin1((int)a_branchNames.size()-1);
	int i,nReturn;
	
	if (cnNumberOfBranchesMin1 < 0) { return -1; }

	SetFilter(filter::Type::ByTime2,a_startTime,a_endTime);

	for (i = 0; i < cnNumberOfBranchesMin1; ++i) {
		strAllBranches += a_branchNames[i].c_str();
		strAllBranches += ";";
	}
	strAllBranches += a_branchNames[cnNumberOfBranchesMin1].c_str();
	cnStrSize = (int)strAllBranches.size();

	if (WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr], &m_filter, sizeof(m_filter)) != sizeof(m_filter)) { return -1; }
	if (WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr], &cnStrSize, 4) != 4) { return -1; }
	if (WriteViaTcpSocket(cpnPipes[byPipe::pipePurpose::Cntr], strAllBranches.c_str(), cnStrSize) != cnStrSize) { return -1; }

	nReturn = getter::NoIndexing::GetMultipleEntriesTI(a_branchNames,a_startTime,a_endTime);

	return nReturn;
}


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

static ssize_t ReadPipeStatic(int a_socket, void* a_pBuffer, size_t a_bufferSize)
{
	return recv(a_socket, (char*)a_pBuffer,(int)a_bufferSize, MSG_WAITALL);
}


static void DisconnectStatic(const int a_sockets[])
{
	const int cnNumberOfPipes((int)data::byPipe::pipePurpose::Count);
	for(int i(0);i< cnNumberOfPipes;++i){
		if (a_sockets[i] > 0){
#ifdef	_WIN32
			closesocket(a_sockets[i]);
#else
			close(a_sockets[i]);
#endif
		}
	}
}


static int WriteViaTcpSocket(int a_socket, const void* a_cpBuffer, int a_nSize)
{
#define MAX_NUMBER_OF_ITERS	100000
	const char* pcBuffer = (const char*)a_cpBuffer;
	const char *cp = NULL;
	int len_to_write = 0;
	int len_wrote = 0;
	int n = 0;

	len_to_write = a_nSize;
	cp = pcBuffer;
	for (int i(0); len_to_write > 0; ++i)
	{
		n = ::send(a_socket, cp, len_to_write, 0);
		if (CHECK_FOR_SOCK_ERROR(n)) {
			if (SOCKET_WOULDBLOCK(errno)) {
				if (i < MAX_NUMBER_OF_ITERS) { SWITCH_SCHEDULING(0); continue; }
				else { return _SOCKET_TIMEOUT_; }
			}
			else { return E_SEND; }
		}
		else {
			cp += n;
			len_to_write -= n;
			len_wrote += n;
		}
	}

	return len_wrote;
}


static int CreateAndConnectSocket(int a_nIndex, void* a_pOwner, const char* a_svrName, int a_port, uint16_t* a_pIsEndienDiffer)
{
	const char *host = NULL;
	ComunicationStruct comStr;
	fd_set rfds;
	struct sockaddr_in addr;
	unsigned long ha;
	int rtn = -1;
	int maxsd = 0;
	int addr_len;
	int nSocket, nSndRcv;
	uint16_t unRemEndianHint;
	char l_host[MAX_HOSTNAME_LENGTH];

	nSocket = (int)::socket(AF_INET, SOCK_STREAM, 0);
	if (CHECK_FOR_SOCK_INVALID(nSocket)) {return E_NO_SOCKET;}

	host = a_svrName;
	if (host == NULL || *host == '\0') {
		if (::gethostname(l_host, MAX_HOSTNAME_LENGTH) < 0) { return E_UNKNOWN_HOST; }
		host = l_host;
	}

	memset((char *)&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(a_port);

	if ((ha = inet_addr(host)) == INADDR_NONE) {
		struct hostent* hostent_ptr = gethostbyname(a_svrName);
		if (!hostent_ptr) { return E_UNKNOWN_HOST; }
		a_svrName = inet_ntoa(*(struct in_addr *)hostent_ptr->h_addr_list[0]);
		if ((ha = inet_addr(a_svrName)) == INADDR_NONE) { return E_UNKNOWN_HOST; }
	}

	memcpy((char *)&addr.sin_addr, (char *)&ha, sizeof(ha));

#ifdef	_WIN32
	unsigned long on = 1;
	ioctlsocket(nSocket, FIONBIO, &on);
#else  /* #ifdef	_WIN32 */
	int status;
	if ((status = fcntl(nSocket, F_GETFL, 0)) != -1) {
		status |= O_NONBLOCK;
		fcntl(nSocket, F_SETFL, status);
	}
#endif  /* #ifdef	_WIN32 */

	addr_len = sizeof(addr);
	rtn = ::connect(nSocket, (struct sockaddr *) &addr, addr_len);

	if (rtn != 0) {
		struct timeval  aTimeout;
		int nErrno = errno;///?
		if (!SOCKET_INPROGRESS(nErrno)) { return E_NO_CONNECT; }

		FD_ZERO(&rfds);
		FD_SET((unsigned int)nSocket, &rfds);
		maxsd = (int)(nSocket + 1);

		aTimeout.tv_sec = 1;
		aTimeout.tv_usec = 100;

		rtn = ::select(maxsd, (fd_set *)0, &rfds, (fd_set *)0, &aTimeout);

		switch (rtn)
		{
		case 0:	/* time out */
			return _SOCKET_TIMEOUT_;
		case SOCKET_ERROR:
			if (errno == EINTR) {/*interrupted by signal*/return _EINTR_ERROR_; }
			return E_SELECT;
		default:
			rtn = 0;
			break;
		}

		if (!FD_ISSET(nSocket, &rfds)) { return E_FATAL; }
	}

#ifdef	_WIN32
	on = 0;
	ioctlsocket(nSocket, FIONBIO, &on);
#else  /* #ifdef	_WIN32 */
	if ((status = fcntl(nSocket, F_GETFL, 0)) != -1) {
		status &= ~O_NONBLOCK;
		fcntl(nSocket, F_SETFL, status);
	}
#endif  /* #ifdef	_WIN32 */

	nSndRcv = recv(nSocket,(char*)&unRemEndianHint,2, MSG_WAITALL);
	if( nSndRcv!=2 ){closesocket(nSocket);return -1;}
	*a_pIsEndienDiffer = (unRemEndianHint == 1) ? 0 : 1;

	comStr.index = a_nIndex;
	comStr.ptr = (int64_t)((size_t)a_pOwner);
	comStr.pid = getpid();
	if( WriteViaTcpSocket(nSocket,&comStr,sizeof(comStr))!=sizeof(comStr) ){closesocket(nSocket);return -1;}

	return nSocket;
}
