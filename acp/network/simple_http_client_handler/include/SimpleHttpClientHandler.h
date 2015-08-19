#ifndef MODULES_ACP_NETWORK_SIMPLE_HTTP_CLIENT_HANDLER_INCLUDE_SIMPLEHTTPCLIENTHANDLER_H_
#define MODULES_ACP_NETWORK_SIMPLE_HTTP_CLIENT_HANDLER_INCLUDE_SIMPLEHTTPCLIENTHANDLER_H_

#include <acp/core.h>
#include <acp/debug/tracer/Tracer.h>
#include <acp/network/libs/handling_servers/Servers.h>

#include <Client.h>

namespace acp_network_simple_http_client_handler {

	template<int BUFFER_SIZE, long TIMEOUT> class TSimpleHttpClientHandler;
	template<int BUFFER_SIZE, long TIMEOUT> class SimpleHttpHandlingController;
	struct RequestProcessingData;

	/********************************************************************************
	 * Configuration of features applied to process an http request
	 ********************************************************************************/
	struct Features {
		// Indicates whether get parameters are retrieved from the request
		// Usage: saving memory
		bool storeGetParameters;
		// Indicates whether post parameters are retrieved from the request
		// Usage: saving memory
		bool storePostParameters;
		// Sets authentication settings for Basic HTTP authentication.
		// A pointer to a string stored in flash memory is required.
		// Format: Base64EncodedUserPassword[:Realm]
		// Default realm is "web"
		const __FlashStringHelper* authentication;
		// Indicates whether CORS (Cross-origin resource sharing) is enabled
		bool enableCORS;
	};

	/********************************************************************************
	 * Name-value pair of a GET or POST parameter
	 ********************************************************************************/
	class HttpParameter {
	public:
		// Name of parameter
		const char* name;
		// Value of parameter
		const char* value;

		//--------------------------------------------------------------------------------
		// Returns whether name matches given string
		bool nameEquals(const char* str) {
			if (name == NULL) {
				return false;
			}

			return (strcmp(str, name) == 0);
		}

		//--------------------------------------------------------------------------------
		// Returns whether name matches given string
		bool nameEquals(const __FlashStringHelper* fstr) {
			if (name == NULL) {
				return false;
			}

			PGM_P str = reinterpret_cast<PGM_P>(fstr);
			const char* nameChar = name;
			while (*nameChar != 0) {
				if (*nameChar != pgm_read_byte(str)) {
					return false;
				}

				str++;
				nameChar++;
			}

			return (*nameChar == pgm_read_byte(str));
		}
	};

	/********************************************************************************
	 * Request method supported by the http client handler.
	 *********************************************************************************/
	struct HttpMethod
	{
		enum Type
		{
			GET, POST, OPTIONS
		};
		Type t_;
		HttpMethod(Type t) : t_(t) {}
		operator Type () const {return t_;}
	private:
		template<typename T>
		operator T () const;
	};

	/********************************************************************************
	 * Internal bundle with request processing data
	 ********************************************************************************/
	struct RequestProcessingData {
		// Time when the request started
		unsigned long startTime;

		// Request method
		HttpMethod method;
		// String with requested url
		const char* url;
		// String with received get parameters
		char* getParameters;
		// String with received post parameters
		char* postParameters;

		// Origin header (usually sent in CORS requests)
		const char* originHeader;
		// Content-length header (negative number if not set in http headers)
		int contentLength;

		// Receive buffer
		uint8_t* buffer;
		// Size of buffer
		int bufferSize;
		// Number of unprocessed received bytes stored in received buffer.
		int bufferedBytes;

		// Constructor with default values
		RequestProcessingData():url(NULL), getParameters(NULL), postParameters(NULL), method(HttpMethod::GET), originHeader(NULL), contentLength(-1) {}
	};

	/********************************************************************************
	 * Bundle with data retrieved from an http request
	 ********************************************************************************/
	class HttpRequest {
	private:
		// Bundle with request data created during processing http request
		RequestProcessingData& requestData;

