
void restartCallBack() {
  tDisplayWaitingCode.restart();
}

void startScanRoutine() {
  if (tScanRoutine.isFirstIteration()) {
    // you can disable other non important task here
    printReadingYourCode();
  }
  //  Serial.println("TASK SCANNING IS RUNNING");
  while (QRSerial.available()) {
    RX_DATA = QRSerial.readStringUntil('\n');
    RX_DATA = RX_DATA.substring(0, RX_DATA.length() - 1);
    Serial.println(RX_DATA);
    tScanRoutine.disable(); //disabled task after successfull reading
  }
}

void afterScanning() {
  //  Serial.println("AFTER SCANNING");
  if (tScanRoutine.timedOut()) {
    printNorm("Failed to catch", 0, 0, true);
    printNorm("your code...", 0, 1, false);
    delay(PROMPT_INTERVAL);
    tDisplayWaitingCode.restartDelayed(2 * TASK_SECOND);
  } else {
    // flag that the data is for evaluation
    isForEvaluation = true;
    GLOBAL_STATE = 1;
    OperationLogs oplogs = {String(GLOBAL_STATE), "null"};
    saveOperationLogs(oplogs);
    // enable evaluation task
    tVerifyCode.restart();
  }
}

void startVerifyCode() {
  if (tVerifyCode.isFirstIteration()) {
    printVerifyingYourCode();
  }
  //  Serial.println("TASK VERIFICATION STARTED");
  // send a request to web server for code verification
  if (isSocketActive) {
    // send a message event with verify message type
    Dictionary my_payloads[] = {
      {"type", SOCKET_MSG_TYPES.verify},
      {"data", RX_DATA}
    };
    String event = eventBuilder(SOCKET_EVENT_TYPES.message, my_payloads, 2);
    io.sendEVENT(event); // returns a boolean if true it means code was sent to socket server
  }
}

void afterVerification() {
  // run after verification
}

bool beforeAwaitDocumentReceive() {
  // update global state
  GLOBAL_STATE = 3;
  OperationLogs oplogs = {String(GLOBAL_STATE), SOCKET_RESPONSE_PAYLOADS.fail};
  saveOperationLogs(oplogs);
  // ON OPEN START TASK AWAITING APPROVAL - this task check if the
  // requester changes the state of the request to received
  if (LAST_PRINTED_CODE != 5) {
    // prevents refreshing screen
    printPleaseUpdateToReceive();
  }
  return true;
}

void awaitDocumentReceive() {
  //  Serial.println("SENDING EVENT");
  // ping the server if the document was receive
  if (isSocketActive) {
    // send a message event with verify message type
    if (!isDocumentReceived) {
      Dictionary my_payloads[] = {
        {"type", SOCKET_MSG_TYPES.checkDocStatus},
        {"data", RX_DATA} // send this data because we are sure that this is verified
      };
      String event = eventBuilder(SOCKET_EVENT_TYPES.message, my_payloads, 2);
      io.sendEVENT(event);
      signed int remaining = (tAwaitDocumentReceive.untilTimeout() / 1000);
      if (remaining > -1) {
        if (remaining <= 9) {
          printBlank(14, 1);
        }
        printNormStr(String(remaining), 13, 1, false);
      } else {
        tAwaitDocumentReceive.disable();
        isNotAlreadyTimeOut = false;
      }
    }
  }
}

void afterAwaitDocumentReceive() {
  //  tAwaitDoorOpening.restart();
  if (tAwaitDocumentReceive.timedOut()) {
    // RESTART
    Serial.println("RECEIVE TIMED OUT");
    isNotAlreadyTimeOut = false;
    GLOBAL_STATE = 0;
    OperationLogs oplogs = {String(GLOBAL_STATE), "null"};
    saveOperationLogs(oplogs);
    tPromptPrinter.setOnEnable(&printThatDidNotWork);
    tPromptPrinter.setOnDisable(&restartCallBack);
    tPromptPrinter.setTimeout(2 * TASK_SECOND);
    tPromptPrinter.restart();
  }
}

