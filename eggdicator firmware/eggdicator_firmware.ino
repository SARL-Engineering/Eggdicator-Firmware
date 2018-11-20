/* Eggdicator control software
   Author: Aaron Rito
   Date 3/6/18
   Update 11/20/18
*/
#include <ArduinoJson.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <String.h>

const int PUMP_1 = 9;
const int PUMP_2 = 10;
const int STEP = 7;
const int M0 = 6;
const int M1 = 11;
const int M2 = 12;
const int MOTOR_EN = 5;
const int MOTOR_DIR = 8;
const int VIEW_CLAMP = 4;
const int THREEWAY_CLAMP = 13;
const int CLAMP_3 = A0;
const int CLAMP_4 = A1;
const int LED_PIN = A2;
const int INT_INPUT_0 = 2;
const int INT_INPUT_1 = 3;
const int ANA_INPUT_3 = A3;
const int ANA_INPUT_4 = A4;
const int ANA_INPUT_5 = A5;
int command = 0;
int DEBUG = 0;

//settings
struct conf
{
  int pump_1_power;
  int pump_1_idle;
  int pump_2_power;
  int pump_2_idle;
  int motor_dir;
  int motor_speed;
  int motor_ramp;
  int motor_angle;
};
struct conf confs;
int CONF_SIZE = 8;

//LED stuff
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define NUM_LEDS 12
#define BRIGHTNESS 100
int led_color[3];
CRGB leds[NUM_LEDS];

//Json
char read_buffer[200];
void(* resetFunc) (void) = 0;

void setup() {
  //setting up the PWM
  sei();                    // Enable global interrupts
  TCCR1A = 0;           // undo the configuration done by...
  TCCR1B = 0;           // ...the Arduino core library
  TCNT1  = 0;           // reset timer
  TCCR1A = _BV(COM1A1)  // non-inverted PWM on ch. A
           | _BV(COM1B1)  // same on ch; B
           | _BV(WGM11);  // mode 10: ph. correct PWM, TOP = ICR1
  TCCR1B = _BV(WGM13)   // ditto
           | _BV(CS10);   // prescaler = 1
  ICR1   = 320;         // TOP = 320

  pinMode(STEP, OUTPUT);
  pinMode(PUMP_1, OUTPUT);
  pinMode(PUMP_2, OUTPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_DIR, OUTPUT);
  pinMode(VIEW_CLAMP, OUTPUT);
  pinMode(THREEWAY_CLAMP, OUTPUT);
  pinMode(CLAMP_3, OUTPUT);
  pinMode(CLAMP_4, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(INT_INPUT_0, INPUT);
  pinMode(INT_INPUT_1, INPUT);
  pinMode(ANA_INPUT_3, INPUT);
  pinMode(ANA_INPUT_4, INPUT);
  pinMode(ANA_INPUT_5, INPUT);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  //INIT SETTINGS
  FastLED.showColor(CRGB::Black);
  Serial.begin(57600);
  Serial.flush();
  download_config();
  analogWrite25k(PUMP_1, confs.pump_1_idle);
  analogWrite25k(PUMP_2, confs.pump_2_idle);
  digitalWrite(STEP, LOW);
  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
  digitalWrite(MOTOR_EN, LOW);
  digitalWrite(MOTOR_DIR, LOW);
  digitalWrite(VIEW_CLAMP, LOW);
  digitalWrite(THREEWAY_CLAMP, LOW);
  digitalWrite(CLAMP_3, LOW);
  digitalWrite(CLAMP_4, LOW);
  Serial.println("v: "); //waiting for command
}

void loop() {
  // 1 = motor on
  // 2 = motor off
  // 3 = threeway clamp on (GOOD PATH)
  // 4 = threeway clamp off (TRASH PATH)
  // 5 = viewing clamp on (OPEN)
  // 6 = viewing clamp off (CLOSED)
  // 7 = clamp 3 on (OPEN)
  // 8 = clamp 3 off (CLOSED)
  // 9 = clamp 4 on (OPEN)
  // 10 = clamp 4 off (CLOSED)
  // 11 = pump 1 on
  // 12 = pump 1 off
  // 13 = pump 2 on
  // 14 = pump 2 off
  // 15 = flash on
  // 16 = flash off
  // 17 = reset
  // 18 = send configs
  // 19 = update configs
  // x: = download configs SEND ONLY
  // v: = waiting for command SEND ONLY
  if (Serial.available())
  {
    Serial.readBytesUntil('\n', read_buffer, 200);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(read_buffer);
    if (root.success())
    {
      JsonArray& command_list = root["commands"];
      for (int i = 0; i < sizeof(command_list) / sizeof(command_list[0]); i++)
      {
        command = command_list[i];
        switch (command) {
          case 1:
            Serial.println("motor on received");
            //motor stuff
            break;
          case 2:
            Serial.println("motor off received");
            //motor stuff
            break;
          case 3:
            Serial.println("Threeway clamp on received (GOOD PATH)");
            digitalWrite(THREEWAY_CLAMP, HIGH);
            break;
          case 4:
            Serial.println("Threeway clamp off received (TRASH PATH)");
            digitalWrite(THREEWAY_CLAMP, LOW);
            break;
          case 5:
            Serial.println("Viewing clamp on received (OPEN)");
            digitalWrite(VIEW_CLAMP, HIGH);
            break;
          case 6:
            Serial.println("Viewing clamp off received (CLOSED)");
            digitalWrite(VIEW_CLAMP, LOW);
            break;
          case 7:
            Serial.println("clamp 3 on received (OPEN)");
            digitalWrite(VIEW_CLAMP, HIGH);
            break;
          case 8:
            Serial.println("clamp 3 off received (CLOSED)");
            digitalWrite(VIEW_CLAMP, LOW);
            break;
          case 9:
            Serial.println("clamp 4 on received (OPEN)");
            digitalWrite(VIEW_CLAMP, HIGH);
            break;
          case 10:
            Serial.println("clamp 4 off received (CLOSED)");
            digitalWrite(VIEW_CLAMP, LOW);
            break;
          case 11:
            Serial.println("pump 1 on received");
            analogWrite25k(PUMP_1, confs.pump_1_power);
            break;
          case 12:
            Serial.println("pump 1 off received");
            analogWrite25k(PUMP_1, confs.pump_1_idle);
            break;
          case 13:
            Serial.println("pump 2 on received");
            analogWrite25k(PUMP_2, confs.pump_2_power);
            break;
          case 14:
            Serial.println("pump 2 off received");
            analogWrite25k(PUMP_2, confs.pump_2_idle);
            break;
          case 15:
            Serial.println("Flash on received");
            FastLED.showColor(CHSV(led_color[0], led_color[1], led_color[2]));
            break;
          case 16:
            Serial.println("Flash off received");
            FastLED.showColor(CRGB::Black);
            break;
          case 17:
            Serial.println("Reset received");
            resetFunc();
            break;
          case 18:
            Serial.println("Send configs received");
            //add call to json send function
            break;
          case 19:
            Serial.println("Update configs received");
            download_config();
            break;
          default:
            Serial.println("invalid command");
            break;
        }
        Serial.flush();
        Serial.println("v: ");
      }
    }
  }
}
void analogWrite25k(int pin, int value){
  
  switch (pin) {
    case PUMP_1:
      OCR1A = value;
      break;
    case PUMP_2:
      OCR1B = value;
      break;
    default:
      break;
  }
}
