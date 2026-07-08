#include <SPI.h>

const int CS_SAR  = 10;   // SAR DAC chip select
const int CS_OUT  = 9;    // Reconstruction DAC chip select

const int CMP_PIN = A0;   // Comparator output
const int VIN_DBG = A1;   // Sample-and-hold output (debug)

const int SAR_BITS = 8;
const float VREF = 5.0;

void writeDAC(int csPin, uint16_t value) {
  value &= 0x0FFF;                  // Ensure 12-bit data
  uint16_t command = 0x3000 | value; // MCP4921 configuration

  digitalWrite(csPin, LOW);
  SPI.transfer(highByte(command));
  SPI.transfer(lowByte(command));
  digitalWrite(csPin, HIGH);
}

uint8_t sarADC() {
  uint8_t code = 0;

  for (int bit = SAR_BITS - 1; bit >= 0; bit--) {
    code |= (1 << bit);  // Tentatively set bit

    uint16_t dacValue = ((uint16_t)code << 4);
    writeDAC(CS_SAR, dacValue);

    delayMicroseconds(5); // DAC settling time

    int cmp = analogRead(CMP_PIN);

    if (cmp < 100) {
      code &= ~(1 << bit); // Clear bit if Vin < Vdac
    }
  }
  return code;
}

void setup() {
  pinMode(CS_SAR, OUTPUT);
  pinMode(CS_OUT, OUTPUT);

  digitalWrite(CS_SAR, HIGH);
  digitalWrite(CS_OUT, HIGH);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE0);

  Serial.begin(9600);
}

void loop() {
  uint8_t adcCode = sarADC();

  uint16_t dacOut = ((uint16_t)adcCode << 4);
  writeDAC(CS_OUT, dacOut);

  float quantV = (adcCode * VREF) / 255.0;
  int vinDbg = analogRead(VIN_DBG);

  Serial.print("ADC Code: ");
  Serial.print(adcCode);
  Serial.print("  Quantised Voltage = ");
  Serial.print(quantV, 3);
  Serial.print(" V  | Vin analog read: ");
  Serial.println(vinDbg);

  delay(66); // Sampling rate ≈ 15 Hz