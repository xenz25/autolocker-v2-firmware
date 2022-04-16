#include <AceButton.h>
using namespace ace_button;
#include <HardwareSerial.h>

HardwareSerial QRSerial(2);

AceButton irBtn;

bool canStartQRScanning = true;
bool isForEvaluation = false;
bool isCodeFailedCatch = false;

void handleEvent(AceButton*, uint8_t, uint8_t);

void initScanner() {
  QRSerial.begin(115200);
  irBtn.init(IR_OUT);
  pinMode(SC_SIG, OUTPUT);
  digitalWrite(SC_SIG, HIGH);

  pinMode(IR_OUT, INPUT_PULLUP);

  ButtonConfig* irConfig = irBtn.getButtonConfig();
  irConfig->setEventHandler(handleEvent);
  irConfig->setFeature(ButtonConfig::kFeatureSuppressAfterClick);

  // Check if the button was pressed while booting
  if (irBtn.isPressedRaw()) {
    Serial.println(F("setup(): button was pressed while booting"));
  }

  Serial.println(F("setup(): ready"));
}

bool enableScanningEvent() {
  GLOBAL_STATE = 0;
  ACTIVE_CELL_INDEX = NON_SELECTABLE_ADDRESS;
  // set back to defaultValue - so we the listener knows we are done
  CHANGE_ACTION_FLAG = CHANGE_ACTION_FLAG_TYPE.defaultValue;
  printWaitingResponse();
  canStartQRScanning = true;
  isForEvaluation = false;
  isCodeFailedCatch = false;
  // foudn on websocket handler
  isDocumentReceived = false;
  isServerNotifiedCellIsFree = false;
  return true;
}

void disableScanningEvent() {
  canStartQRScanning = false;
  isForEvaluation = true;
}

// The event handler for the button.
void handleEvent(AceButton* irBtn, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("pressed");
      if (canStartQRScanning && !isForEvaluation) {
        tDisplayWaitingCode.disable();

        /// send a pulse to start scanner
        digitalWrite(SC_SIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(SC_SIG, LOW);

        tScanRoutine.setTimeout(6 * TASK_SECOND);
        tScanRoutine.restart();
      }
      break;
    case AceButton::kEventReleased:
      Serial.println("released");
      break;
  }
}
