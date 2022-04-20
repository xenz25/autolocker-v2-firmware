// SD CARD Libs
#include "FS.h"
#include "SD_MMC.h"

//FILE MANAGER - contants
const char *NULL_RTR = "null";
const char * CRD_SSID = "/ssid.txt";
const char * CRD_PASS = "/pass.txt";
const char * OP_LOGS = "/opLogs.txt";
const char * ACTIVE_C_INDEX = "/cellIdx.txt";
const char * RX_DATA_FILE = "/verifiedRxData.txt";
const char * MODEX = "/modex.txt"; // allow us to run device in normal and dev mode

// SOCKET ARGS
const char * SOCK_CONFIG = "/sockConfig.txt";

bool initSD() {
  if (!SD_MMC.begin()) {
    return false;
  }
  return true;
}


//TODO best if you can make it work in const chars
String readFile(fs::FS &fs, const char * path) {
  File file = fs.open(path);
  if (!file) {
    return "null"; // return null if fails
  }
  String dat;
  while (file.available()) {
    dat += ((char) file.read());
  }
  return dat;
}

bool writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("[ERR] > SD CARD INACCESIBLE");
    return false;
  }
  if (file.print(message)) {
    return true;
  } else {
    Serial.println("[ERR] > SD CARD INACCESIBLE");
    //internal error occur (sd card failed)
    return false;
  }
}

//checking if credentials are available
WFCredentials getCredentials() {
  String ssidTmp = readFile(SD_MMC, CRD_SSID);
  String passTmp = readFile(SD_MMC, CRD_PASS);
  WFCredentials cred = {ssidTmp, passTmp};
  return cred;
}

bool hasCredentials(WFCredentials cred) {
  return !(cred.ssid == NULL_RTR && cred.pass == NULL_RTR);
}

String grabAtPath(const char * path) {
  return readFile(SD_MMC, path);
}

bool saveCredentials(String ssid, String pass, const char* ssid_path, const char* pass_path) {
  const char * c_ssid = ssid.c_str();
  const char * c_pass = pass.c_str();

  bool case1 = writeFile(SD_MMC, ssid_path, c_ssid);
  bool case2 = writeFile(SD_MMC, pass_path, c_pass);
  return (case1 && case2);
}

bool saveSocketArgs(String payload) {
  const char * socketArgs = payload.c_str();
  bool case1 = writeFile(SD_MMC, SOCK_CONFIG, socketArgs);
  return case1;
}

SocketArgs getSocketConfig() {
  String configText = readFile(SD_MMC, SOCK_CONFIG);
  StringSplitter *st = new StringSplitter(configText, ',', 3);
  SocketArgs sockArgs = {st->getItemAtIndex(0), atol(st->getItemAtIndex(1).c_str()), st->getItemAtIndex(2) };
  return sockArgs;
}

bool saveOperationLogs(OperationLogs oplogs){
  String operationData = oplogs.GLOBAL_STATE;
  operationData.concat(",");
  operationData.concat(oplogs.VALUE);
  bool result = writeFile(SD_MMC, OP_LOGS, operationData.c_str());
  return result;
}

OperationLogs getOpLogs() {
  // returns the device operation logs saved on sd card
  // use in resuming critical actions
  OperationLogs opLogs;
  String opLogsData = readFile(SD_MMC, OP_LOGS);
  if (opLogsData == NULL_RTR) {
    // if no op logs create it and return a valid op log
    String payload = "0,null";
    writeFile(SD_MMC, OP_LOGS, payload.c_str());
    opLogs = { "0", "null" };
    return opLogs;
  }
  StringSplitter *st = new StringSplitter(opLogsData, ',', 2);
  opLogs = {st->getItemAtIndex(0), st->getItemAtIndex(1)};
  return opLogs;
}

int getActiveCellIndex(){
  String cellIndex = readFile(SD_MMC, ACTIVE_C_INDEX);
  int INDEX = cellIndex.toInt();
  return INDEX;
}

bool saveActiveCellIndex(){
  String cellIndex = String(ACTIVE_CELL_INDEX);
  bool result = writeFile(SD_MMC, ACTIVE_C_INDEX, cellIndex.c_str());
  return result;
}

String getSavedRxData(){
  String savedRxData = readFile(SD_MMC, RX_DATA_FILE);
  savedRxData = (savedRxData == NULL_RTR) ? "" : savedRxData;
  return savedRxData;
}

bool saveRxData(){
  bool result = writeFile(SD_MMC, RX_DATA_FILE, RX_DATA.c_str());
  return result;
}

int getModex(){
  String modeState = readFile(SD_MMC, MODEX);
  if(modeState != NULL_RTR){
    return modeState.toInt();
  }
  return DEVICE_MODES.normMode; // normal mode
}
