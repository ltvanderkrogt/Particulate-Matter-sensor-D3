#include <SoftwareSerial.h>

SoftwareSerial mySerial(D1, D2); // RX, TX

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

void printValues(byte data[]) {
  // Print the values to the terminal
  int pm1_standard = (data[4] << 8) | data[5];
  int pm25_standard = (data[6] << 8) | data[7];
  int pm10_standard = (data[8] << 8) | data[9];

  Serial.print("PM1.0: ");
  Serial.print(pm1_standard);
  Serial.print(" ug/m3, PM2.5: ");
  Serial.print(pm25_standard);
  Serial.print(" ug/m3, PM10: ");
  Serial.print(pm10_standard);
  Serial.println(" ug/m3");
}

uint16_t calculateChecksum(byte data[]) {
  uint16_t checksum = 0;

  // Sum the bytes from index 0 to 29
  for (int i = 0; i < 30; i++) {
    checksum += data[i];
  }

  return checksum;
}

void loop() {
  if (mySerial.available() >= 32) {
    // Wait for the reception of at least 32 bytes (complete data frame)
    if (mySerial.read() == 0x42 && mySerial.read() == 0x4D) {
      // Check for the beginning of a data frame
      byte data[32];
      for (int i = 0; i < 32; i++) {
        data[i] = mySerial.read();
      }

      // Check the received checksum
      uint16_t receivedChecksum = (data[30] << 8) | data[31];
      uint16_t calculatedChecksum = calculateChecksum(data);

      if (receivedChecksum == 0xFFFF || receivedChecksum == calculatedChecksum) {
        // Print the values
        printValues(data);
      } else {
        // Print received bytes for debugging
        debugPrintBytes(data, 32);
        // Calculate and print the correct checksum
        calculateAndPrintChecksum(data);
      }
    }
  }
}

void debugPrintBytes(byte data[], int length) {
  const char* descriptions[] = {
    "BYTE0 Starter1  Starter1 0x42",
    "BYTE1 Starter2  Starter2 0x4d",
    "BYTE2 High  Frame length",
    "BYTE3 Low Frame length",
    "BYTE4 High  PM1.0 standard",
    "BYTE5 Low PM1.0 standard",
    "BYTE6 High  PM2.5 standard",
    "BYTE7 Low PM2.5 standard",
    "BYTE8 High  PM10 standard",
    "BYTE9 Low PM10 standard",
    "BYTE10  High  PM1.0 atmospheric",
    "BYTE11  Low PM1.0 atmospheric",
    "BYTE12  High  PM2.5 atmospheric",
    "BYTE13  Low PM2.5 atmospheric",
    "BYTE14  High  PM10 atmospheric",
    "BYTE15  Low PM10 atmospheric",
    "BYTE16  High  PM0.1 total",
    "BYTE17  Low   PM0.1 total",
    "BYTE18  High  PM0.5 total",
    "BYTE19  Low   PM0.5 total",
    "BYTE20  High  PM1.0 total",
    "BYTE21  Low   PM1.0 total",
    "BYTE22  High  PM2.5 total",
    "BYTE23  Low   PM2.5 total",
    "BYTE24  High  PM5.0 total",
    "BYTE25  Low   PM5.0 total",
    "BYTE26  High  PM10 total",
    "BYTE27  Low   PM10 total",
    "BYTE28  Low Reserved",
    "BYTE29  High  Reserved",
    "BYTE30  Low checksum =(BYTE0+BYTE1+…+BYTE29)",
    "BYTE31  High checksum =(BYTE0+BYTE1+…+BYTE29)"
  };

  for (int i = 0; i < length; i++) {
    Serial.print("Byte ");
    Serial.print(i + 1);
    Serial.print(": 0b");
    for (int j = 7; j >= 0; j--) {
      Serial.print((data[i] >> j) & 1);
    }

    Serial.print(", 0x");
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);

    Serial.print(", ");
    Serial.print(static_cast<int>(data[i]));

    Serial.print(", Description: ");
    Serial.println(descriptions[i]);

    if ((i + 1) % 4 == 0) {
      // Print a blank line after every 4 bytes for better readability
      // Serial.println();
    }
  }
}

void calculateAndPrintChecksum(byte data[]) {
  uint16_t checksum = calculateChecksum(data);

  Serial.print("Checksum is ");
  if (checksum < 0x1000) Serial.print("0");
  if (checksum < 0x100) Serial.print("0");
  if (checksum < 0x10) Serial.print("0");
  Serial.print(checksum, HEX);

  Serial.print(" and should be ");
  if (data[30] < 0x10) Serial.print("0");
  Serial.print(data[30], HEX);
  if (data[31] < 0x10) Serial.print("0");
  Serial.println(data[31], HEX);

  if (checksum == ((data[30] << 8) | data[31]) || data[30] == 0xFF || data[31] == 0xFF) {
    Serial.println("Checksum is correct!");
  } else {
    Serial.println("Checksum is incorrect!");
  }
}
