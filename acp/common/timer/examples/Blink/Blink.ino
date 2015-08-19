//----------------------------------------------------------------------
// Includes required to build the sketch (including ext. dependencies)
#include <Blink.h>
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Summary of available objects:
// blinkTimer (acp.common.timer)
// led (acp.common.switch)
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Event callback for blinkTimer.OnTick
void onBlink() {
  led.revert();
}