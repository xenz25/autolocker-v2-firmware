
struct WFCredentials {
  String ssid;
  String pass;
};

struct RoutinePayload {
  bool validity;
  const char * responseText;
};

struct SocketArgs {
  String address;
  long port;
  String url;
};

// locker message types
struct SocketMessageTypes {
  String verify = "QR_VERIFY";
  String update = "STATUS_UPDATE";
  String checkDocStatus = "CHECK_DOCUMENT_RECEIVE";
  String updateToFree = "CELL_FREE";
} SOCKET_MSG_TYPES;

struct SocketEventTypes {
  String message = "MESSAGE";
  String error = "ERROR";
  String assign = "ASSIGN";
  String pong = "PONG";
} SOCKET_EVENT_TYPES;

// server response types
struct SocketResponseTypes {
  const char * verify = "QR_VERIFY_RESPONSE";
  const char * checkDocStatus = "DOC_STATUS_RESPONSE";
  const char * assign = "ASSIGN";
  const char * ping = "PING";
  const char * updateToFree = "UPDATE_TO_FREE_RESPONSE";
  const char * reboot = "REBOOT";
} SOCKET_RESPONSE_TYPES;

struct SocketPayloads {
  const char * ok = "OK";
  const char * fail = "NO";
} SOCKET_RESPONSE_PAYLOADS;

struct Dictionary {
  String key;
  String value;
};

struct SocketResponse {
  const char * RESPONSE_TYPE;
  const char * VALUE;
};

// if the door state change on any cell doors and the state of change flag is not from scanner
// some one probably open the cell door without a scanner
// else if from scanner ignore the state
struct ChangeLockActionFlag {
  uint8_t defaultValue = -1; // use to flag the door iistener
  uint8_t fromScanner = 0;
} CHANGE_ACTION_FLAG_TYPE;

struct OperationLogs {
  String GLOBAL_STATE;
  String VALUE;
};

struct DeviceModes {
  int localMode = 1;// WILL TAKE EFFECT ON HTTP AND SOCKET CONFIGUTRATION
  int globalMode = 0;
} DEVICE_MODES;
