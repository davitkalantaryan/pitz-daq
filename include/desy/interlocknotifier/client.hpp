//
// file:		client.hpp
// created on:	2018 Jun 22
// to include:	#include <desy/interlocknotifier/client.hpp>


#ifndef __desy_interlocknotifier_client_hpp__
#define __desy_interlocknotifier_client_hpp__


#include <common/common_sockettcp.hpp>
#include <stdint.h>
#include "desy/interlocknotifier/server_client_common.h"
#include "desy/interlocknotifier/server_client_common.h"

namespace desy { namespace interlockNotifier{


class Client
{
public:
	virtual ~Client();

	int ConnectToServer(const char* serverName);
	void DisconnectFromServer();

	int ReceiveInterlockData(i4_mod_ctrl_t* a_pBuffer);

protected:
	virtual bool IsInterlock(const i4_mod_ctrl_t& a_ilockData);

protected:
	common::SocketTCP	m_socket;
	uint64_t			m_isEndianDiffer : 1;
};


}}  // namespace desy { namespace interlockNotifier{


#endif  // #ifndef __desy_interlocknotifier_client_hpp__
