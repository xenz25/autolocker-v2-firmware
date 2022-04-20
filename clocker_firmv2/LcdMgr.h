#include <LiquidCrystal_I2C.h>

#define LCD_C 16
#define LCD_R 2

LiquidCrystal_I2C lcd(0x27, LCD_C, LCD_R);

uint8_t LAST_PRINTED_CODE = 0; // so we can print the last printed prompt in case of interruption

void initLCD() {
  lcd.init();
  lcd.backlight();
}

//lcd functions - displaying data
void horAPrint(const char *text, int row) {
  int space = (LCD_C - strlen(text)) / 2;
  lcd.setCursor(space, row);
  lcd.print(text);
}

void clDisp() {
  lcd.clear();
}

void printNorm(const char *text, int col, int row, bool clean) {
  if (clean) clDisp();
  lcd.setCursor(col, row);
  lcd.print(text);
}

void printNormStr(String text, int col, int row, bool clean) {
  if (clean) clDisp();
  lcd.setCursor(col, row);
  lcd.print(text);
}

void printBlank(int col, int row) {
  lcd.setCursor(col, row);
  lcd.write(254);
}

void printErr(const char* err) {
  printNorm(err, 0, 0, true);
  printNorm("Please Restart..", 0, 1, false);
}

void lcdMakeBlink() {
  lcd.blink();
}

void printConnectionLost() {
  printNorm("Connection lost", 0, 0, true);
  printNorm("Reconnecting...", 0, 1, false);
}


void printWasOpenedByForce() {
  printNorm("A cell was", 0, 0, true);
  printNorm("opened by force.", 0, 1, false);
}

void printWaitingResponse() {
  LAST_PRINTED_CODE = 0;
  printNorm("Place QR Code", 0, 0, true);
  printNorm("to scan...", 0, 1, false);
  lcdMakeBlink();
}

void printReadingYourCode() {
  LAST_PRINTED_CODE = 1;
  printNorm("Reading your", 0, 0, true);
  printNorm("code...", 0, 1, false);
  lcdMakeBlink();
}

void printVerifyingYourCode() {
  LAST_PRINTED_CODE = 2;
  printNorm("Verifying your", 0, 0, true);
  printNorm("Code...", 0, 1, false);
  lcdMakeBlink();
}

bool printUnknownCode() {
  LAST_PRINTED_CODE = 3;
  clDisp();
  horAPrint("Unknown Code", 0);
  horAPrint("Provided", 1);
  lcd.noBlink();
  return true;
}

bool printThatDidNotWork() {
  LAST_PRINTED_CODE = 10;
  printNorm("That didn't", 0, 0, true);
  printNorm("work :( ", 0, 1, false);
  lcd.noBlink();
  return true;
}

void printOpeningCell() {
  LAST_PRINTED_CODE = 4;
  String toPrint = "Opening Cell: ";
  toPrint.concat((ACTIVE_CELL_INDEX + 1));
  printNorm(toPrint.c_str(), 0, 0, true);
}

void printPleaseUpdateToReceive() {
  LAST_PRINTED_CODE = 5;
  printNorm("Please update", 0, 0, true);
  printNorm("to received...", 0, 1, false);
}

void printGrabYourDocuments() {
  LAST_PRINTED_CODE = 6;
  printNorm("You can get", 0, 0, true);
  printNorm("your documents.", 0, 1, false);
  lcdMakeBlink();
}

void printPleaseCloseTheDoor() {
  LAST_PRINTED_CODE = 7;
  printNorm("Please close", 0, 0, true);
  printNorm("the door...", 0, 1, false);
  lcdMakeBlink();
}

void printPleaseWait() {
  LAST_PRINTED_CODE = 8;
  printNorm("Please wait...", 0, 0, true);
  lcdMakeBlink();
}

void printThankYou() {
  LAST_PRINTED_CODE = 9;
  printNorm("Thank you :)", 0, 0, true);
  printNorm("Good luck today!", 0, 1, false);
}

void printConnectingToServer() {
  printNorm("Connecting", 0, 0, true);
  printNorm("to server...", 0, 1, false);
  lcdMakeBlink();
}

void printRetrying() {
  printNorm("Retrying...", 0, 0, true);
  lcdMakeBlink();
}

void printLastPrompt() {
  switch (LAST_PRINTED_CODE) {
    case 0:
      printWaitingResponse();
      break;
    case 1:
      printReadingYourCode();
      break;
    case 2:
      printVerifyingYourCode();
      break;
    case 3:
      printUnknownCode();
      break;
    case 4:
      printOpeningCell();
      break;
    case 5:
      printPleaseUpdateToReceive();
      break;
    case 6:
      printGrabYourDocuments();
      break;
    case 7:
      printPleaseCloseTheDoor();
      break;
    case 8:
      printPleaseWait();
      break;
    case 9:
      printThankYou();
      break;
  }
}
