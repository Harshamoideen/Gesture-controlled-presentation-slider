#include <Wire.h>
#include <BleKeyboard.h>
#include <Arduino.h>

BleKeyboard bleKeyboard("Gesture Remote", "ESP32", 100);

#define APDS9960_ADDR 0x39
uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(APDS9960_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(APDS9960_ADDR, 1);
  return Wire.read();
}

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(APDS9960_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

int lastTotal = 0;

//micro
#define MIC_PIN 35
int threshold =2000; // Adjust this threshold based on your environment and microphone sensitivity
int lastVal = 0;
unsigned long lastClap = 0;
unsigned long clapTimes[3];
int clapIndex = 0;
unsigned long lastClapTime = 0;

bool unlock = false;

unsigned long lastGestureTime = 0;
#define GESTURE_COOLDOWN 800  // ms — ignore gestures for 800ms after one fires

// Register addresses
#define REG_ENABLE    0x80
#define REG_ATIME     0x81
#define REG_WTIME     0x83
#define REG_CONFIG1   0x8D
#define REG_CONFIG2   0x90
#define REG_PPULSE    0x8E
#define REG_CONTROL   0x8F
#define REG_ID        0x92
#define REG_STATUS    0x93
#define REG_PDATA     0x9C

// Gesture registers
#define REG_GPENTH    0xA0
#define REG_GEXTH     0xA1
#define REG_GCONF1    0xA2
#define REG_GCONF2    0xA3
#define REG_GOFFSET_U 0xA4
#define REG_GOFFSET_D 0xA5
#define REG_GPULSE    0xA6
#define REG_GOFFSET_L 0xA7
#define REG_GOFFSET_R 0xA9
#define REG_GCONF3    0xAA
#define REG_GCONF4    0xAB
#define REG_GFLVL     0xAE
#define REG_GSTATUS   0xAF
#define REG_GFIFO_U   0xFC
#define REG_GFIFO_D   0xFD
#define REG_GFIFO_L   0xFE
#define REG_GFIFO_R   0xFF

#define GESTURE_SENSITIVITY_1 30   // swipe detection
#define GESTURE_SENSITIVITY_2 25   // near/far detection

// Gesture deltas
int gesture_ud_delta_ = 0;
int gesture_lr_delta_ = 0;

// Direction counts
int gesture_ud_count_ = 0;
int gesture_lr_count_ = 0;

// Near/Far counts
int gesture_near_count_ = 0;
int gesture_far_count_ = 0;

// Gesture state
int gesture_state_ = 0;

// States
#define NEAR_STATE 1
#define FAR_STATE  2

#define MAX_SAMPLES 32

uint8_t u_data[MAX_SAMPLES];
uint8_t d_data[MAX_SAMPLES];
uint8_t l_data[MAX_SAMPLES];
uint8_t r_data[MAX_SAMPLES];

int samples = 0;

#define NEAR_ZERO 20
#define NEAR_COUNT_LIMIT 6
#define FAR_COUNT_LIMIT 2

static int near_count = 0;
static int far_count = 0;


void setup() {
  Serial.begin(115200);
  Wire.begin(21,22);
  bleKeyboard.begin();

  Serial.println(F("\n🔍 APDS-9960 GESTURE DIAGNOSTIC"));
  Serial.println(F("================================="));
  
  // Read ID
  uint8_t id = readRegister(REG_ID);
  Serial.print(F("Device ID: 0x"));
  Serial.println(id, HEX);
  
  if (id != 0xAB && id != 0x9E) {
    Serial.println(F("⚠️  Unknown device!"));
  }
  
  // Power on
  writeRegister(REG_ENABLE, 0x01);
  delay(10);
  
  // Configure for gesture
  Serial.println(F("\n📊 CONFIGURING GESTURE ENGINE..."));
  
  // Set pulse count and length
  writeRegister(REG_GPULSE, 0xC9);  // 16 pulses, 16μs length
  
  // Set gain and LED
  writeRegister(REG_GCONF2, 0x69);  // Gain=4x, LED=100mA, Wait=2.78ms
  
  // Set thresholds
  writeRegister(REG_GPENTH, 30);    // Entry threshold
  writeRegister(REG_GEXTH,20);     // Exit threshold
  
  // Enable gesture engine
  writeRegister(REG_GCONF4, 0x01);  // GMODE=1
  writeRegister(REG_ENABLE, 0x4D);  // PON + GEN + WEN
  
  // Read back config to verify
  Serial.print(F("GCONF2: 0x"));
  Serial.println(readRegister(REG_GCONF2), HEX);
  Serial.print(F("GPULSE: 0x"));
  Serial.println(readRegister(REG_GPULSE), HEX);
  Serial.print(F("GPENTH: "));
  Serial.println(readRegister(REG_GPENTH));
  Serial.print(F("GEXTH: "));
  Serial.println(readRegister(REG_GEXTH));


  
  
}

void loop() {

  int val = analogRead(MIC_PIN);
  unsigned long now = millis();
  //Serial.print("Mic: "); Serial.println(val);
  
 if (val > threshold) {
    if (now - lastClap > 250) {
      if (clapIndex < 3) {
        clapTimes[clapIndex] = now;
        clapIndex++;
        Serial.print("Clap ");
        Serial.println(clapIndex);

      lastClap = millis();}
      lastClapTime=millis();
    
  }
}

  // --- Reset if too slow ---
if (clapIndex > 0 && (now - lastClapTime > 4000)) {
  clapIndex = 0;
  Serial.println("Reset (timeout)");
}
  // --- When 3 claps detected ---
  if (clapIndex == 3) {

    unsigned long gap1 = clapTimes[1] - clapTimes[0];
    unsigned long gap2 = clapTimes[2] - clapTimes[1];

    Serial.print("Gap1: "); Serial.println(gap1);
    Serial.print("Gap2: "); Serial.println(gap2);

    int pause = 1000;

    if (gap1 < pause && gap2 > pause) {
      unlock = true;
       Serial.println("Unlocked");
    
    } else if (gap1 > pause && gap2 < pause) {
      unlock = false;
      Serial.println("Locked");
   
    }  else {
      Serial.println("Invalid pattern");
    }
    clapIndex = 0; // reset for next detection
  }
  lastVal = val;
  delay(2);
    
    
  // ===== READ FIFO =====
uint8_t gstatus = readRegister(REG_GSTATUS);
if (!(gstatus & 0x01)) return;

uint8_t gflvl = readRegister(REG_GFLVL);
if (gflvl < 4) return;

samples = 0;

for (int i = 0; i < gflvl && i < MAX_SAMPLES; i++) {
  u_data[i] = readRegister(REG_GFIFO_U);
  d_data[i] = readRegister(REG_GFIFO_D);
  l_data[i] = readRegister(REG_GFIFO_L);
  r_data[i] = readRegister(REG_GFIFO_R);
  samples++;
}

// ===== FIND FIRST/LAST VALID =====
int first = -1, last = -1;

for (int i = 0; i < samples; i++) {
  if (u_data[i] > 20 && d_data[i] > 20 &&
      l_data[i] > 20 && r_data[i] > 20) {
    first = i;
    break;
  }
}

for (int i = samples - 1; i >= 0; i--) {
  if (u_data[i] > 20 && d_data[i] > 20 &&
      l_data[i] > 20 && r_data[i] > 20) {
    last = i;
    break;
  }
}

if (first == -1 || last == -1) return;

// ===== RATIO CALC =====
/*int ud_first = ((u_data[first] - d_data[first]) * 100) /
               (u_data[first] + d_data[first]);

int lr_first = ((l_data[first] - r_data[first]) * 100) /
               (l_data[first] + r_data[first]);

int ud_last = ((u_data[last] - d_data[last]) * 100) /
              (u_data[last] + d_data[last]);

int lr_last = ((l_data[last] - r_data[last]) * 100) /
              (l_data[last] + r_data[last]);*/


int ud_first = (u_data[first]+d_data[first]) > 0 ?
    ((u_data[first]-d_data[first])*100)/(u_data[first]+d_data[first]) : 0;

int lr_first = (l_data[first]+r_data[first]) > 0 ?
    ((l_data[first]-r_data[first])*100)/(l_data[first]+r_data[first]) : 0;

int ud_last = (u_data[last]+d_data[last]) > 0 ?
    ((u_data[last]-d_data[last])*100)/(u_data[last]+d_data[last]) : 0;

int lr_last = (l_data[last]+r_data[last]) > 0 ?
    ((l_data[last]-r_data[last])*100)/(l_data[last]+r_data[last]) : 0;



int ud_delta = ud_last - ud_first;
int lr_delta = lr_last - lr_first;

// ACCUMULATE
gesture_ud_delta_ += ud_delta;
gesture_lr_delta_ += lr_delta;

// Fix: reset deltas whenever the device is locked

if (!unlock) {
    gesture_ud_delta_ = 0;
    gesture_lr_delta_ = 0;
    return;
}

String gesture = "NONE";

//LEFT / RIGHT 
if (gesture_lr_delta_ > 40)
  gesture = "LEFT";

else if (gesture_lr_delta_ < -40)
  gesture = "RIGHT";

// NEAR / FAR 
if (u_data[last] >= 250 || d_data[last] >= 250 ||
    l_data[last] >= 250 || r_data[last] >= 250) {
  gesture_near_count_ = 0;   // saturated — don't count
  gesture_far_count_ = 0;

} else if (abs(ud_delta) < 20 && abs(lr_delta) < 20) {

 if (abs(ud_delta) <= 2 && abs(lr_delta) <= 2)
    gesture_near_count_++;   // near = very small delta (hand coming straight in)
else
    gesture_far_count_++;    // far  = slightly uneven (hand going aw

  if (gesture_near_count_ >= 5 && gesture_far_count_ >= 2) {

    if (ud_delta == 0 && lr_delta == 0)
      gesture = "NEAR";
    else
      gesture = "FAR";

    gesture_near_count_ = 0;
    gesture_far_count_ = 0;
  }
}


if (gesture != "NONE" && unlock) {

  

  if (millis() - lastGestureTime < GESTURE_COOLDOWN) {
    gesture_ud_delta_ = 0;
    gesture_lr_delta_ = 0;
    return;
  }
  lastGestureTime = millis();  // ← stamp the time

  Serial.println(gesture);

  // RESET 
  gesture_ud_delta_ = 0;
  gesture_lr_delta_ = 0;
  gesture_near_count_ = 0;
  gesture_far_count_ = 0;

  uint8_t flush = readRegister(REG_GFLVL);
for (int i = 0; i < flush; i++) {
  readRegister(REG_GFIFO_U);
  readRegister(REG_GFIFO_D);
  readRegister(REG_GFIFO_L);
  readRegister(REG_GFIFO_R);
}

  if (bleKeyboard.isConnected()) {

    if (gesture == "RIGHT")
      bleKeyboard.write(KEY_RIGHT_ARROW);

    if (gesture == "LEFT")
      bleKeyboard.write(KEY_LEFT_ARROW);

    if (gesture == "NEAR") {
      bleKeyboard.press(KEY_LEFT_GUI);
      bleKeyboard.press('+');
      delay(50);
      bleKeyboard.releaseAll();
    }

    if (gesture == "FAR") {
      bleKeyboard.press(KEY_LEFT_GUI);
      bleKeyboard.press('-');
      delay(50);
      bleKeyboard.releaseAll();
    }
  }
}
  delay(5);
}
    


