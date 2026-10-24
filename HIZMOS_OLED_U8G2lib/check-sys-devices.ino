// Boot self-test trimmed to hardware present on this build:
//   - OLED (if this screen draws, it works)
//   - CC1101 (probed over its SPI bus via getCC1101())
//   - PN532 NFC (probed over I2C at address 0x24)
// The old nRF24 x2 + SD checks were removed (hardware not present).

#define PN532_I2C_ADDR 0x24

void checksysdevices() {
  static const unsigned char image_satellite_dish_bits[] U8X8_PROGMEM = {
    0x00,0x00,0x00,0x1e,0x0c,0x20,0x34,0x4c,0xc6,0x50,0x15,0x57,
    0x49,0x45,0x09,0x07,0x91,0x08,0x51,0x12,0x22,0x10,0xc2,0x24,
    0x04,0x23,0x0c,0x3c,0x30,0x08,0xc0,0x07
  };
  static const unsigned char image_nfc_bits[] U8X8_PROGMEM = {
    0x00,0x08,0x00,0x10,0x00,0x12,0x00,0x22,0x42,0x24,0x87,0x24,
    0x8d,0x24,0x99,0x24,0xf1,0x24,0x62,0x24,0x00,0x22,0x00,0x12,
    0x00,0x10,0x00,0x08
  };
  static const unsigned char image_screen_bits[] U8X8_PROGMEM = {
    0xff,0x1f,0x01,0x10,0x01,0x10,0x01,0x10,0x01,0x10,0x01,0x10,
    0x01,0x10,0xff,0x1f,0x00,0x00,0x7c,0x00,0x00,0x00,0x00,0x00
  };

  // ---- Device states ----
  String oled_state   = "work";   // OLED is drawing this screen, so it works
  String cc1101_state = "error";
  String nfc_state    = "error";

  // ---- Probe CC1101 (module #1 pins) ----
  ELECHOUSE_cc1101.setSpiPin(cc1101_SCK, cc1101_MISO, cc1101_MOSI, CC1101_CS);
  ELECHOUSE_cc1101.setGDO(CC1101_GDO0, CC1101_GDO2);
  if (ELECHOUSE_cc1101.getCC1101()) {
    cc1101_state = "work";
  }

  // ---- Probe PN532 NFC over I2C ----
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.beginTransmission(PN532_I2C_ADDR);
  if (Wire.endTransmission() == 0) {
    nfc_state = "work";
  }

  // Green LED if the two external modules are present
  if (cc1101_state == "work" && nfc_state == "work") {
    setColor(0, 255, 0);
  }

  // ---- Draw results ----
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);

  // Icons
  u8g2.drawXBMP(10, 20, 14, 16, image_screen_bits);          // OLED
  u8g2.drawXBMP(47, 20, 15, 16, image_satellite_dish_bits);  // CC1101
  u8g2.drawXBMP(91, 19, 14, 16, image_nfc_bits);             // NFC

  // Labels
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(9,  10, "oled");
  u8g2.drawStr(43, 10, "cc1101");
  u8g2.drawStr(90, 10, "nfc");

  // States
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(5,  53, oled_state.c_str());
  u8g2.drawStr(42, 53, cc1101_state.c_str());
  u8g2.drawStr(86, 53, nfc_state.c_str());

  u8g2.sendBuffer();
}
