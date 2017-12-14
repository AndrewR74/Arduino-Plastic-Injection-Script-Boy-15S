// Machine Selenoids (Shift Register)
const int OPEN_MOLD = 0,
          CLOSE_MOLD = 1,
          OPEN_BARREL = 2,
          CLOSE_BARREL = 3,
          OPEN_PLUNGER = 4,
          CLOSE_PLUNGER = 5,
          OPEN_PUSHER = 6,
          OPERATE_AUGER = 7;

// Machine Cams
const int CAM_MOLD_OPEN = A0,
          CAM_MOLD_CLOSE = A1,
          CAM_BARREL_OPEN = A2,
          CAM_BARREL_CLOSE = A3,
          CAM_PLUNGER_OPEN = A4,
          CAM_PLUNGER_CLOSE = A5;

/// Ardino main control buttons
const int BTN_MANUAL_MODE = 7,
          BTN_SEMI_MODE = 8,
          BTN_STOP_ALL = 2,
          BTN_CYCLE_START = 10,
          BTN_TOGGLE_SETTING_MODE = 11;

/// Ardino setting mode control buttons
const int BTN_SETTING_UP = 7,
          BTN_SETTING_DOWN = 8,
          BTN_SETTING_VALUE_UP = 9,
          BTN_SETTING_VALUE_DOWN = 10;

/// Setting LED correspondance (Shift Register)
const int SETTING_AFTER_CLOSE_MOLD_DELAY = 8,
          SETTING_HEAT_MATERIAL_TIME = 9,
          SETTING_HOLD_PRESSURE_TIME = 10,
          SETTING_COOL_TIME = 11,
          SETTING_FEED_MATERIAL_TIME = 12,
          SETTING_AFTER_OPEN_BARREL_DELAY = 13,
          SETTING_AFTER_OPEN_MOLD_DELAY = 14,
          SETTING_PUSH_TIME = 15;

//Pin connected to ST_CP of 74HC595
int latchPin = 5;
//Pin connected to SH_CP of 74HC595
int clockPin = 6;
////Pin connected to DS of 74HC595
int dataPin = 4;

int oePin = 13, mrPin = 12;


// Variables
boolean registers[16] = {
  0,0,0,0,
  0,0,0,0,
  0,0,0,0,
  0,0,0,0};
unsigned long previousMillis_stop_all = 0, 
              previousMillis_toggle_setting = 0,
              previousMillis_mold_closed = 0,        // Delay after mold closed
              previousMillis_barrel_closed = 0,      // Delay after barrel closed
              previousMillis_plunger_closed  = 0,    // Delay after plunger closed
              previousMillis_feed_material = 0,      // Delay until material reloaded
              previousMillis_cool_time    = 0,       // Delay until piece solidifies
              previousMillis_barrel_opened   = 0,    // Delay after barrel opened
              previousMillis_mold_opened    = 0,     // Delay after mold opened
              previousMillis_pushing     = 0;        // Time to push piece

// Milliseconds program delays
const int mold_close_delay = 1000;
const int barrel_close_delay = 1000;
const int plunger_close_delay = 1000;
const int feed_material_time = 1000;
const int cool_time = 1000;
const int barrel_open_delay = 1000;
const int mold_open_delay = 1000;
const int pushing_time = 1000;

bool _settingMode = false;
bool _isInCycle = false;
int _cycleStep = 0;

const long debounce = 300;