		//--------------------------------------------------------------------------------
		// Decodes next parameter from an URL encoded query string
		HttpParameter decodeNextParameter(char* &queryString) {
			HttpParameter result;
			result.name = NULL;
			result.value = NULL;

			if ((queryString == NULL) || (*queryString == 0)) {
				return result;
			}

			result.name = (const char*)queryString;

			char* writeIdx = queryString;
			while ((*queryString != 0) && (*queryString != '&')) {
				// Decode character encoded as hexadecimal number
				if (*queryString == '%') {
					byte hex[2];
					hex[0] = 255;
					hex[1] = 255;
					for (int i=0; i<2; i++) {
						queryString++;
						char c = *queryString;
						if (('0' <= c) && (c <= '9')) {
							hex[i] = c - '0';
							continue;
						}

						if (('A' <= c) && (c <= 'F')) {
							hex[i] = (c - 'A') + 10;
							continue;
						}

						if (('a' <= c) && (c <= 'f')) {
							hex[i] = (c - 'a') + 10;
							continue;
						}

						// If non-hex character is detected, we end the loop
						break;
					}

					if ((hex[0] != 255) && (hex[1] != 255)) {
						queryString++;
						*writeIdx = (char)(hex[0]*16+hex[1]);
						writeIdx++;
					}

					continue;
				}

				// Decode space
				if (*queryString == '+') {
					*writeIdx = ' ';
					writeIdx++;
					queryString++;
					continue;
				}

				// End of name part
				if ((result.value == NULL) && (*queryString == '=')) {
					*writeIdx = 0;
					writeIdx++;
					result.value = (const char*)writeIdx;
					queryString++;
					continue;
				}

				// We copy all remaining characters
				*writeIdx = *queryString;
				writeIdx++;
				queryString++;
			}

			// If the last character was &, then move reading position to next character
			while (*queryString == '&') {
				queryString++;
			}

			// Finish reading with terminating character
			*writeIdx = 0;

			// Complete value part if necessary
			if (result.value == NULL) {
				result.value = (const char*)writeIdx;
			}

			// Set query pointer to NULL, if the end of query is reached.
			if (*queryString == 0) {
				queryString = NULL;
			}

			return result;
		}
	public:
		//--------------------------------------------------------------------------------
		// Constructs bundle with request data.
		HttpRequest(RequestProcessingData& requestProcessingData):requestData(requestProcessingData) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Returns http request method.
		inline HttpMethod getMethod() {
			return requestData.method;
		}

		//--------------------------------------------------------------------------------
		// Returns requested url
		inline const char* getUrl() {
			return requestData.url;
		}

		//--------------------------------------------------------------------------------
		// Returns whether url starts with given string.
		bool urlStartsWith(const char* str) {
			if ((str == NULL) || (requestData.url == NULL)) {
				return false;
			}

			const char* urlPtr = requestData.url;
			while (*str != 0) {
				if ((*urlPtr == 0) || (*str != *urlPtr)) {
					return false;
				}

				str++;
				urlPtr++;
			}

			return true;
		}

		//--------------------------------------------------------------------------------
		// Returns whether url starts with given string.
		bool urlStartsWith(const __FlashStringHelper* fstr) {
			if ((fstr == NULL) || (requestData.url == NULL)) {
				return false;
			}

			PGM_P str = reinterpret_cast<PGM_P>(fstr);
			const char* urlPtr = requestData.url;
			char c = pgm_read_byte(str);
			while (c != 0) {
				if ((*urlPtr == 0) || (c != *urlPtr)) {
					return false;
				}

				str++;
				urlPtr++;
				c = pgm_read_byte(str);
			}

			return true;
		}

		//--------------------------------------------------------------------------------
		// Returns whether there is unprocessed GET parameter
		inline bool hasNextGetParameter() {
			return (requestData.getParameters != NULL) && (*(requestData.getParameters) != 0);
		}

		//--------------------------------------------------------------------------------
		// Returns whether there is unprocessed POST parameter
		inline bool hasNextPostParameter() {
			return (requestData.postParameters != NULL) && (*(requestData.postParameters) != 0);
		}

		//--------------------------------------------------------------------------------
		// Returns next GET parameter.
		// If no next parameter is available, the name and value of parameter are set to NULL.
		inline HttpParameter nextGetParameter() {
			return decodeNextParameter(requestData.getParameters);
		}

		//--------------------------------------------------------------------------------
		// Returns next POST parameter.
		// If no next parameter is available, the name and value of parameter are set to NULL.
		inline HttpParameter nextPostParameter() {
			return decodeNextParameter(requestData.postParameters);
		}
	};

	/********************************************************************************
	 * Response builder
	 ********************************************************************************/
	class HttpResponse {
	private:
		// Client that produces the response
		Client* client;

		// Allow origin field required to support CORS
		const char* corsAllowOrigin;

		// State of the output.
		// bit 0 - http status printed
		// bit 1 - http headers closed
		// bit 2 - allow-credentials for CORS enabled/disabled
		uint8_t state;

		//--------------------------------------------------------------------------------
		// Ensures that the status line was printed.
		void ensureStatusPrinted() {
			if ((state & B00000001) == B00000001) {
				return;
			}

			setStatus(200, F("OK"));
		}