void awaitDoorToOpen() {
  if (tAwaitDoorOpening.isFirstIteration()) {
    printOpeningCell();
    Serial.print("OPENING CELL: ");
    Serial.println((ACTIVE_CELL_INDEX + 1));

    // set change flag to from scanner so the listener knows we are changing door states from scanner
    CHANGE_ACTION_FLAG = CHANGE_ACTION_FLAG_TYPE.fromScanner;
  }
  // WAIT FOR THE ACTIVE CELL TO OPEN
  // second argument is 1 since we are sure that the cell is awaiting oeen state
  bool actionState = lockStateManager((ACTIVE_CELL_INDEX + 1), 1);
  if (actionState && btnHandler.getBitValue(btnHandler.getNewVal(), ACTIVE_CELL_INDEX) == 0) {
    OperationLogs oplogs = {String(GLOBAL_STATE), SOCKET_RESPONSE_PAYLOADS.ok};
    saveOperationLogs(oplogs);
    tAwaitDoorOpening.delay(2 * TASK_SECOND);
    btnHandler.updateOldVal(); // call update to catch new state
    // stop the task
    Serial.print(ACTIVE_CELL_INDEX);
    Serial.println(" WAS OPENED");
    tAwaitDoorOpening.disable();
  }
}

void afterAwaitDoorToOpen() {
  tPrintGrabDocuments.setTimeout(4 * TASK_SECOND); // so we dont block updation task
  tPrintGrabDocuments.restart();
}

bool beforeAwaitDoorToClose() {
  GLOBAL_STATE = 4; // door closing state
  OperationLogs oplogs = {String(GLOBAL_STATE), String(ACTIVE_CELL_INDEX)};
  saveOperationLogs(oplogs);
  printPleaseCloseTheDoor();
  return true;
}

void awaitDoorToClose() {
  // WAIT FOR THE ACTIVE CELL TO CLOSE
  // second argument is zero since we are sure that the cell is awaiting lock state
  bool actionState = lockStateManager((ACTIVE_CELL_INDEX + 1), btnHandler.evaluateBits(ACTIVE_CELL_INDEX));
  if (actionState && btnHandler.getBitValue(btnHandler.getNewVal(), ACTIVE_CELL_INDEX) == 1) {
    btnHandler.updateOldVal(); // call update to catch new state
    // stop the tasl
    Serial.print(ACTIVE_CELL_INDEX);
    Serial.println(" WAS CLOSED");
    tAwaitDoorClosing.disable();
  }
}

void afterAwaitDoorToClose() {
  // should notify server that the cell is now free
  Serial.println("STARTING NORIFYNG TASK");
  tNoifyCellIsFree.restartDelayed(2 * TASK_SECOND);
}

bool beforeNotifyCellIsFree() {
  GLOBAL_STATE = 5;
  OperationLogs oplogs = {String(GLOBAL_STATE), SOCKET_RESPONSE_PAYLOADS.fail };
  saveOperationLogs(oplogs);
  printPleaseWait();
  return true;
}

void notifyCellIsFree() {
  // NOTIFY THE SERVER
  if (!isServerNotifiedCellIsFree) {
    Dictionary my_payloads[] = {
      {"type", SOCKET_MSG_TYPES.updateToFree},
      {"name", LOCKER_NAME},
      {"auth", LOCKER_AUTH_KEY},
      {"cellNum", String(ACTIVE_CELL_INDEX + 1)}
    };
    String event = eventBuilder(SOCKET_EVENT_TYPES.message, my_payloads, 4);
    io.sendEVENT(event);
  }
}

void afterNotifyCellIsFree() {
  printThankYou();
  delay(3000);
  // initiate another scan routine acceptance
  tDisplayWaitingCode.restartDelayed(3 * TASK_SECOND); // 3 second waiting for new scan events
}

void awaitAssignToServer() {
  Dictionary my_payloads[] = {
    {"name", LOCKER_NAME},
    {"auth", LOCKER_AUTH_KEY}
  };
  String event = eventBuilder(SOCKET_EVENT_TYPES.assign, my_payloads, 2);
  io.sendEVENT(event);
}


void showGrabYourDocuments() {
  if (tPrintGrabDocuments.isFirstIteration()) {
    isDocumentReceived = true; // true for now
    printGrabYourDocuments();
  }
}

