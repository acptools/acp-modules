#ifndef MODULES_ACP_NETWORK_LIBS_HANDLING_SERVERS_INCLUDE_SERVERS_H_
#define MODULES_ACP_NETWORK_LIBS_HANDLING_SERVERS_INCLUDE_SERVERS_H_

#include <acp/core.h>
#include <Client.h>

namespace acp_network_libs_handling_srv {

	/********************************************************************************
	 * Base class representing a looping server.
	 ********************************************************************************/
	class LoopingServer {
	public:
		//--------------------------------------------------------------------------------
		// Realizes initialization actions.
		virtual void init() = 0;

		//--------------------------------------------------------------------------------
		// Realizes all looping actions.
		virtual void loop() = 0;

		//--------------------------------------------------------------------------------
		// Realizes closing actions.
		virtual void finalize() = 0;
	};

	/********************************************************************************
	 * Base class for implementations of client handlers that support management
	 * of server life cycle.
	 ********************************************************************************/
	class ClientHandlerWithServerSupport {
	public:
		//--------------------------------------------------------------------------------
		// Sets the server that exclusively uses this client handler.
		virtual void setServer(LoopingServer* server) = 0;

		//--------------------------------------------------------------------------------
		// Handles a client.
		virtual void handle(Client& client, bool isNew) = 0;
	};
}

/********************************************************************************
 * Base class for implementations of client handlers that support management
 * of server life cycle.
 ********************************************************************************/
template<typename SERVER, typename CLIENT> class SingleClientServer: public SERVER, acp_network_libs_handling_srv::LoopingServer {
private:
	// Client handler used by this server.
	acp_network_libs_handling_srv::ClientHandlerWithServerSupport* clientHandler;
public:

	//--------------------------------------------------------------------------------
	// Constructs a single client server with associated client handler and listening
	// at given port.
	SingleClientServer(uint16_t port, acp_network_libs_handling_srv::ClientHandlerWithServerSupport& clientHandler):SERVER(port) {
		this->clientHandler = &clientHandler;
		this->clientHandler->setServer(this);
	}

	//--------------------------------------------------------------------------------
	// Realizes initialization actions.
	virtual void init() {
		if (this->clientHandler != NULL) {
			this->begin();
		}
	}

	//--------------------------------------------------------------------------------
	// Realizes all looping actions.
	virtual void loop() {
		if (this->clientHandler == NULL) {
			return;
		}

		CLIENT client = this->available();
		if (client) {
			clientHandler->handle(client, true);
			this->available();
		}
	}

	//--------------------------------------------------------------------------------
	// Realizes closing actions.
	virtual void finalize() {
		this->clientHandler = NULL;
	}
};

#endif
