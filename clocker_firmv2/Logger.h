//#define DEBUG_DISABLE

#ifndef DEBUG_DISABLE

void console_notice(const char *msg) {
  Serial.print("[NOTICE] ");
  Serial.println(msg);
}

void console_warning(const char *msg) {
  Serial.print("[WARN] ");
  Serial.println(msg);
}

void console_error(const char *msg) {
  Serial.print("[ERR] ");
  Serial.println(msg);
}

#endif