void setup() {
  Serial.begin(9600);
  
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);      
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(oePin, OUTPUT);
  pinMode(mrPin, OUTPUT);

  pinMode(BTN_STOP_ALL, INPUT);
  pinMode(BTN_CYCLE_START, INPUT);
  pinMode(BTN_MANUAL_MODE, INPUT);
  pinMode(BTN_SEMI_MODE, INPUT);
  pinMode(BTN_TOGGLE_SETTING_MODE, INPUT);
  
  digitalWrite(latchPin, LOW);
  digitalWrite(mrPin, LOW); // Clear Output
    delay(50);
  digitalWrite(mrPin, HIGH); // Clear Output
  digitalWrite(latchPin, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:



  int _BTN_STOP_ALL = digitalRead(BTN_STOP_ALL);
  int _BTN_TOGGLE_SETTING_MODE = digitalRead(BTN_TOGGLE_SETTING_MODE);
  int _BTN_CYCLE_START = digitalRead(BTN_CYCLE_START);

  if(_BTN_STOP_ALL == HIGH) {
    _stopAll();
    Serial.println("Stop All");
  } else {

    if(_BTN_TOGGLE_SETTING_MODE == HIGH) {
      _toggleSettingMode();
      Serial.println("Settings Mode");
    } else {
      int _runMode = _getRunMode();
      //Serial.println(_runMode);
      if(_runMode == 1)
      {
        // Manual Mode
      } else {
        if(_runMode == 3)
          _isInCycle = true;
  
        if(_runMode == 2 && _BTN_CYCLE_START == HIGH && !_isInCycle)
          _isInCycle = true;


        if(_isInCycle) {
          Serial.println(_cycleStep);
          if(_cycleStep == 0) { // Close Mold
            if(_isCamPressed(CAM_MOLD_OPEN)) {
              _clearRegister();
              registers[CLOSE_MOLD] = true;
              WriteReg();
            } else if(_isCamPressed(CAM_MOLD_CLOSE)) {
              if(previousMillis_mold_closed == 0)
                previousMillis_mold_closed = millis();
                
              if (millis() - previousMillis_mold_closed >= mold_close_delay) {
                _cycleStep = _cycleStep + 1;
                previousMillis_mold_closed = 0;
              }
                
              _clearRegister();
              WriteReg();
            }
          } else if(_cycleStep == 1) { // Close Barrel
            if(_isCamPressed(OPEN_BARREL)) {
              _clearRegister();
              registers[CLOSE_BARREL] = true;
              WriteReg();
            } else if(_isCamPressed(CAM_BARREL_CLOSE)) {
              if(previousMillis_barrel_closed == 0)
                previousMillis_barrel_closed = millis();
                
              if (millis() - previousMillis_barrel_closed >= barrel_close_delay) {
                _cycleStep = _cycleStep + 1;
                previousMillis_barrel_closed = 0;
              }
              _clearRegister();
              WriteReg();
            }
          } else if(_cycleStep == 2) { // Close Plunger
            if(_isCamPressed(CAM_PLUNGER_OPEN)) {
              _clearRegister();
              registers[CLOSE_PLUNGER] = true;
              WriteReg();
            } else if(_isCamPressed(CAM_PLUNGER_CLOSE)) {
              if(previousMillis_plunger_closed == 0)
                previousMillis_plunger_closed = millis();
                
              if (millis() - previousMillis_plunger_closed >= plunger_close_delay) {
                _cycleStep = _cycleStep + 1;
                previousMillis_plunger_closed = 0;
              }
              _clearRegister();
              WriteReg();
            }
          } else if(_cycleStep == 3) { // Open Plunger
            if(_isCamPressed(CAM_PLUNGER_CLOSE)) {
              _clearRegister();
              registers[OPEN_PLUNGER] = true;
              registers[OPERATE_AUGER] = true; // Begin Feeding Material
              WriteReg();
            } else if(_isCamPressed(CAM_PLUNGER_OPEN)) {            
              _cycleStep = _cycleStep + 1;
              previousMillis_feed_material = millis(); // Record how long we have been feeding material
              _clearRegister();
              registers[OPERATE_AUGER] = true; // Continue Feeding Material
              WriteReg();
            }
          } 
          else if(_cycleStep == 4) { // Feed Material & Cool Time
              unsigned long _currentMilli = millis();
              
              if(previousMillis_cool_time == 0)
                previousMillis_cool_time = _currentMilli;
                
              if (_currentMilli - previousMillis_cool_time >= cool_time && _currentMilli - previousMillis_feed_material >= feed_material_time) {
                _cycleStep = _cycleStep + 1;
                previousMillis_cool_time = 0;
              }

              if(_currentMilli - previousMillis_feed_material >= feed_material_time) {
                registers[OPERATE_AUGER] = false;
                WriteReg();
              }
             
          } else if(_cycleStep == 5) { // Open Barrel
            if(_isCamPressed(OPEN_BARREL)) {
              if(previousMillis_barrel_opened == 0)
                previousMillis_barrel_opened = millis();
                
              if (millis() - previousMillis_barrel_opened >= barrel_open_delay) {
                _cycleStep = _cycleStep + 1;
                previousMillis_barrel_opened = 0;
              }
              
              _clearRegister();
              WriteReg();
            } else if(_isCamPressed(CAM_BARREL_CLOSE)) {
              _clearRegister();
              registers[OPEN_BARREL] = true;
              WriteReg();
            }
          } else if(_cycleStep == 6) { 
            if(_isCamPressed(CAM_MOLD_OPEN)) {
              if(previousMillis_mold_opened == 0)
                previousMillis_mold_opened = millis();
                
              if (millis() - previousMillis_mold_opened >= mold_open_delay) {
                _cycleStep = _cycleStep + 1;
                previousMillis_mold_opened = 0;
              }
              _clearRegister();
              WriteReg();
            } else if(_isCamPressed(CAM_MOLD_CLOSE)) {
              _clearRegister();
              registers[OPEN_MOLD] = true;
              WriteReg();
            }
          } else if(_cycleStep == 7) {
            if(previousMillis_pushing == 0)
                previousMillis_pushing = millis();
                
            if (millis() - previousMillis_pushing >= pushing_time) {
              _cycleStep = _cycleStep + 1;
              previousMillis_pushing = 0;
            }
            
            _clearRegister();
            registers[OPEN_PUSHER] = true;
            WriteReg();
          } else if(_cycleStep == 8) {
            _clearRegister();
            WriteReg();
            _cycleStep = 0;
            _isInCycle = false; // End Semi Auto
          }
        } else {
          // Not in cycle yet
          _clearRegister();
        }
      }
    }
  }
}

