//----------------------------------------------------------------------
// Includes required to build the sketch (including ext. dependencies)
#include <RegAccess.h>
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Summary of available objects:
// messenger (acp.messenger.gep_stream_messenger)
// registryProtocol (acp.messenger.registry_access_protocol)
//----------------------------------------------------------------------

// Size of register
#define REGISTER_SIZE 22
// Registers 0-22
long registerValues[REGISTER_SIZE];
// Special register with id 1000
long specialRegister = 0;

// Sample binary register
#define BINARY_REGISTER_ID 50
#define BINARY_REGISTER_SIZE 20
char binaryRegister[BINARY_REGISTER_SIZE] = "Binary register";

//----------------------------------------------------------------------
// Event callback for Program.OnStart
void onStart() {
  Serial.begin(9600);
  messenger.setStream(Serial); 
  
  // Initialize register values
  registerValues[0] = 1;
  for (int i=1; i<10; i++) {
    registerValues[i] = registerValues[i-1]*10;
  }
  
  registerValues[10] = -1;
  for (int i=11; i<20; i++) {
    registerValues[i] = registerValues[i-1]*10;
  }
  
  registerValues[20] = LONG_MAX;
  registerValues[21] = LONG_MIN;
}

//----------------------------------------------------------------------
// Event callback for messenger.OnMessageReceived
void onMessageReceived(const char* message, int messageLength, long messageTag) {
  char response[100];
  int responseSize = 100;
  registryProtocol.handleRequest(message, messageLength, response, responseSize);
  if (responseSize == 0) {
    return;
  }

  if (messageTag >= 0) {
    messenger.sendMessage(0, response, responseSize, messageTag); 
  } else {
    messenger.sendMessage(0, response, responseSize); 
  }
}

//----------------------------------------------------------------------
// Event callback for registryProtocol.OnWriteIntRegister
bool onWriteIntRegister(unsigned int registerId, long value) {
  if ((registerId >= 0) && (registerId < REGISTER_SIZE)) {
    registerValues[registerId] = value;
    registryProtocol.markModifiedRegister(registerId);
    return true;
  }
  
  if (registerId == 1000) {
    specialRegister = value;
    return true;
  }

  return false;  
}

//----------------------------------------------------------------------
// Event callback for registryProtocol.OnReadIntRegister
long onReadIntRegister(unsigned int registerId) {
   if ((registerId >= 0) && (registerId < REGISTER_SIZE)) {
      return registerValues[registerId];
   }
   
   if (registerId == 1000) {
     return specialRegister;
   }
   
   return 0;
}

//----------------------------------------------------------------------
// Event callback for registryProtocol.OnWriteBinRegister
bool onWriteBinRegister(unsigned int registerId, const char* data, int dataLength) {
  if ((registerId == BINARY_REGISTER_ID) && (dataLength < BINARY_REGISTER_SIZE)) {
    memcpy(binaryRegister, data, dataLength);
    binaryRegister[dataLength] = 0;
    return true;
  }
	
  return false;	
}

//----------------------------------------------------------------------
// Event callback for registryProtocol.OnReadBinRegister
int onReadBinRegister(unsigned int registerId, char* dstBuffer, int dstBufferSize) {
  if ((registerId == BINARY_REGISTER_ID) && (BINARY_REGISTER_SIZE <= dstBufferSize))  {
    memcpy(dstBuffer, binaryRegister, BINARY_REGISTER_SIZE);
    return BINARY_REGISTER_SIZE;
  }
	
  return -1; 		
}
