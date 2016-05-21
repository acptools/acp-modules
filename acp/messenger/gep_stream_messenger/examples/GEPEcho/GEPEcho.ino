//----------------------------------------------------------------------
// Includes required to build the sketch (including ext. dependencies)
#include <GEPEcho.h>
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Summary of available objects:
// messenger (acp.messenger.gep_stream_messenger)
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Event callback for Program.OnStart
void onStart() {
  Serial.begin(9600);
  messenger.setStream(Serial);
}

//----------------------------------------------------------------------
// Event callback for messenger.OnMessageReceived
void onMessageReceived(const char* message, int messageLength, long messageTag) {
  char reply[30];
  
  // Consider at most 15 byte from beginning of the received message
  if (messageLength > 15) {
    messageLength = 15;
  }
  
  // Duplicate chars (bytes)
  for (int i=0; i<messageLength; i++) {
    reply[2*i] = message[i];
    reply[2*i+1] = message[i];
  }
   
  // Reply with a broadcasted message (destinationId is set to 0) 
  if (messageTag >= 0) { 
    messenger.sendMessage(0, reply, messageLength*2, messageTag);
  } else {
    messenger.sendMessage(0, reply, messageLength*2);  
  }
}


