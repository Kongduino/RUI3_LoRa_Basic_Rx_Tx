bool tx_done = true;
uint32_t lastSend, pingCounter = 0;

#define PING_INTERVAL 30000 // 10 seconds

void hexDump(uint8_t * buf, uint16_t len) {
  char alphabet[17] = "0123456789abcdef";
  Serial.print(F("   +------------------------------------------------+ +----------------+\r\n"));
  Serial.print(F("   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |\r\n"));
  for (uint16_t i = 0; i < len; i += 16) {
    if (i % 128 == 0)
      Serial.print(F("   +------------------------------------------------+ +----------------+\r\n"));
    char s[] = "|                                                | |                |\r\n";
    uint8_t ix = 1, iy = 52;
    for (uint8_t j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t c = buf[i + j];
        s[ix++] = alphabet[(c >> 4) & 0x0F];
        s[ix++] = alphabet[c & 0x0F];
        ix++;
        if (c > 31 && c < 128) s[iy++] = c;
        else s[iy++] = '.';
      }
    }
    uint8_t index = i / 16;
    if (i < 256)
      Serial.write(' ');
    Serial.print(index, HEX);
    Serial.write('.');
    Serial.print(s);
  }
  Serial.print(F("   +------------------------------------------------+ +----------------+\r\n"));
}

void send_cb(void) {
  Serial.printf("P2P set Rx mode %s\r\n", api.lorawan.precv(65534) ? "Success" : "Fail");
  tx_done = true; // alert loop() that Tx is finished.
}

void setup() {
  Serial.begin(115200);
  Serial.println("RAKwireless LoRaWan P2P Rx Example");
  Serial.println("------------------------------------------------------");
  delay(2000);
  if (api.lorawan.nwm.get() != 0) {
    Serial.printf("Set Node device work mode %s\r\n", api.lorawan.nwm.set(0) ? "Success" : "Fail");
    api.system.reboot();
  }
  Serial.println("P2P Start");
  Serial.printf("Set Node device work mode %s\r\n", api.lorawan.nwm.set(0) ? "Success" : "Fail");
  Serial.printf("Set P2P mode frequency %s\r\n", api.lorawan.pfreq.set(868000000) ? "Success" : "Fail");
  Serial.printf("Set P2P mode spreading factor %s\r\n", api.lorawan.psf.set(12) ? "Success" : "Fail");
  Serial.printf("Set P2P mode bandwidth %s\r\n", api.lorawan.pbw.set(125) ? "Success" : "Fail");
  Serial.printf("Set P2P mode code rate %s\r\n", api.lorawan.pcr.set(0) ? "Success" : "Fail");
  Serial.printf("Set P2P mode preamble length %s\r\n", api.lorawan.ppl.set(8) ? "Success" : "Fail");
  Serial.printf("Set P2P mode tx power %s\r\n", api.lorawan.ptp.set(22) ? "Success" : "Fail");
  api.lorawan.registerPSendCallback(send_cb);
  lastSend = millis() - PING_INTERVAL;
}

void loop() {
  if (millis() - lastSend >= PING_INTERVAL) {
    // time for a PING. Maybe.
    if (!tx_done) {
      Serial.println("Previous Tx isn't finished yet. Check settings, maybe, if it is taking too long...");
      lastSend = millis() - 1000; // retry in 1 second
      return;
    }
    char payload[32];
    sprintf(payload, "Packet #%08d", pingCounter++);
    tx_done = false; // Letting loop know we are sending
    api.lorawan.precv(0);
    uint8_t ln = strlen(payload) + 1;
    bool result = api.lorawan.psend(ln, (uint8_t*)payload);
    hexDump((uint8_t*)payload, ln);
    Serial.printf("P2P send %s\r\n", result ? "Success" : "Fail");
    if (!result) {
      Serial.println("  --> Check settings probably...");
      tx_done = true;
    }
    lastSend = millis(); // update timer
  }
}
