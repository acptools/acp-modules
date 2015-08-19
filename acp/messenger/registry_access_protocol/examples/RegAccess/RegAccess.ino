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

//----------------------------------------------------------------------
// Event callback for Program.OnStart
void onStart() {
  Serial.begin(33600);
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
  char response[10];
  int responseSize = 10;
  registryProtocol.handleRequest(message, messageLength, response, responseSize);
  if (responseSize == 0) {
    return;
  }

  if (messageTag >= 0) {
    messenger.sendMessage(response, responseSize, messageTag); 
  } else {
    messenger.sendMessage(response, responseSize); 
  }
}

//----------------------------------------------------------------------
// Event callback for registryProtocol.OnWriteRegister
bool onWriteRegister(unsigned int registerId, long value) {
  if ((registerId >= 0) && (registerId < REGISTER_SIZE)) {
    registerValues[registerId] = value;
    return true;
  }
  
  if (registerId == 1000) {
    specialRegister = value;
    return true;
  }

  return false;  
}

//----------------------------------------------------------------------
// Event callback for registryProtocol.OnReadRegister
long onReadRegister(unsigned int registerId) {
   if ((registerId >= 0) && (registerId < REGISTER_SIZE)) {
      return registerValues[registerId];
   }
   
   if (registerId == 1000) {
     return specialRegister;
   }
   
   return 0;
}
