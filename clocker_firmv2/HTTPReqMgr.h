#include <HTTPClient.h>

// use to authenticate self if included in the database
bool checkRequirements() {
  bool status = false;
  HTTPClient http;

  // secret path to get requirements
  http.begin(HTTP_RQ_PATH);
  // SET LOCKER AUTH AS KEY AN PASS FOR AUTHORIZATION
  http.setAuthorization(LOCKER_NAME, LOCKER_AUTH_KEY);

  int httpCode = http.GET();

  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      status = saveSocketArgs(http.getString());
    }
  } 

  http.end();

  return status;
}
