#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// --- AP MODE --- //
//init server
AsyncWebServer server(80); //server at port 80

bool wasAPStarted = false;

// AP MODE FLAGS
bool isAlreadySaved = false;

String wf_idx;
String wf_suc;

void initEvents();

bool startAPMode() {
  //constants
  const char *AP_SSID = "AUTO-LOCKER-V1";
  const char *AP_PASS = "XWt6DYyZ"; //XBOX WALMART tokyo 6 DRIP YELP yelp ZIP

  // grab html data
  wf_idx = grabAtPath("/wf_idx.txt");
  wf_suc = grabAtPath("/wf_suc.txt");

  if (wf_idx != NULL_RTR && wf_suc != NULL_RTR) {
    while (!WiFi.softAP(AP_SSID, AP_PASS)) {
      //passed
    }
    delay(PROMPT_INTERVAL);
    isAlreadySaved = false;
    initEvents();
    server.begin(); //start server
    return true;
  }
  return false; // raise ERR_NO_MATCHING_HTML
}

void AP_MODE_ROUTINE() {
  wasAPStarted = true;
  printNorm("Starting", 0, 0, true);
  printNorm("AP Mode...", 0, 1, false);
  delay(PROMPT_INTERVAL);

  if (startAPMode()) {
    const char * AP_ADDR = "192.168.4.1";

    // prompt the user
    printNorm("AP Started", 0, 0, true);
    delay(PROMPT_INTERVAL);
    printNorm("Connect to: ", 0, 0, true);
    printNorm(AP_ADDR, 0, 1, false);

    lcdMakeBlink();
  } else {
    console_error(ERR_NO_MATCHING_HTML);
    printErr(ERR_NO_MATCHING_HTML);
  }
}
// AP MODE EVENTS
void initEvents() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    isAlreadySaved = false;
    request->send(200, "text/html", wf_idx);
  });

  server.on("/success", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (isAlreadySaved) {
      request->redirect("/");
    } else {
      isAlreadySaved = true;
      request->send(200, "text/html", wf_suc);
    }
  });

  server.on("/save-credentials", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (isAlreadySaved) {
      request->redirect("/");
    } else {
      if (request->hasParam("input_ssid") && request->hasParam("input_pass")) {
        String ssid = request->getParam("input_ssid")->value();
        String pass = request->getParam("input_pass")->value();

        //save to sd card
        if (saveCredentials(ssid, pass, CRD_SSID, CRD_PASS)) {
          request->redirect("/success");
          delay(PROMPT_INTERVAL);

          printNorm("Credentials", 0, 0, true);
          printNorm("Recorded...", 0, 1, false);

          delay(PROMPT_INTERVAL * 2);
          printNorm("Restarting...", 0, 0, true);

          delay(PROMPT_INTERVAL);
          restartMachine(PROMPT_INTERVAL);
        }
      } else {
        request->redirect("/");
      }
    }
  });

  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404, "text/html", "<h1>Page Not Found!</h1>");
  });
}
// --- AP MODE --- //

// --- WIFI MODE --- //
// CONSTANTS
const char *WF_SSID;
const char *WF_PASS;

bool startWiFiMode() {
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(LOCKER_NAME); //SET HOST NAME SO WE CAN IDENTIFY IT ON OUR ROUTER
  int CONNECT_ATTEMPT = 0;

  while ( WiFi.status() != WL_CONNECTED) {
    console_notice("Attempting to connect to SSID: ");
    console_notice(WF_SSID);
    WiFi.begin(WF_SSID, WF_PASS);
    if (CONNECT_ATTEMPT++ >= 3)break;
    delay(10000);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool CONNECTION_ROUTINE(WFCredentials cred) {

  WF_SSID = (cred.ssid.c_str());
  WF_PASS = (cred.pass.c_str());

  Serial.println(WF_SSID);
  Serial.println(WF_PASS);

  printNorm("Connecting to:", 0, 0, true);
  printNormStr(WF_SSID, 0, 1, false);
  lcd.setCursor(15, 0);
  lcdMakeBlink();

  if (startWiFiMode()) {
    delay(PROMPT_INTERVAL);
    printNorm("Connection", 0, 0, true);
    printNorm("Established.", 0, 1, false);
    return true;
  } else {
    printNorm("Connection", 0, 0, true);
    printNorm("Failed.", 0, 1, false);
    lcdMakeBlink();
    delay(PROMPT_INTERVAL);
    AP_MODE_ROUTINE(); // start ap mode on connection failed
  }
  return false;
}
