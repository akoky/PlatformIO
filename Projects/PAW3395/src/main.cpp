#include <Arduino.h>
#include <SPI.h>

// Registers
#define Product_ID 0x00
#define Revision_ID 0x01
#define Motion 0x02
#define Delta_X_L 0x03
#define Delta_X_H 0x04
#define Delta_Y_L 0x05
#define Delta_Y_H 0x06
#define SQUAL 0x07
#define RawData_Sum 0x08
#define Maximum_RawData 0x09
#define Minimum_RawData 0x0A
#define Shutter_Lower 0x0B
#define Shutter_Upper 0x0C
#define Observation 0x15
#define Motion_Burst 0x16
#define Power_Up_Reset 0x3A
#define Shutdown 0x3B
#define Performance 0x40
#define Set_Resolution 0x47
#define Resolution_X_Low 0x48
#define Resolution_X_High 0x49
#define Resolution_Y_Low 0x4A
#define Resolution_Y_High 0x4B
#define Angle_Snap 0x56
#define RawData_output 0x58
#define RawData_status 0x59
#define Ripple_Control 0x5A
#define Axis_Control 0x5B
#define Motion_Ctrl 0x5C
#define Inv_Product_ID 0x5F
#define Run_Downshift 0x77
#define Rest1_Period 0x78
#define Rest1_Downshift 0x79
#define Rest2_Period 0x7A
#define Rest2_Downshift 0x7B
#define Rest3_Period 0x7C
#define Run_Downshift_Mult 0x7D
#define Rest_Downshift_Mult 0x7E

// Define ESP Pwr Pin
int ESP_PWR_Pin = 2;

// Define NCS Pin
int NCS_Pin = 16;

// Define Motion Pin
int Motion_Pin = 4;

// Define NRESET pin
int NRESET_Pin = 5;

void enable_ncs()
{
  digitalWrite(NCS_Pin, LOW);
}

void disable_ncs()
{
  digitalWrite(NCS_Pin, HIGH);
}

void write_reg(byte addr, byte data)
{
  enable_ncs();

  SPI.transfer(addr | 0x80); // send adress of the register, with MSBit = 1 to indicate it's a write
  SPI.transfer(data);

  delayMicroseconds(1); // tSCLK-NCS for write operation
  disable_ncs();
  delayMicroseconds(4); // tSWW/tSWR minus tSCLK-NCS
}

byte read_reg(byte addr)
{
  enable_ncs();

  SPI.transfer(addr & 0x7F); // send adress of the register, with MSBit = 0 to indicate it's a read
  delayMicroseconds(2);      // tSRAD

  byte data = SPI.transfer(0);

  delayMicroseconds(1); // tSCLK-NCS for read operation
  disable_ncs();
  delayMicroseconds(4); // tSRW/tSRR minus tSCLK-NCS

  return data;
}

// void StartUp()
// {

//   // hard reset
//   disable_ncs();
//   enable_ncs();
//   disable_ncs();

//   write_reg(Shutdown, 0xb6);
//   delay(300);

//   // Drive NCS HIGH, then low to reset the SPI port
//   enable_ncs();
//   delayMicroseconds(40);
//   disable_ncs();
//   delayMicroseconds(40);

//   write_reg(Power_Up_Reset, 0x5a);
//   delay(50);

//   read_reg(Motion);
//   read_reg(Delta_X_L);
//   read_reg(Delta_X_H);
//   read_reg(Delta_Y_L);
//   read_reg(Delta_Y_H);

//   Serial.println("Optical Chip Initialized");
// }

void setup()
{
  Serial.begin(9600);

  // ESP Power Indicator
  pinMode(ESP_PWR_Pin, OUTPUT);
  digitalWrite(ESP_PWR_Pin, LOW);

  // NCS pin
  pinMode(NCS_Pin, OUTPUT);

  // Motion pin
  pinMode(Motion_Pin, OUTPUT);

  delay(50);

   SPI.begin();
   SPI.setDataMode(SPI_MODE3);
   SPI.setBitOrder(MSBFIRST);
  // Serial.println("");
  // Serial.println("ON");

  // StartUp();

  // // Toggle NRESET pin
  // pinMode(NRESET, OUTPUT);
  // digitalWrite(NRESET, HIGH);
  // digitalWrite(NRESET, LOW);
  // delay(5);
}

void loop()
{
  byte burstBuffer[12];

  enable_ncs();
  SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE3));
  SPI.transfer(Motion_Burst);
  delayMicroseconds(35); // wait for tSRAD

  SPI.transfer(burstBuffer, 12);
  delayMicroseconds(1); // tSCLK-NCS for read operation

  SPI.endTransaction();

  int motion = (burstBuffer[0] & 0x80) > 0;
  int xl = burstBuffer[2];
  int xh = burstBuffer[3];
  int yl = burstBuffer[4];
  int yh = burstBuffer[5];

  int x = xh << 8 | xl;
  int y = yh << 8 | yl;

  disable_ncs();

  if (motion)
  {
    Serial.print(x);
    Serial.print('\t');
    Serial.println(y);
  }

  delayMicroseconds(500);

}