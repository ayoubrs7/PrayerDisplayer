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
#define INCLUDE_xTaskGetHandle 1
#define INCLUDE_vTaskGetInfo 1
#include <Arduino.h>
#include "Wire.h"
#include <mod_timings.h>
#include <mod_prayer.h>
#include <svc_display.h>
#include <mod_cli0.h>
#include <svc_cli.h>
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

void appMainRegisterCommands();

void commandTasks(cmd *c);

/*=============================================================================
                                Private Variables
=============================================================================*/

// Task Handles
static TaskHandle_t modCliTaskHandle = nullptr;
static TaskHandle_t modBTETaskHandle = nullptr;
static TaskHandle_t modPrayerTaskHandle = nullptr;

// Task Parameters
static modBTEParams_t modBTEParams;
static modPrayerParams_t modPrayerParams;

static QueueHandle_t timingsQueue;

static SemaphoreHandle_t prayerSemaphore;

/*=============================================================================
                                Private Constants
=============================================================================*/

static const TaskParameters_t modBTETaskParams = {
    modBTETaskProcess,
    "modTimingsTask",
    8192,
    (void *) &modBTEParams,
    5
};

static const TaskParameters_t modPrayerTaskParams = {
    modPrayerTaskProcess,
    "modPrayerTask",
    8192,
    (void *) &modPrayerParams,
    5
};

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
    appMainRegisterCommands();
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

#ifndef ARDUINO
[[noreturn]] int main() {
    setup();
    while (true) {
        loop();
    }
    return 0;
}
#endif

/*=============================================================================
                                Private Functions
=============================================================================*/

void appMainRegisterCommands() {
    SimpleCLI *cli = svcCliGetCli0();
    // Register the commands
    svcCliAddCmdHelp("tasks", "List all tasks");
    cli->addBoundlessCommand("tasks", commandTasks);
}

void commandTasks(cmd *c) {
    Command cmd(c);

    const TaskHandle_t taskHandleList[] = {
        modCliTaskHandle,
        modBTETaskHandle,
        modPrayerTaskHandle
    };

    Serial.write("\r\n--------------------------------------------------------------------------\r\n");
    Serial.printf("%-15s | %-8s | %-10s | %-15s | %-10s\r\n", "Task Name", "State", "Priority",
                  "High Watermark", "Handle");
    Serial.printf("%-15s | %-8s | %-10s | %-15s | %-10s\r\n", "", "0-5", "",
                  "(bytes)", "");
    Serial.printf("--------------------------------------------------------------------------\r\n");

    for (int i = 0; i < sizeof(taskHandleList) / sizeof(taskHandleList[0]); i++) {
        Serial.printf("%-15s | %-8d | %-10d | %-15d | %-10p\r\n",
                      pcTaskGetName(taskHandleList[i]),
                      eTaskGetState(taskHandleList[i]),
                      uxTaskPriorityGet(taskHandleList[i]),
                      uxTaskGetStackHighWaterMark(taskHandleList[i]),
                      taskHandleList[i]);
    }
    Serial.write("--------------------------------------------------------------------------\r\n");
}

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
