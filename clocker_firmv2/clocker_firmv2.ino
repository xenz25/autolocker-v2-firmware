
uint8_t GLOBAL_STATE; // no value by default

void startNormalTask();
void restartMachine(unsigned long);

#define _TASK_TIMEOUT
#define _TASK_PRIORITY
#include <TaskScheduler.h>

#include "StringSplitter.h"
#include "Logger.h"
#include "Structs.h"

#include "CONSTANTS.h"
#include "LcdMgr.h"
#include "SDCardMgr.h"
#include "WifiMgr.h"
#include "HTTPReqMgr.h"

// task
Scheduler sc;
Scheduler hpr;

void startScanRoutine();
void afterScanning();
void startVerifyCode();
void afterVerification();
bool beforeAwaitDocumentReceive();
void awaitDocumentReceive();
void afterAwaitDocumentReceive();
bool beforeAwaitDoorToClose();
void awaitDoorToClose();
void afterAwaitDoorToClose();
void awaitDoorToOpen();
void afterAwaitDoorToOpen();
bool beforeNotifyCellIsFree();
void notifyCellIsFree();
void afterNotifyCellIsFree();

// higher priority callbacks
void checkIRState();
void socketConnection();
void checkDoorStates();
void awaitAssignToServer();
void afterAssignToServer();
void sendDoorStateToServer();
void showGrabYourDocuments();
void afterTaskShowGrabDocs();
void checkWifiAndReconnect();

// found in scan event manager
bool enableScanningEvent();
void disableScanningEvent();

// HIGHER PRIORITY TASKS
Task tSendDoorStateToServer(2 * TASK_MILLISECOND, TASK_FOREVER, &sendDoorStateToServer, &hpr, true);
Task tCheckIRState(5 * TASK_MILLISECOND, TASK_FOREVER, &checkIRState, &hpr, true);
Task tSocketConnection(5 * TASK_MILLISECOND, TASK_FOREVER, &socketConnection, &hpr, true);
Task tCheckDoorStates(5 * TASK_MILLISECOND, TASK_FOREVER, &checkDoorStates, &hpr, true);
Task tCheckWifiState(3 * TASK_SECOND, TASK_FOREVER, &checkWifiAndReconnect, &hpr, true);

// add TASK HERE THAT CHECK DOOR STATE AND UPDATE SERVER
// LEAST PRIORITY TASKS
Task tAwaitAssignToServer(5 * TASK_SECOND, TASK_FOREVER, &awaitAssignToServer, &sc, false, NULL, &afterAssignToServer);
Task tDisplayWaitingCode(TASK_IMMEDIATE, TASK_FOREVER, NULL, &sc, false, &enableScanningEvent, &disableScanningEvent);
Task tScanRoutine(TASK_IMMEDIATE, TASK_FOREVER, &startScanRoutine, &sc, false, NULL, &afterScanning);
Task tVerifyCode(500 * TASK_MILLISECOND, TASK_FOREVER, &startVerifyCode, &sc, false, NULL, &afterVerification);
Task tAwaitDocumentReceive(1000 * TASK_MILLISECOND, TASK_FOREVER, &awaitDocumentReceive, &sc, false, &beforeAwaitDocumentReceive, &afterAwaitDocumentReceive);
Task tAwaitDoorClosing(TASK_IMMEDIATE, TASK_FOREVER, &awaitDoorToClose, &sc, false, &beforeAwaitDoorToClose, &afterAwaitDoorToClose);
Task tAwaitDoorOpening(TASK_IMMEDIATE, TASK_FOREVER, &awaitDoorToOpen, &sc, false, NULL, &afterAwaitDoorToOpen);
Task tNoifyCellIsFree(200 * TASK_MILLISECOND, TASK_FOREVER, &notifyCellIsFree, &sc, false, &beforeNotifyCellIsFree, &afterNotifyCellIsFree);
Task tPrintGrabDocuments(TASK_IMMEDIATE, TASK_FOREVER, &showGrabYourDocuments, &sc, false, NULL, &afterTaskShowGrabDocs);
Task tPromptPrinter(TASK_IMMEDIATE, TASK_FOREVER, NULL, &sc, false, NULL, NULL);

#include "WebSocketHandler.h"

// ROUTINES
#include "Routines.h"

// placed under task initialization
#include "ScanEvtMgr.h"
#include "LockMgr.h"
#include "TaskCallbacks.h"

// FLAGS
bool isDeviceReady = false;

void setup() {
  Serial.begin(115200);
  console_notice("-- DEBUG LOG ACTIVE --");
  //  initLCD();

  RoutinePayload rt1 = START_WIFI_CONFIG_ROUTINE();
  console_notice(rt1.responseText);

  if (!rt1.validity) {
    // failed
    delay(PROMPT_INTERVAL);
    console_error(rt1.responseText);
    //    printErr(rt1.responseText);
    return;
  }

  if (!wasAPStarted) { // prevent initialization since no wifi available
    RoutinePayload rt2 = GATHER_SOCKET_REQUIREMENTS();
    console_notice(rt2.responseText);

    if (!rt2.validity) {
      delay(PROMPT_INTERVAL);
      console_error(rt2.responseText);
      //    printErr(rt2.responseText);
      return;
    }

    delay(PROMPT_INTERVAL);
    printNorm("Preparing...", 0, 0, true);

    // WEB SOCKET CONNECTION
    initWebSocketConnection();

    // set high priority task scheduler
    sc.setHighPriorityScheduler(&hpr);
  }
}

void loop() {
  sc.execute();
}

