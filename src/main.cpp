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

volatile uint8_t g_steering = 0;
volatile uint8_t g_throttle = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      portENTER_CRITICAL(&mux)
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        
        String data = String(rxValue.c_str());
        int8_t commaIndex = data.indexOf(',');
        if(commaIndex  != -1)
        {
          g_steering = data.substring(0,commaIndex).toInt()+100;
          g_throttle = data.substring(commaIndex+1).toInt()+100;
          /* Serial.print("\nSteering: ");
          Serial.print(g_steering);
          Serial.print("\nThrottle: ");
          Serial.print(g_throttle); */
          uint16_t steeringPWM = map(g_steering,-100,100,1638,8192);
          /* Serial.print("\nSteeringPWM: ");
          Serial.print(steeringPWM); */
        }
      }
      portEXIT_CRITICAL(&mux);
    }
};

#define SERVO_PIN 21  
#define MOTOR_IN1 6  
#define MOTOR_IN2 7  
#define DRIVER_SLEEP_PIN 4

#define SERVO_CH 3
#define MOTOR_CH1 0
#define MOTOR_CH2 1

void setup() {
  Serial.begin(115200);

  BLEDevice::init("BLE CAR");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  pServer->getAdvertising()->start();
  //Serial.println("Waiting a connection");


  pinMode(DRIVER_SLEEP_PIN, OUTPUT);
  digitalWrite(DRIVER_SLEEP_PIN, HIGH);  

  ledcSetup(MOTOR_CH1, 10000, 8); // 10 kHz, 8 Bit
  ledcSetup(MOTOR_CH2, 10000, 8);
  ledcAttachPin(MOTOR_IN1, MOTOR_CH1);
  ledcAttachPin(MOTOR_IN2, MOTOR_CH2);

  ledcSetup(SERVO_CH, 50, 12);
  ledcAttachPin(SERVO_PIN, SERVO_CH);
  pinMode(9, OUTPUT);
  digitalWrite(9,LOW);
}


void loop() {
  uint8_t steering = g_steering;
  uint8_t throttle = g_throttle;
  
  //give steering command to servo(0 to 200, full left to full right)
  ledcWrite(SERVO_CH,map(steering,-100,100,205,410));

  //throttle command to motor(-100 to 100, full backward to full forward)
  if(throttle >= 0){//forward
    ledcWrite(MOTOR_CH1,map(throttle,0,100,0,255));
    ledcWrite(MOTOR_CH2,0);
  }
  else{//backward
    ledcWrite(MOTOR_CH2,map(-throttle,0,100,0,255));
    ledcWrite(MOTOR_CH1,0);
  }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
		// do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