void _clearRegister() {
  for(int i = 0; i < 16; i++)
    registers[i] = false;
}

void _toggleSettingMode() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis_toggle_setting >= debounce) {
    
    previousMillis_toggle_setting = currentMillis;

    _settingMode = !_settingMode;
  }
}

void _stopAll() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis_stop_all >= debounce) {
    
    previousMillis_stop_all = currentMillis;
    
    digitalWrite(oePin, HIGH); // Disable Output
    
    digitalWrite(latchPin, LOW);
    digitalWrite(mrPin, LOW); // Clear Output
      delay(50);
    digitalWrite(mrPin, HIGH); // Clear Output
    digitalWrite(latchPin, HIGH);

    _isInCycle = false;
    _cycleStep = 0;
  }
}

int _getRunMode() {
  int _BTN_MANUAL_MODE = digitalRead(BTN_MANUAL_MODE);
  int _BTN_SEMI_MODE = digitalRead(BTN_SEMI_MODE);

  if(_BTN_MANUAL_MODE == HIGH && _BTN_SEMI_MODE == HIGH)
    return 3; // Auto Mode

  if(_BTN_SEMI_MODE == HIGH)
    return 2; // Semi

  if(_BTN_MANUAL_MODE == HIGH)
    return 1; // Manual

  return 0; // Settings Mode
}

bool _isCamPressed(int cam) {
  int sensorValue = analogRead(cam);
  return (sensorValue > 700);
}















void WriteReg() {

  // Clear everything
  digitalWrite(dataPin, 0);
  digitalWrite(clockPin, 0);

  digitalWrite(latchPin, LOW);
      
  for(int i = 15; i >= 0; i--) {
    digitalWrite(clockPin, LOW);
    digitalWrite(dataPin, registers[i]);
    digitalWrite(clockPin, HIGH);
    // Prevent Bleed
    digitalWrite(dataPin, 0);
  }
    
  digitalWrite(latchPin, HIGH);
}