void afterTaskShowGrabDocs() {
  if (tPrintGrabDocuments.timedOut()) {
    tAwaitDoorClosing.restart();
  }
}

bool compare(const char * val1, const char * val2) {
  return strcmp(val1, val2) == 0;
}

// ahdnler for socket response need to be here since it has to disable some task
void handleSIOResponse(uint8_t * response_payload) {
  SocketResponse sioResponse = serializeResponse(response_payload);

  Serial.println(sioResponse.RESPONSE_TYPE);
  Serial.println(sioResponse.VALUE);

  // assign events
  if (compare(sioResponse.RESPONSE_TYPE, SOCKET_RESPONSE_TYPES.assign)) {
    Serial.println(sioResponse.VALUE);
    if (compare(sioResponse.VALUE, "OK")) {
      Serial.println("OK");
      isAssignEventAcknowledge = true;
      tAwaitAssignToServer.disable();
    }
  }

  // ping events is high priority and wont block other events
  if (compare(sioResponse.RESPONSE_TYPE, SOCKET_RESPONSE_TYPES.ping)) {
    Dictionary my_payloads[] = {
      {"data", sioResponse.VALUE}
    };
    String event = eventBuilder(SOCKET_EVENT_TYPES.pong, my_payloads, 1);
    io.sendEVENT(event);
  }

  if (compare(sioResponse.RESPONSE_TYPE, SOCKET_RESPONSE_TYPES.verify)) {
    // disable verification tas
    tVerifyCode.disable();// stop verification task
    if (isForEvaluation) { // ignore other send responses
      isForEvaluation = false;
      GLOBAL_STATE = 2;

      if (compare(sioResponse.VALUE, SOCKET_RESPONSE_PAYLOADS.fail)) {
        Serial.println("UNKNOWN CODE");
        tPromptPrinter.setOnEnable(&printUnknownCode);
        tPromptPrinter.setOnDisable(&restartCallBack);
        tPromptPrinter.setTimeout(2 * TASK_SECOND);
        OperationLogs oplogs = {String(GLOBAL_STATE), sioResponse.VALUE};
        saveOperationLogs(oplogs);
        tPromptPrinter.restart();
      } else {
        // WILL UPDATE ACTIVE CELL INDEX VALUE
        updateActiveCellIndex(sioResponse.VALUE);
        // OPEN SPECIFIED CELL
        tAwaitDocumentReceive.setTimeout(RECEIVING_DOCUMENT_TIMEOUT * TASK_SECOND);
        tAwaitDocumentReceive.restart();
        OperationLogs oplogs = {String(GLOBAL_STATE), sioResponse.VALUE};
        saveOperationLogs(oplogs);
        saveRxData(); // log verified rx data code
        saveActiveCellIndex();
      }
    }
  } else if (compare(sioResponse.RESPONSE_TYPE, SOCKET_RESPONSE_TYPES.checkDocStatus)) {
    if (compare(sioResponse.VALUE, SOCKET_RESPONSE_PAYLOADS.ok)) {
      // document was received
      if (!isDocumentReceived) {
        // await door to open
        tAwaitDocumentReceive.disable();
        tAwaitDoorOpening.restart();
      }
    } else {
      if (isNotAlreadyTimeOut) {
        // restart cheking document if received
        tAwaitDocumentReceive.setTimeout(RECEIVING_DOCUMENT_TIMEOUT * TASK_SECOND);
        tAwaitDocumentReceive.enableIfNot();
      } else {
        Serial.println("EVENT TIMED OUT -- UPDATED");
        tAwaitDocumentReceive.disable();
      }
    }
  } else if (compare(sioResponse.RESPONSE_TYPE, SOCKET_RESPONSE_TYPES.updateToFree)) {
    // response to cell is free updation
    if (compare(sioResponse.VALUE, SOCKET_RESPONSE_PAYLOADS.ok)) {
      isServerNotifiedCellIsFree = true;
      tNoifyCellIsFree.disable();
      OperationLogs oplogs = {String(GLOBAL_STATE), SOCKET_RESPONSE_PAYLOADS.ok };
      saveOperationLogs(oplogs);
    }
  }
}