// perform necesserayy task based on last operation logs
void afterAssignToServer() {
  if (!WAS_ALREADY_INITIALIZED) {
    // SO WE SONT INITIALIZE THINGS AGAIN AFTER RECONNECTION
    WAS_ALREADY_INITIALIZED = true;

    // initilalize periherals and devices
    initScanner(); // start scanning peripherals
    initLocks(); // check locks and initialize

    OperationLogs opLogs = getOpLogs();

    if (opLogs.GLOBAL_STATE != "0" && opLogs.GLOBAL_STATE != "1") {
      disableScanningEvent(); // we wont trigger scan events since there are pending task
      CHANGE_ACTION_FLAG = CHANGE_ACTION_FLAG_TYPE.fromScanner;
      RX_DATA = getSavedRxData(); // use for task awaiting document receive

      Serial.println(opLogs.GLOBAL_STATE);
      Serial.println(opLogs.VALUE);
      // there are pending operations before
      if (opLogs.GLOBAL_STATE == "2") {
        if (opLogs.VALUE != SOCKET_RESPONSE_PAYLOADS.fail) {
          ACTIVE_CELL_INDEX = getActiveCellIndex();
          uint8_t cellState = btnHandler.getBitValue(btnHandler.getNewVal(), ACTIVE_CELL_INDEX);
          if (cellState == 1) {
            // door is close
            tAwaitDocumentReceive.setTimeout(RECEIVING_DOCUMENT_TIMEOUT * TASK_SECOND);
            tAwaitDocumentReceive.restart();
          } else {
            // door is open
            tPrintGrabDocuments.setTimeout(4 * TASK_SECOND); // so we dont block updation task
            tPrintGrabDocuments.restart();
          }
        } else {
          startNormalTask();
        }
      } else if (opLogs.GLOBAL_STATE == "3") {
        ACTIVE_CELL_INDEX = getActiveCellIndex();
        if (opLogs.VALUE == SOCKET_RESPONSE_PAYLOADS.ok) {
          tPrintGrabDocuments.setTimeout(4 * TASK_SECOND); // so we dont block updation task
          tPrintGrabDocuments.restart();
        } else {
          startNormalTask(); // just start another scanning because door is close
          //          tAwaitDocumentReceive.setTimeout(RECEIVING_DOCUMENT_TIMEOUT * TASK_SECOND);
          //          tAwaitDocumentReceive.restart();
        }
      } else if (opLogs.GLOBAL_STATE == "4") {
        ACTIVE_CELL_INDEX = getActiveCellIndex();
        uint8_t cellState = btnHandler.getBitValue(btnHandler.getNewVal(), ACTIVE_CELL_INDEX);
        if (cellState == 1) {
          // door is close
          tNoifyCellIsFree.restart();
        } else {
          // door is open
          tAwaitDoorClosing.restart();
        }
      } else if (opLogs.GLOBAL_STATE == "5") {
        ACTIVE_CELL_INDEX = getActiveCellIndex();
        if (opLogs.VALUE == SOCKET_RESPONSE_PAYLOADS.ok) {
          startNormalTask();
        } else {
          tNoifyCellIsFree.restart();
        }
      }
    } else {
      startNormalTask();
    }
  }
}

void startNormalTask() {
  Serial.println("NO PENDING OPERATIONS WORKING TREE IS CLEAN!");
  GLOBAL_STATE = 0;
  printNorm("Locker is", 0, 0, true);
  printNorm("ready...", 0, 1, false);
  delay(PROMPT_INTERVAL);
  tDisplayWaitingCode.restart();
}

// placed under high priority scheduler
void checkIRState() {
  irBtn.check();
}

void socketConnection() {
  //handle wifi reconnection
  io.loop();
}

void checkDoorStates() {
  btnHandler.readNow();
}

// watches the door state changes and notify the server if it is a compromised event
// or a safe event
void sendDoorStateToServer() {
  // update server whenever state changes
  if (detectorValue != btnHandler.getNewVal()) {
    // to verify compromised event
    //    /CHANGE_ACTION_FLAG != CHANGE_ACTION_FLAG_TYPE.defaultValue
    if (CHANGE_ACTION_FLAG == CHANGE_ACTION_FLAG_TYPE.defaultValue) { // check if update or compromised
      // device was compromised put to deep sleep else
      Serial.println("FORCING DETECTED");
      Dictionary my_payloads[] = {
        {"name", LOCKER_NAME},
        {"auth", LOCKER_AUTH_KEY}
      };
      String event = eventBuilder(SOCKET_EVENT_TYPES.error, my_payloads, 2);
      bool wasSent = io.sendEVENT(event); // returns a boolean if true it means code was sent to socket server
      printWasOpenedByForce();
      sc.disableAll(true); // stop process execution
      esp_deep_sleep_start();
    } else {
      // device was updated via qr code scanner
      Serial.println("CHANGED");
      Dictionary my_payloads[] = {
        {"type", SOCKET_MSG_TYPES.update},
        {"data", String(btnHandler.getNewVal())},
        {"cellCount", String(MAX_CELL_COUNT)}
      };
      String event = eventBuilder(SOCKET_EVENT_TYPES.message, my_payloads, 3);
      bool wasSent = io.sendEVENT(event); // returns a boolean if true it means code was sent to socket server
      if (wasSent) {
        //update detector value
        detectorValue = btnHandler.getNewVal();
      }
    }
  }
}

//CHECK WIFI AND IF DISCONNECTED WILL INITIATE RECONNECT
void checkWifiAndReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.reconnect();
    tCheckWifiState.setInterval(10 * TASK_SECOND);
  }
}

void restartMachine(unsigned long after) {
  delay(after);
  ESP.restart();
}