		//--------------------------------------------------------------------------------
		// Closes the header section in generated http response.
		void closeHeaders() {
			ensureStatusPrinted();

			// Generate headers for CORS
			if (corsAllowOrigin != NULL) {
				header(F("Access-Control-Allow-Origin"), corsAllowOrigin);
				if ((state & B00000100) == B00000100) {
					header(F("Access-Control-Allow-Credentials"), F("true"));
					header(F("Access-Control-Allow-Headers"), F("Authorization"));
				}
			}

			// Required closing of connection after completing the request.
			client->println(F("Connection: close"));

			// Print separation line after header fields.
			client->println();

			// Set that headers are closed.
			state |= B00000010;
		}

	public:
		//--------------------------------------------------------------------------------
		// Constructs an http response for given client.
		HttpResponse(Client* client, const char* corsAllowOrigin, bool corsAllowCredentials):client(client), corsAllowOrigin(corsAllowOrigin), state(0) {
			if (corsAllowCredentials) {
				state |= B00000100;
			}
		}

		//--------------------------------------------------------------------------------
		// Sets the response status.
		void setStatus(int statusCode, const char* reasonPhrase) {
			// Check whether http status has been printed
			if ((state & B00000001) == B00000001) {
				return;
			}

			client->print(F("HTTP/1.1 "));
			client->print(statusCode);
			client->print(' ');
			client->println(reasonPhrase);
			state |= B00000001;
		}

		//--------------------------------------------------------------------------------
		// Sets the response status.
		void setStatus(int statusCode, const __FlashStringHelper* reasonPhrase) {
			// Check whether http status has been printed
			if ((state & B00000001) == B00000001) {
				return;
			}

			client->print(F("HTTP/1.1 "));
			client->print(statusCode);
			client->print(' ');
			client->println(reasonPhrase);
			state |= B00000001;
		}

		//--------------------------------------------------------------------------------
		// Send an http header.
		void header(const char* fieldName, const char* fieldValue) {
			// Check whether headers can be sent
			if ((state & B00000010) == B00000010) {
				return;
			}

			ensureStatusPrinted();
			client->print(fieldName);
			client->print(':');
			client->print(' ');
			client->println(fieldValue);
		}

		//--------------------------------------------------------------------------------
		// Send an http header.
		void header(const __FlashStringHelper* fieldName, const char* fieldValue) {
			// Check whether headers can be sent
			if ((state & B00000010) == B00000010) {
				return;
			}

			ensureStatusPrinted();
			client->print(fieldName);
			client->print(':');
			client->print(' ');
			client->println(fieldValue);
		}

		//--------------------------------------------------------------------------------
		// Send an http header.
		void header(const __FlashStringHelper* fieldName, const __FlashStringHelper* fieldValue) {
			// Check whether headers can be sent
			if ((state & B00000010) == B00000010) {
				return;
			}

			ensureStatusPrinted();
			client->print(fieldName);
			client->print(':');
			client->print(' ');
			client->println(fieldValue);
		}

		//--------------------------------------------------------------------------------
		// Returns Print object for writing header fields.
		Print* getHeaderPrint() {
			// Check whether headers can be sent
			if ((state & B00000010) == B00000010) {
				return NULL;
			}

			ensureStatusPrinted();
			return client;
		}

		//--------------------------------------------------------------------------------
		// Starts production of the response content with undefined content type.
		inline Print* startContent() {
			return startContent((const char*)NULL);
		}

		//--------------------------------------------------------------------------------
		// Starts production of the response content of given type and returns pointer to
		// a Print object to which the content should be printed/written.
		Print* startContent(const char* contentType) {
			// If header are closed, we do nothing
			if ((state & B00000010) == B00000010) {
				return client;
			}

			// Produce content type
			if (contentType != NULL) {
				header(F("Content-Type"), contentType);
			}

			closeHeaders();
			return client;
		}

		//--------------------------------------------------------------------------------
		// Starts production of the response content of given type and returns pointer to
		// a Print object to which the content should be printed/written.
		Print* startContent(const __FlashStringHelper* contentType) {
			// If header are closed, we do nothing
			if ((state & B00000010) == B00000010) {
				return client;
			}

			// Produce content type
			if (contentType != NULL) {
				header(F("Content-Type"), contentType);
			}

			closeHeaders();
			return client;
		}
	};

