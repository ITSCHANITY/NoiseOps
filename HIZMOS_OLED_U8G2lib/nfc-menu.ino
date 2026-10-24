// ============================================================================
// NFC (PN532) support
// ----------------------------------------------------------------------------
// Uses the Adafruit PN532 library over I2C, sharing the OLED's I2C bus
// (SDA=I2C_SDA / SCL=I2C_SCL). Install "Adafruit PN532" via the Library Manager.
//
// Implemented:  Read tag (reads and displays the card UID)
// Stubbed:      Write Data, Saved Tags, Emulate Tag, P2P  -> show a notice
//               (these need SD storage / more complex logic not present here)
// ============================================================================

#include <Adafruit_PN532.h>

// I2C PN532. IRQ/RESET not wired separately here, so pass -1.
static Adafruit_PN532 nfc(-1, -1, &Wire);
static bool nfcReady = false;

// Initialize the PN532 once. Returns true if the reader responds.
bool nfcInit() {
  if (nfcReady) return true;

  Wire.begin(I2C_SDA, I2C_SCL);
  nfc.begin();

  uint32_t version = nfc.getFirmwareVersion();
  if (!version) {
    return false;  // reader not found
  }

  nfc.SAMConfig();   // configure to read RFID tags
  nfcReady = true;
  return true;
}

// Blocking-ish "Read tag" screen. Runs until BACK is pressed.
void nfcReadTag() {
  // Try to bring the reader up first.
  if (!nfcInit()) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(6, 20, "PN532 not found");
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(6, 36, "Check I2C wiring");
    u8g2.drawStr(6, 46, "SDA=GPIO8 SCL=GPIO9");
    u8g2.drawStr(6, 60, "BACK to exit");
    u8g2.sendBuffer();
    while (digitalRead(BTN_BACK) == HIGH) { delay(20); }
    while (digitalRead(BTN_BACK) == LOW)  { delay(20); }
    return;
  }

  uint8_t uid[7];
  uint8_t uidLength = 0;

  while (true) {
    // Exit on BACK
    if (digitalRead(BTN_BACK) == LOW) {
      while (digitalRead(BTN_BACK) == LOW) { delay(20); }
      break;
    }

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(4, 11, "NFC - Read Tag");
    u8g2.drawHLine(0, 14, 128);

    // 100ms non-blocking-ish poll for an ISO14443A card
    bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);

    u8g2.setFont(u8g2_font_5x7_tr);
    if (found && uidLength > 0) {
      setColor(0, 255, 0);  // green on read

      char line[40];
      sprintf(line, "UID len: %d bytes", uidLength);
      u8g2.drawStr(4, 28, line);

      // Format UID as hex
      String hex = "";
      for (uint8_t i = 0; i < uidLength; i++) {
        char b[4];
        sprintf(b, "%02X", uid[i]);
        hex += b;
        if (i < uidLength - 1) hex += ":";
      }
      u8g2.drawStr(4, 40, "UID:");
      u8g2.drawStr(4, 50, hex.c_str());

      Serial.print("NFC UID: ");
      Serial.println(hex);
    } else {
      setColor(0, 0, 0);
      u8g2.drawStr(4, 34, "Waiting for tag...");
    }

    u8g2.drawStr(4, 63, "BACK to exit");
    u8g2.sendBuffer();
  }

  setColor(0, 0, 0);
}

// Simple notice for features not implemented in this build.
void nfcNotImplemented(const char* feature) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(4, 16, feature);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(4, 34, "Not implemented yet");
  u8g2.drawStr(4, 63, "BACK to exit");
  u8g2.sendBuffer();
  while (digitalRead(BTN_BACK) == HIGH) { delay(20); }
  while (digitalRead(BTN_BACK) == LOW)  { delay(20); }
}

void handlenfcmenu() {
  const char* menuItems[] = {"Read tag", "Write Data", "Saved Tags", "Emulate Tag", "P2P comunication"};
  const int menuLength = sizeof(menuItems) / sizeof(menuItems[0]);
  const int visibleItems = 3;

  static int selectedItem = 0;
  static int scrollOffset = 0;
  static unsigned long lastInputTime = 0;
  
  if (millis() - lastInputTime > 150) {
    if (digitalRead(BTN_UP) == LOW) {
      selectedItem--;
      if (selectedItem < 0) selectedItem = menuLength - 1;
      scrollOffset = constrain(selectedItem - visibleItems + 1, 0, menuLength - visibleItems);
      lastInputTime = millis();
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      selectedItem++;
      if (selectedItem >= menuLength) selectedItem = 0;
      scrollOffset = constrain(selectedItem - visibleItems + 1, 0, menuLength - visibleItems);
      lastInputTime = millis();
    }

    if (digitalRead(BTN_SELECT) == LOW) {
      switch (selectedItem) {
        case 0:
          nfcReadTag();
          break;
        case 1:
          nfcNotImplemented("Write Data");
          break;
        case 2:
          nfcNotImplemented("Saved Tags");
          break;
        case 3:
          nfcNotImplemented("Emulate Tag");
          break;
        case 4:
          nfcNotImplemented("P2P comunication");
          break;
      }
      lastInputTime = millis();
    }
  }

 
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tf);

  for (int i = 0; i < visibleItems; i++) {
    int menuIndex = i + scrollOffset;
    if (menuIndex >= menuLength) break;

    int y = i * 20 + 16;
    if (menuIndex == selectedItem) {
      u8g2.drawRBox(4, y - 12, 120, 16, 4);
      u8g2.setDrawColor(0);
      u8g2.drawStr(10, y, menuItems[menuIndex]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(10, y, menuItems[menuIndex]);
    }
  }

  int barX = 124;
  int spacing = 64 / menuLength;

  for (int i = 0; i < menuLength; i++) {
    int dotY = i * spacing + spacing / 2;
    if (i == selectedItem) {
      u8g2.drawBox(barX, dotY - 3, 3, 6);
    } else {
      u8g2.drawPixel(barX + 1, dotY);
    }
  }

  u8g2.sendBuffer();
}
