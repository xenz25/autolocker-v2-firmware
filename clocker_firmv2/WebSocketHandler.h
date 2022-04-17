#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <ArduinoJson.h>

SocketIOclient io;

bool isSocketActive = false;
bool isDocumentReceived = false;
bool isNotAlreadyTimeOut = true; // use to prevent re initialization of document receiving task
bool isAssignEventAcknowledge = false;
bool isServerNotifiedCellIsFree = false;

// this task is on TASKCALLBACKS
void handleSIOResponse(uint8_t *);
String eventBuilder(String, struct Dictionary[], int);

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++) {
    if (i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      Serial.printf("[IOc] Disconnected!\n");
      //printConnectionLost();
      isAssignEventAcknowledge = false; // for locker loggin on server side
      isSocketActive = false;
      break;
    case sIOtype_CONNECT: {
        Serial.printf("[IOc] Connected to url: %s\n", payload);
        // join default namespace (no auto join in Socket.IO V3)
        io.send(sIOtype_CONNECT, "/");
        isSocketActive = true;
        //emit assign to server task
        tAwaitAssignToServer.restart();
        //printLastPrompt();
        break;
      }
    case sIOtype_EVENT: {
        Serial.printf("[IOc] get event: %s\n", payload);
        handleSIOResponse(payload);
        break;
      }
    case sIOtype_ACK:
      Serial.printf("[IOc] get ack: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_ERROR:
      Serial.printf("[IOc] get error: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_EVENT:
      Serial.printf("[IOc] get binary: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_ACK:
      Serial.printf("[IOc] get binary ack: %u\n", length);
      hexdump(payload, length);
      break;
  }
}

void initWebSocketConnection() {
  SocketArgs socketArgs = getSocketConfig();
  Serial.println(socketArgs.address);
  Serial.println(socketArgs.port);
  Serial.println(socketArgs.url);

  // INIT CONNECTION
  io.begin("192.168.100.57", socketArgs.port, socketArgs.url);
  io.onEvent(socketIOEvent);
  io.setReconnectInterval(5000);
}


String eventBuilder(String eventName, struct Dictionary payloads[], int payload_len) {
  // creat JSON message for Socket.IO (event)
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  // add event name
  // Hint: socket.on('event_name', ....
  array.add(eventName);

  // add payload (parameters) for the event
  JsonObject param1 = array.createNestedObject();
  for (int i = 0; i < payload_len; i++) {
    param1[payloads[i].key] = payloads[i].value;
  }

  // JSON to String (serializion)
  String output;
  serializeJson(doc, output);

  return output;
}

SocketResponse serializeResponse(uint8_t * payload) {
  DynamicJsonDocument doc(1024);
  // put your setup code here, to run once:
  //  uint8_t * payload = "[{\"EVENT\": \"RESPONSE\",\"VALUE\": \"OK\"}]";

  deserializeJson(doc, (const char *) payload);

  const char* response_type = doc[0]["EVENT"];
  const char* response_value = doc[0]["VALUE"];

  SocketResponse response = { response_type,  response_value };
  return response;
}