	/********************************************************************************
	 * Controller for a simple http client handler.
	 ********************************************************************************/
	template<int BUFFER_SIZE, long TIMEOUT> class SimpleHttpHandlingController {
		friend class TSimpleHttpClientHandler<BUFFER_SIZE, TIMEOUT>;
	private:
		// Server managed by (associated with) this client handler.
		acp_network_libs_handling_srv::LoopingServer* server;

		// State of the controller
		// bit 0 indicates whether the associated server was initialized
		// bit 1 indicates whether CORS support is enabled
		uint8_t state;

		// Authentication settings (see Features struct for more details)
		const __FlashStringHelper* authentication;

		//--------------------------------------------------------------------------------
		// Reads at most given number of bytes from client to buffer.
		int readBytes(Client& client, uint8_t* buffer, int length) {
			const bool useBlockRead = true;
			if (useBlockRead) {
				return client.read(buffer, length);
			} else {
				int receivedBytes = 0;
				while (client.connected() && client.available() && (receivedBytes < length)) {
					int ch = client.read();
					if (ch >= 0) {
						*buffer = (char)ch;
						buffer++;
						receivedBytes++;
					}
				}
				return receivedBytes;
			}
		}

		//--------------------------------------------------------------------------------
		// Fills the line buffer (the reading finishes when end of line is reached or the buffer is filled)
		// and returns the length of the line prefix (including the LF character).
		// Returns 0, if the client closes the session
		// Returns -1, if the line is too long to fit into the buffer
		// Returns -2, if timeout with respect to requestStartTime occurred
		int readLine(Client& client, uint8_t* buffer, int& bufferedBytes, const int bufferSize, unsigned long requestStartTime) {
			// Check whether the buffer contains the LF character
			if (bufferedBytes > 0) {
				uint8_t* dataPtr = buffer;
				uint8_t* const dataEnd = buffer+bufferedBytes;
				while (dataPtr != dataEnd) {
					if (*dataPtr == '\n') {
						return dataPtr - buffer + 1;
					}
					dataPtr++;
				}
			}

			// Read data from client
			int freeSize = bufferSize - bufferedBytes;
			while (client.connected() && (freeSize > 0)) {
				// Check timeout
				if (millis() - requestStartTime > TIMEOUT) {
					return -2;
				}

				// Request number of available bytes
				int availableBytes = client.available();
				if (availableBytes > 0) {
					// Eliminate the buffer overflow
					if (availableBytes > freeSize) {
						availableBytes = freeSize;
					}

					// Read data
					uint8_t* dataPtr = buffer+bufferedBytes;
					const int receivedBytes = readBytes(client, dataPtr, availableBytes);
					if ((receivedBytes < 0) || (receivedBytes > availableBytes)) {
						// Something went wrong - disconnect client
						closeConnection(client);
						ACP_TRACE(F("HTTP: Wrong state."));
						return 0;
					}

					bufferedBytes += receivedBytes;
					freeSize -= receivedBytes;
					uint8_t* const dataEnd = buffer+bufferedBytes;

					// Check whether received data contain a new line separator
					while (dataPtr != dataEnd) {
						if (*dataPtr == '\n') {
							return dataPtr - buffer + 1;
						}
						dataPtr++;
					}
				} else {
					ACP_TRACE(F("HTTP: Waiting for data."));
					delay(1);
				}
			}

			return (freeSize == 0) ? -1 : 0;
		}

		//--------------------------------------------------------------------------------
		// Read the data until incoming bytes do not contain the LF character.
		void skipToLineEnd(Client& client, uint8_t* buffer, int& bufferedBytes, const int bufferSize, unsigned long requestStartTime) {
			// Read data from client
			while (client.connected()) {
				// Check timeout
				if (millis() - requestStartTime > TIMEOUT) {
					return;
				}

				int availableBytes = client.available();
				if (availableBytes > 0) {
					// Eliminate the buffer overflow
					if (availableBytes > bufferSize) {
						availableBytes = bufferSize;
					}

					// Read data
					bufferedBytes = readBytes(client, buffer, availableBytes);
					if ((bufferedBytes < 0) || (bufferedBytes > availableBytes)) {
						// Something went wrong - disconnect client
						closeConnection(client);
						ACP_TRACE(F("HTTP: Wrong state."));
						return;
					}

					uint8_t* dataPtr = buffer;
					uint8_t* const dataEnd = buffer+bufferedBytes;

					// Check whether received data contain a new line separator
					while (dataPtr != dataEnd) {
						if (*dataPtr == '\n') {
							dataPtr++;
							break;
						}
						dataPtr++;
					}

					// If LF is detected, we copy
					if (dataPtr != dataEnd) {
						// Copy data received after the LF character to the beginning of the buffer
						bufferedBytes = dataEnd - dataPtr;
						while (dataPtr != dataEnd) {
							*buffer = *dataPtr;
							dataPtr++;
							buffer++;
						}
						return;
					}
				} else {
					ACP_TRACE(F("HTTP: Waiting for data."));
					delay(1);
				}
			}
		}

		//--------------------------------------------------------------------------------
		// Closes the client
		void closeConnection(Client& client) {
			client.flush();
			delay(1);
			client.stop();
		}

		//--------------------------------------------------------------------------------
		// Removes given number of bytes from the beginning of the buffer.
		void removeBufferPrefix(uint8_t* buffer, int& bufferedBytes, int prefixLength) {
			uint8_t* dataPtr = buffer + prefixLength;
			uint8_t* const dataEnd = buffer + bufferedBytes;
			while (dataPtr != dataEnd) {
				*buffer = *dataPtr;
				buffer++;
				dataPtr++;
			}

			bufferedBytes -= prefixLength;
		}

		//--------------------------------------------------------------------------------
		// Locates character in block of memory.
		// Returns the index of the first occurrence or -1, if the character is not found.
		int locateCharacter(const uint8_t* ptr, int value, int num) {
			for (int i=0; i<num; i++) {
				if (*ptr == value) {
					return i;
				}

				ptr++;
			}

			return -1;
		}

		//--------------------------------------------------------------------------------
		// Checks whether the content of a buffer starts with a given string.
		bool startsWithFString(const uint8_t* ptr, const __FlashStringHelper* ifsh) {
			PGM_P p = reinterpret_cast<PGM_P>(ifsh);
			while (true) {
				const unsigned char c = pgm_read_byte(p);
				if (c == 0) {
					return true;
				}

				if (c != *ptr) {
					return false;
				}

				ptr++;
				p++;
			}
		}

		//--------------------------------------------------------------------------------
		// Handles an invalid state.
		void handleInvalidState(Client& client, int errorCode, unsigned long requestStartTime) {
			// Log error
			if (errorCode == 0) {
				ACP_TRACE(F("HTTP: client disconnected before request completed."));
			} else if (errorCode == -1) {
				ACP_TRACE(F("HTTP: request (receive) buffer congested."));
			} else if (errorCode == -2) {
				ACP_TRACE(F("HTTP: timeout."));
			} else {
				ACP_TRACE(F("HTTP: unexpected error (%d)."), errorCode);
			}

			// If the client is not connected, it is not possible to inform the client
			if (!client.connected()) {
				return;
			}

			client.println(F("HTTP/1.1 500 Internal Error"));
			client.println(F("Connection: close"));
			closeConnection(client);
		}

		//--------------------------------------------------------------------------------
		// Realizes basic http authentication (including the 401 response, if necessary)
		// Returns true, if the user is authorized, false otherwise.
		bool handleBasicAuthentication(Client& client, const uint8_t* authHeaderField, const __FlashStringHelper* authSetting) {
			if (authSetting == NULL) {
				return true;
			}

			PGM_P settingPtr = reinterpret_cast<PGM_P>(authSetting);
			bool authenticated = true;

			// Check basic authentication method
			while ((*authHeaderField != 0) && (*authHeaderField != ':')) {
				authHeaderField++;
			}

			if (*authHeaderField != 0) {
				authHeaderField++;
			}

			while (*authHeaderField == ' ') {
				authHeaderField++;
			}

			if (startsWithFString((const uint8_t*)authHeaderField, F("Basic "))) {
				authHeaderField += 6;
			} else {
				authenticated = false;
			}

			if (authenticated) {
				// Check whether authorization data match content of the field
				while (true) {
					const char c = pgm_read_byte(settingPtr);
					// Check whether authentication settings reached the end of login:password part
					if ((c == 0) || (c == ':')) {
						authenticated = (*authHeaderField == '\r') || (*authHeaderField == 0);
						break;
					}

					// Check whether required and received authentication strings match
					if (*authHeaderField != c) {
						authenticated = false;
						break;
					}

					settingPtr++;
					authHeaderField++;
				}
			}

			return authenticated;
		}

		//--------------------------------------------------------------------------------
		// Handles a client.
		void handle(Client& client) {
			// Check whether the client object represents a valid client.
			if (!client) {
				return;
			}

			ACP_TRACE(F("HTTP: new client."));

			// Buffer for storing all request related data (URL, parameters, etc.)
			uint8_t requestBuffer[BUFFER_SIZE + 1];
			requestBuffer[BUFFER_SIZE] = 0;

			// Bundle with data retrieved from the request
			RequestProcessingData requestData;

			// Initialized floating receive buffer inside request buffer
			requestData.buffer = requestBuffer;// start of receive buffer
			requestData.bufferedBytes = 0;// number of received bytes in the buffer
			requestData.bufferSize = BUFFER_SIZE;// size of the receive buffer

			// Store start time
			requestData.startTime = millis();

			// Initialize features
			Features features;
			features.storeGetParameters = true;
			features.storePostParameters = true;
			features.enableCORS = ((state & B00000010) == B00000010);
			features.authentication = authentication;

			// Read the request line
			{
				int lineLength = readLine(client, requestData.buffer, requestData.bufferedBytes, requestData.bufferSize, requestData.startTime);
				if (lineLength <= 0) {
					handleInvalidState(client, lineLength, requestData.startTime);
					return;
				}

				int spaceIdx = locateCharacter(requestData.buffer, ' ', lineLength);
				if (spaceIdx < 0) {
					// Protocol error
					handleInvalidState(client, -10, requestData.startTime);
					return;
				}

				// Detect method
				if ((spaceIdx == 3) && startsWithFString(requestData.buffer, F("GET"))) {
					requestData.method = HttpMethod::GET;
				} else if ((spaceIdx == 4) && startsWithFString(requestData.buffer, F("POST"))) {
					requestData.method = HttpMethod::POST;
				} else if ((spaceIdx == 7) && startsWithFString(requestData.buffer, F("OPTIONS"))) {
					requestData.method = HttpMethod::OPTIONS;
				} else {
					handleInvalidState(client, -20, requestData.startTime);
					return;
				}

				// Shift URL to the beginning of the buffer
				removeBufferPrefix(requestData.buffer, requestData.bufferedBytes, spaceIdx+1);
				lineLength -= spaceIdx+1;
				requestData.url = (const char*)(requestData.buffer);

				// Find space after URL with get parameters
				spaceIdx = locateCharacter(requestData.buffer, ' ', lineLength);
				if (spaceIdx < 0) {
					// Protocol error
					handleInvalidState(client, -10, requestData.startTime);
					return;
				}

				// Find question mark that delimits get parameters
				int qmPos = locateCharacter(requestData.buffer, '?', spaceIdx);

				// Ends URL string with null
				int urlLength = spaceIdx;
				requestData.buffer[spaceIdx] = 0;
				if (qmPos >= 0) {
					requestData.buffer[qmPos] = 0;
					urlLength = qmPos;
				}

				// Update request processing features for requested URL
				setFeatures(requestData.url, features);

				// Disable some features for OPTIONS request (usually a CORS preflight request)
				if (requestData.method == HttpMethod::OPTIONS) {
					features.storeGetParameters = false;
					features.storePostParameters = false;
				}

				// Disable processing of POST data for other request methods
				if (requestData.method != HttpMethod::POST) {
					features.storePostParameters = false;
				}

				// Process get parameters (if accepted and received) and store required information in the buffer
				int usedBufferBytes;
				if ((features.storeGetParameters) && (qmPos >= 0)) {
					requestData.getParameters = (char*)(requestData.buffer + (qmPos+1));
					usedBufferBytes = spaceIdx + 1;
				} else {
					usedBufferBytes = urlLength + 1;
				}

				requestData.buffer += usedBufferBytes;
				requestData.bufferSize -= usedBufferBytes;
				requestData.bufferedBytes -= usedBufferBytes;
				lineLength -= usedBufferBytes;

				// Move buffer content to the next line
				removeBufferPrefix(requestData.buffer, requestData.bufferedBytes, lineLength);
			}

			ACP_TRACE(F("HTTP: requested url %s"), requestData.url);

			// Set initial authentication
			bool authenticated = (features.authentication == NULL);

			// Process request header fields
			while (client.connected()) {
				// Check whether there is enough space to receive empty line indicating end of header fields
				if (requestData.bufferSize < 2) {
					handleInvalidState(client, -1, requestData.startTime);
					return;
				}

				// Read a single line of http request
				int lineLength = readLine(client, requestData.buffer, requestData.bufferedBytes, requestData.bufferSize, requestData.startTime);
				// Check presence of empty line indicating end of header fields
				if ((lineLength == 2) && (requestData.buffer[0] == '\r')) {
					removeBufferPrefix(requestData.buffer, requestData.bufferedBytes, lineLength);
					break;
				}

				// If there is an error other than the buffer is over-filled, handle invalid state
				if ((lineLength <= 0) && (lineLength != -1)) {
					handleInvalidState(client, lineLength, requestData.startTime);
					return;
				}

				// Check whether we have at least name of header field
				int colonPos = locateCharacter(requestData.buffer, ':', (lineLength != -1) ? lineLength : requestData.bufferSize);
				if (colonPos < 0) {
					// If the name is not fully received, we notify error (not to miss an important header field)
					handleInvalidState(client, -1, requestData.startTime);
					return;
				}

				bool importantHeaderMissed = false;

				// Check basic authentication
				if (features.authentication != NULL) {
					if ((colonPos == 13) && startsWithFString(requestData.buffer, F("Authorization:"))) {
						if (lineLength == -1) {
							importantHeaderMissed = true;
						} else {
							// Ends the line with authentication header field
							requestData.buffer[lineLength] = 0;
							// Check authentication data
							authenticated = handleBasicAuthentication(client, requestData.buffer, features.authentication);
						}
					}
				}

				// Check CORS-related headers
				if (features.enableCORS) {
					if ((colonPos == 6) && startsWithFString(requestData.buffer, F("Origin:"))) {
						if (lineLength == -1) {
							importantHeaderMissed = true;
						} else {
							// Store origin
							int skipChars = colonPos + 1;
							while (requestData.buffer[skipChars] == ' ') {
								skipChars++;
							}

							removeBufferPrefix(requestData.buffer, requestData.bufferedBytes, skipChars);
							lineLength -= skipChars;

							requestData.originHeader = (const char*)(requestData.buffer);
							while ((*(requestData.buffer) != '\r') && (*(requestData.buffer) != '\n')) {
								requestData.buffer++;
								requestData.bufferedBytes--;
								requestData.bufferSize--;
								lineLength--;
							}

							// Terminate the origin string with zero
							*(requestData.buffer) = 0;
							// Move floating buffer behind this terminating character
							requestData.buffer++;
							requestData.bufferedBytes--;
							requestData.bufferSize--;
							lineLength--;

							// Set colonPos to an invalid value
							colonPos = -1;
						}
					}
				}

				// Check post data
				if (features.storePostParameters) {
					// Store content length if provided
					if ((colonPos == 14) && (requestData.contentLength < 0) && startsWithFString(requestData.buffer, F("Content-Length:"))) {
						if (lineLength == -1) {
							importantHeaderMissed = true;
						} else {
							// Skip spaces
							const char* readPtr = (const char*)(requestData.buffer + colonPos + 1);
							while (*readPtr == ' ') {
								readPtr++;
							}

							// Decode content length
							requestData.contentLength = 0;
							while (('0' <= *readPtr) && (*readPtr <= '9')) {
								requestData.contentLength = requestData.contentLength * 10 + (*readPtr - '0');
								readPtr++;
							}
						}
					}

					// Verify content type
					if ((colonPos == 12) && startsWithFString(requestData.buffer, F("Content-Type:"))) {
						if (lineLength == -1) {
							importantHeaderMissed = true;
						} else {
							// Skip spaces
							const uint8_t* readPtr = requestData.buffer + colonPos + 1;
							while (*readPtr == ' ') {
								readPtr++;
							}

							// Verify expected content format
							if (!startsWithFString(readPtr, F("application/x-www-form-urlencoded"))) {
								features.storePostParameters = false;
							}
						}
					}
				}

				// If we missed an important header field, notify that buffer overflow
				if (importantHeaderMissed) {
					handleInvalidState(client, -1, requestData.startTime);
					return;
				}

				// Remove the line from receive buffer
				if (lineLength == -1) {
					skipToLineEnd(client, requestData.buffer, requestData.bufferedBytes, requestData.bufferSize, requestData.startTime);
				} else {
					removeBufferPrefix(requestData.buffer, requestData.bufferedBytes, lineLength);
				}
			}

			// Read post data
			if (features.storePostParameters && (requestData.contentLength > 0) && client.connected()) {
				if (requestData.contentLength + 1 <= requestData.bufferSize) {
					int dataLength = readLine(client, requestData.buffer, requestData.bufferedBytes, requestData.contentLength, requestData.startTime);
					if (dataLength == -1) {
						requestData.buffer[requestData.contentLength] = 0;
						requestData.postParameters = (char*)(requestData.buffer);
					}
				}

				if (requestData.postParameters == NULL) {
					handleInvalidState(client, -30, requestData.startTime);
					return;
				}
			}

			// Check whether the client is connected after processing the header of an http request
			if (!client.connected()) {
				handleInvalidState(client, 0, requestData.startTime);
				return;
			}

			HttpResponse response(&client, requestData.originHeader, (features.authentication != NULL));
			if (!authenticated) {
				if (requestData.method != HttpMethod::OPTIONS) {
					ACP_TRACE(F("HTTP: 401 Unauthorized"));
					response.setStatus(401, F("Unauthorized"));
				} else {
					ACP_TRACE(F("HTTP: OPTIONS"));
				}

				Print* hp = response.getHeaderPrint();
				hp->print(F("WWW-Authenticate: Basic realm=\""));

				// Print realm
				PGM_P authPtr = reinterpret_cast<PGM_P>(features.authentication);
				bool colonFound = false;
				char c;
				while ((c = pgm_read_byte(authPtr)) != 0) {
					if (colonFound) {
						hp->print(c);
					}

					if (c == ':') {
						colonFound = true;
					}

					authPtr++;
				}

				// If no realm is defined in setting, use realm "web"
				if (!colonFound) {
					hp->print(F("web"));
				}

				hp->println('\"');
				response.startContent();
			} else {
				if (requestData.method != HttpMethod::OPTIONS) {
					// Construct bundle with request data
					HttpRequest request(requestData);

					// Invoke request processing method
					processRequest(request, response);
				}

				// Ensure that headers are sent
				response.startContent();
			}

			// Close the connection:
			closeConnection(client);

			ACP_TRACE(F("HTTP: client processed."));
		}

	protected:
		//--------------------------------------------------------------------------------
		// Sets features to be applied to process an http request.
		virtual void setFeatures(const char* url, Features& features) {
			if (setFeaturesEvent != NULL) {
				setFeaturesEvent(url, features);
			}
		}

		//--------------------------------------------------------------------------------
		// Processes an http request
		virtual void processRequest(HttpRequest& request, HttpResponse& response) {
			if (processRequestEvent != NULL) {
				processRequestEvent(request, response);
			}
		}

	public:
		// Event handler that sets features of a request.
		void (*setFeaturesEvent)(const char* url, Features& features);

		// Event handler that processes the request
		void (*processRequestEvent)(HttpRequest& request, HttpResponse& response);

		//--------------------------------------------------------------------------------
		// Constructs the client handler.
		inline SimpleHttpHandlingController() {
			server = NULL;
			state = 0;
			setFeaturesEvent = NULL;
			processRequestEvent = NULL;
		}

		//--------------------------------------------------------------------------------
		// Initializes the client handler.
		void init(const __FlashStringHelper* authentication, bool enableCORS) {
			// Set default authentication
			this->authentication = authentication;
			if (this->authentication != NULL) {
				if (pgm_read_byte(reinterpret_cast<PGM_P>(this->authentication)) == 0) {
					this->authentication = NULL;
				}
			}

			// Set default CORS support
			if (enableCORS) {
				state |= B00000010;
			} else {
				state &= B11111101;
			}
		}

		//--------------------------------------------------------------------------------
		// Looper for handling the associated server.
		inline void serverLooper() {
			if (server != NULL) {
				if ((state & B00000001) == 0) {
					server->init();
					state |= B00000001;
				}

				server->loop();
			}
		}
	};

