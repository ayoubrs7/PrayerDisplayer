/*===========================================================================*/
/// \file main.cpp
///
/// \brief
///    The main entry point for the application
/// \author
///     Ayoub Q.
///
/*=============================================================================
                                     Includes
=============================================================================*/

#include "Wire.h"
#include <mod_timings.h>
#include <mod_prayer.h>
#include <svc_display.h>
#include <mod_cli0.h>
/*=============================================================================
                                     Defines
=============================================================================*/

/*=============================================================================
                                      Enums
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

bool mainCreateTask(const TaskParameters_t *taskParameters, TaskHandle_t *taskHandle);

/*=============================================================================
                                Private Variables
=============================================================================*/
static TaskHandle_t modCliTaskHandle = nullptr;
static TaskHandle_t modBTETaskHandle = nullptr;
static TaskHandle_t modPrayerTaskHandle = nullptr;
static QueueHandle_t timingsQueue;

static SemaphoreHandle_t prayerSemaphore;

static modBTEParams_t modBTEParams;
static modPrayerParams_t modPrayerParams;

/*=============================================================================
                                Private Constants
=============================================================================*/

static const TaskParameters_t modBTETaskParams = {
        modBTETaskProcess,
        "modTimingsTask",
        8192,
        (void *) &modBTEParams,
        5};

static const TaskParameters_t modPrayerTaskParams = {
        modPrayerTaskProcess,
        "modPrayerTask",
        8192,
        (void *) &modPrayerParams,
        5};

static const TaskParameters_t modCliParameters = {
        modCli0EntryPoint,
        "CLI0",
        3000,
        reinterpret_cast<void *>(1),
        0,
};


/*=============================================================================
                                Public Functions
=============================================================================*/

void setup() {
    Serial.begin(115200);

    prayerSemaphore = xSemaphoreCreateBinary();

    timingsQueue = xQueueCreate(MAX_DAYS, sizeof(PrayerTimings));

    bool status = modCli0Init();
    mainCreateTask(&modCliParameters, &modCliTaskHandle);
    Serial.printf("[%s] CLI service \n", status ? "O" : "X");

    status = svcDisplayInit();
    Serial.printf("[%s] Display service \n", status ? "O" : "X");

    modBTEParams.timingsQueue = timingsQueue;
    modBTEParams.prayerSemaphore = prayerSemaphore;
    status = mainCreateTask(&modBTETaskParams, &modBTETaskHandle);
    Serial.printf("[%s] BTE module \n", status ? "O" : "X");

    modPrayerParams.prayersQueue = timingsQueue;
    modPrayerParams.prayerSemaphore = prayerSemaphore;
    status = mainCreateTask(&modPrayerTaskParams, &modPrayerTaskHandle);
    Serial.printf("[%s] Prayer module \n", status ? "O" : "X");

}

void loop() {
    delay(2000);
}

/*=============================================================================
                                Private Functions
=============================================================================*/
bool mainCreateTask(const TaskParameters_t *taskParameters, TaskHandle_t *taskHandle) {
    const BaseType_t xReturned = xTaskCreate(taskParameters->pvTaskCode,
                                             taskParameters->pcName,
                                             taskParameters->usStackDepth,
                                             taskParameters->pvParameters,
                                             taskParameters->uxPriority,
                                             taskHandle);

    if (xReturned != pdPASS) {
        Serial.printf("Failed to create task %s\n", taskParameters->pcName);
        return false;
    }

    return true;
}
