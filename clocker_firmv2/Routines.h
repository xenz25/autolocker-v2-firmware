
//should be imported after all dependencies

RoutinePayload START_WIFI_CONFIG_ROUTINE() {
  RoutinePayload payload;
  // ------ INIT LCD ------ //
  initLCD();
  horAPrint("Automated", 0);
  horAPrint("Cell Locker", 1);
  delay(PROMPT_INTERVAL);
  // ------ INIT LCD ------ //

  // ------ INIT SD CARD ------ //
  if (initSD()) {
    console_notice("SD CARD FOUND");
    printNorm("Connecting...", 0, 0, true);
    lcdMakeBlink();

    // --- READ CREDENTIALS --- //
    WFCredentials cred = getCredentials();
    bool hasWifiCred = hasCredentials(cred);
    if (hasWifiCred) {
      console_notice("--- CREDENTIALS AVAILABLE ---");
      console_notice(cred.ssid.c_str());
      console_notice(cred.pass.c_str());
      bool isConnected = CONNECTION_ROUTINE(cred);
      payload = { isConnected, (isConnected ? SUCC_CONNECTED : ERR_CANNOT_CONNECT)};

    } else {
      console_error("--- NO AVAILABLE CREDENTIALS ---");
      // start AP Mode
      printNorm("No WiFi", 0, 0, true);
      printNorm("Data Found...", 0, 1, false);
      lcdMakeBlink();
      delay(PROMPT_INTERVAL);
      AP_MODE_ROUTINE();
    }
  } else {
    // --> SD CARD FAILS TO INITIALIZE OR CANNOT BE FOUND
    console_error(ERR_INACCESSIBLE_SD);
    printErr(ERR_INACCESSIBLE_SD);
    payload = { false, ERR_INACCESSIBLE_SD};
  }
  return payload;
}

RoutinePayload GATHER_SOCKET_REQUIREMENTS() {
  RoutinePayload payload;
  if (WiFi.status() == WL_CONNECTED) {
    printNorm("Connecting", 0, 0, true);
    printNorm("to server...", 0, 1, false);
    lcdMakeBlink();
    delay(PROMPT_INTERVAL);

    int connectAtempt = 0;
    bool wasAdded = false;
    while (!wasAdded) {
      printNorm("Connecting", 0, 0, true);
      printNorm("to server...", 0, 1, false);
      wasAdded = checkRequirements();
      if (connectAtempt++ > 5 || wasAdded) break; // 5 attempts at 5 sec interval
      delay(5000);
      printNorm("Retrying...", 0, 0, true);
      delay(PROMPT_INTERVAL);
    }
    if (!wasAdded) {
      printNorm("Locker is not", 0, 0, true);
      printNorm("Added...", 0, 1, false);
    }
    payload = { wasAdded, wasAdded ? SUCC_VALIDATED : ERR_NVALIDATED};
    return payload;
  } else {
    printNorm("No Network", 0, 0, true);
    printNorm("Connection...", 0, 1, false);
    payload = { false, ERR_CANNOT_CONNECT};
  }
  return payload;
}

RoutinePayload ESTABLISH_WEB_SOCKET_CONNECTION(){
  
}