	/********************************************************************************
	 * View for a simple http client handler.
	 ********************************************************************************/
	template<int BUFFER_SIZE, long TIMEOUT> class TSimpleHttpClientHandler: public acp_network_libs_handling_srv::ClientHandlerWithServerSupport {
		friend class acp_network_libs_handling_srv::ClientHandlerWithServerSupport;
	private:
		// The handling controller.
		SimpleHttpHandlingController<BUFFER_SIZE, TIMEOUT> &controller;

	public:
		//--------------------------------------------------------------------------------
		// Constructs view for the client handler.
		TSimpleHttpClientHandler(SimpleHttpHandlingController<BUFFER_SIZE, TIMEOUT> &controller):controller(controller) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Handles the client. The client is closed immediately after it has been handled.
		void handle(Client& client) {
			controller.handle(client);
		}

		//--------------------------------------------------------------------------------
		// Sets the server that exclusively uses this client handler.
		virtual void setServer(acp_network_libs_handling_srv::LoopingServer* server) {
			// Check whether it is a new server
			if (controller.server == server) {
				return;
			}

			// Finalize previously associated server
			if (controller.server != NULL) {
				controller.server->finalize();
				controller.server = NULL;
			}

			// Associate a new server
			controller.server = server;

			// Indicate that the server is not initialized
			controller.state &= B11111110;
		}

		//--------------------------------------------------------------------------------
		// Handles a client - only new clients are accepted.
		virtual void handle(Client& client, bool isNew) {
			if (!isNew) {
				controller.closeConnection(client);
				return;
			}

			controller.handle(client);
		}
	};
}

#endif /* MODULES_ACP_NETWORK_SIMPLE_HTTP_CLIENT_HANDLER_INCLUDE_SIMPLEHTTPCLIENTHANDLER_H_ */
