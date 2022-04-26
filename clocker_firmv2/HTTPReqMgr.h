#include <HTTPClient.h>

String requestUrl = "";

// use to authenticate self if included in the database
bool checkRequirements() {
  bool status = false;
  HTTPClient http;

  if (GLOBAL_DEVICE_MODE == DEVICE_MODES.localMode) {
    SocketArgs localConfig = getLocalConfig();
    requestUrl.concat("http://");
    requestUrl.concat(localConfig.address);
    requestUrl.concat(":");
    requestUrl.concat(String(localConfig.port));
    requestUrl.concat("/locker/get-requirements");
  } else {
    //GLOBAL MODE
    requestUrl = "https://sanpedronline.herokuapp.com/locker/get-requirements";
  }

  requestUrl.concat("?mode=");
  requestUrl.concat(String(GLOBAL_DEVICE_MODE));

  Serial.println(requestUrl);

  // secret path to get requirements
  http.begin(requestUrl.c_str());
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
