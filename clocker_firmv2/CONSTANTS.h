// AUTH
const char * LOCKER_NAME = "AUTO-LOCKER-V1.0.0";
const char * LOCKER_AUTH_KEY = "zxGFH79aKJBTeh2C";
const uint8_t MAX_CELL_COUNT = 5;
const char * HTTP_RQ_PATH = "http://192.168.100.57:3000/locker/get-requirements";
//const char * HTTP_RQ_PATH = "https://sanpedronline.herokuapp.com/locker/get-requirements";

const unsigned long PROMPT_INTERVAL = 2000;

// IR AND SCANNER
#define IR_OUT 35
#define SC_SIG 32
String RX_DATA = "";

// DOOR LOCK
// use by the door state listener to identify if changing is from the scanner
uint8_t CHANGE_ACTION_FLAG = CHANGE_ACTION_FLAG_TYPE.defaultValue;
#define NON_SELECTABLE_ADDRESS 5 // we only read upto 4 0 address means first cell
uint8_t ACTIVE_CELL_INDEX = NON_SELECTABLE_ADDRESS;

// ---- ERROR CONSTANTS ---- //
const char* ERR_INACCESSIBLE_SD = "[ERR] 0x0";
const char* ERR_NO_MATCHING_HTML = "[ERR] 0x1";
const char* ERR_CANNOT_CONNECT = "[ERR] 0x2";
const char* ERR_NVALIDATED = "[ERR] 0x3";

// --- SUCCESS CONTANTS --- //
const char* SUCC_CONNECTED = "[SUCC] 0x0";
const char* SUCC_VALIDATED = "[SUCC] 0x1";
