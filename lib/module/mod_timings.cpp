/*===========================================================================*/
/// \file mod_timings.cpp
///
/// \brief
///    Module for handling Bluetooth communication to the device
///
/// \details
///    Handle the connection and the data transfer between the device and the mobile application
///
/// \author
///    Ayoub Q.
///
/*===========================================================================*/

/*=============================================================================
                                     Includes
=============================================================================*/

#include <mod_timings.h>
#include <BLEDevice.h>

/*=============================================================================
                                     Defines
=============================================================================*/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

/* Check if Bluetooth configurations are enabled in the SDK */
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define NUMBER_OF_DAYS_HEADER 0x69
#define PRAYER_TIMINGS_HEADER 0x42
#define END_OF_TIMINGS_HEADER 0x88
#define CURRENT_TIME_HEADER 0x20

/*=============================================================================
                                     Macros
=============================================================================*/

/*=============================================================================
                                 Type definitions
=============================================================================*/

/*=============================================================================
                                    Structures
=============================================================================*/

/*=============================================================================
                            Private Function Prototypes
=============================================================================*/

/*=============================================================================
                                Private Constants
=============================================================================*/

static const PrayerTimings nullPrayerTimings = {
    {0, 0, NONE},
    {0, 0, NONE},
    {0, 0, NONE},
    {0, 0, NONE},
    {0, 0, NONE},
    "0/0/0"
};

/*=============================================================================
                                Private Variables
=============================================================================*/

static bool deviceConnected = false;
static PrayerTimings timings[MAX_DAYS] = {nullPrayerTimings};
static bool finishedReceiving = false;
static modBTEParams_t *modBTEParams;
static QueueHandle_t timingsQueue;
static uint8_t currentDay = 0;
static SemaphoreHandle_t prayerSemaphore;

/*=============================================================================
                                Class Definitions
=============================================================================*/

class ConnectionCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) override {
        Serial.println("Device connected");
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer) override {
        Serial.println("Device disconnected");
        deviceConnected = false;
        BLEDevice::startAdvertising();
    }
};

class OperationCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        uint8_t *rxValue = pCharacteristic->getData();
        uint8_t header = rxValue[0];
        if (header == NUMBER_OF_DAYS_HEADER) {
            finishedReceiving = false;
            int numberOfDays = rxValue[1];
            if (numberOfDays != MAX_DAYS) {
                timings[MAX_DAYS - 1] = nullPrayerTimings;
            }
        } else if (header == PRAYER_TIMINGS_HEADER) {
            /*
            Packet representation:
            [0] - Header
            [1] - Day
            [2] - Month
            [3] - Year pt 1
            [4] - Year pt 2
            [5] - Fajr Hour
            [6] - Fajr Minute
            [7] - Dhuhr Hour
            [8] - Dhuhr Minute
            [9] - Asr Hour
            [10] - Asr Minute
            [11] - Maghrib Hour
            [12] - Maghrib Minute
            [13] - Isha Hour
            [14] - Isha Minute
            */
            uint8_t day = rxValue[1] - 1;
            timings[day].date =
                    String(rxValue[1]) + "/" + String(rxValue[2]) + "/" + String(rxValue[3]) + String(rxValue[4]);
            timings[day].fajr.hour = rxValue[5];
            timings[day].fajr.minute = rxValue[6];
            timings[day].fajr.name = FAJR;
            timings[day].dhuhr.hour = rxValue[7];
            timings[day].dhuhr.minute = rxValue[8];
            timings[day].dhuhr.name = DHUHR;
            timings[day].asr.hour = rxValue[9];
            timings[day].asr.minute = rxValue[10];
            timings[day].asr.name = ASR;
            timings[day].maghrib.hour = rxValue[11];
            timings[day].maghrib.minute = rxValue[12];
            timings[day].maghrib.name = MAGHRIB;
            timings[day].isha.hour = rxValue[13];
            timings[day].isha.minute = rxValue[14];
            timings[day].isha.name = ISHA;
        } else if (header == END_OF_TIMINGS_HEADER) {
            finishedReceiving = true;
        } else if (header == CURRENT_TIME_HEADER) {
            uint8_t year1 = rxValue[6]; // year first 2 digits
            uint8_t year2 = rxValue[7]; // year last 2 digits
            uint16_t year = year1 * 100 + year2;

            tm time;
            time.tm_hour = rxValue[1];
            time.tm_min = rxValue[2];
            time.tm_sec = rxValue[3];
            time.tm_mday = rxValue[4];
            time.tm_mon = rxValue[5];
            time.tm_year = year - 1900;
            time_t t = mktime(&time);
            struct timeval tv = {t, 0};
            settimeofday(&tv, nullptr);

            currentDay = rxValue[4];
        }
    }
};

/*=============================================================================
                                Library Entry Point
=============================================================================*/

/*=============================================================================
                                Public Functions
=============================================================================*/

_Noreturn void modBTETaskProcess(void *pvParameters) {
    modBTEParams = (modBTEParams_t *) pvParameters;
    timingsQueue = modBTEParams->timingsQueue;
    prayerSemaphore = modBTEParams->prayerSemaphore;

    BLEDevice::init("PrayerDisplayer");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE);
    pServer->setCallbacks(new ConnectionCallbacks());
    pCharacteristic->setCallbacks(new OperationCallbacks());
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    while (true) {
        if (finishedReceiving) {
            int remainingDays = MAX_DAYS - currentDay;
            for (int i = -1; i < remainingDays; i++) {
                if (timings[currentDay + i].date == "0/0/0") {
                    break;
                }
                xQueueSend(timingsQueue, (void *) &timings[currentDay + i], (TickType_t) 5);
                Serial.printf("Sending %s\n", timings[currentDay + i].date.c_str());
            }
            // Send a null prayer timings to indicate that the timings of the month have been sent
            if (timings[MAX_DAYS - 1].date == "0/0/0") {
                xQueueSend(timingsQueue, (void *) &nullPrayerTimings, (TickType_t) 5);
            }

            finishedReceiving = false;
            xSemaphoreTake(prayerSemaphore, portMAX_DELAY);
            Serial.println("Finished sending prayer timings, waiting for next month");
        }
        vTaskDelay(1000);
    }
}

/*=============================================================================
                                Private Functions
=============================================================================*/
