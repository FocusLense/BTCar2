#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

volatile int g_steering = 0;
volatile int g_throttle = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      portENTER_CRITICAL(&mux)
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        
        String data = String(rxValue.c_str());
        int commaIndex = data.indexOf(',');
        if(commaIndex  != -1)
        {
          g_steering = data.substring(0,commaIndex).toInt();
          g_throttle = data.substring(commaIndex+1).toInt();
          Serial.print("\nSteering: ");
          Serial.print(g_steering);
          Serial.print("\nThrottle: ");
          Serial.print(g_throttle);
          int steeringPWM = map(g_steering,-100,100,1638,8192);
          Serial.print("\nSteeringPWM: ");
          Serial.print(steeringPWM);
        }
      }
      portEXIT_CRITICAL(&mux);
    }
};
// Pins
#define SERVO_PIN 21  
#define MOTOR_IN1 6  
#define MOTOR_IN2 7  
#define DRIVER_SLEEP_PIN 4

// PWM Kanäle
#define SERVO_CH 3
#define MOTOR_CH1 0
#define MOTOR_CH2 1

//
void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("BLE CAR");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a connection");


  //Activate driver: 
  pinMode(DRIVER_SLEEP_PIN, OUTPUT);
  digitalWrite(DRIVER_SLEEP_PIN, HIGH);  // Treiber aktivieren

  //motor/servo setup// PWM Setup für Motor
  ledcSetup(MOTOR_CH1, 10000, 8); // 10 kHz, 8 Bit
  ledcSetup(MOTOR_CH2, 10000, 8);
  ledcAttachPin(MOTOR_IN1, MOTOR_CH1);
  ledcAttachPin(MOTOR_IN2, MOTOR_CH2);

  // PWM Setup für Servo (50Hz, 16Bit)
  ledcSetup(SERVO_CH, 50, 12);
  ledcAttachPin(SERVO_PIN, SERVO_CH);
  pinMode(9, OUTPUT);
  digitalWrite(9,LOW);
}


void loop() {
  portENTER_CRITICAL(&mux);
  int steering = g_steering;
  int throttle = g_throttle;
  portEXIT_CRITICAL(&mux);

  
  ledcWrite(SERVO_CH,map(steering,-100,100,205,410));

  if(throttle >= 0){
    ledcWrite(MOTOR_CH1,map(throttle,0,100,0,255));
    ledcWrite(MOTOR_CH2,0);
  }
  else{
    ledcWrite(MOTOR_CH2,map(-throttle,0,100,0,255));
    ledcWrite(MOTOR_CH1,0);
  }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
		// do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
