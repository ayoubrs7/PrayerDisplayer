/*===========================================================================*/
/// \file mod_prayer.cpp
///
/// \brief
///    Module for managing the prayer timings
///
/// \details
///    Handle the prayer timings and the logic for the prayer times
///
/// \author
///    Ayoub Q.
///
/*===========================================================================*/

/*=============================================================================
                                     Includes
=============================================================================*/

#include <mod_prayer.h>
#include <mod_timings.h>
#include <svc_display.h>
#include <svc_cli.h>

/*=============================================================================
                                     Defines
=============================================================================*/

/*=============================================================================
                                     Macros
=============================================================================*/

/*=============================================================================
                                 Type definitions
=============================================================================*/

/*=============================================================================
                                      Enums
=============================================================================*/
typedef enum {
    MIN_DAY = 1,
    MIN_HOUR = 0,
    MIN_MINUTE = 0,
    MAX_DAY = 31,
    MAX_HOUR = 23,
    MAX_MINUTE = 59
} TimeLimits;
/*=============================================================================
                                    Structures
=============================================================================*/

/*=============================================================================
                            Private Function Prototypes
=============================================================================*/
static void processPrayerTimings();

static void waitUntilNextPrayer();

static void setCurrentTime(cmd *c);

static void printCurrentTime(cmd *c);

static void modPrayerRegisterCommands();

static bool isTimeValid(tm time);

/*=============================================================================
                                Private Variables
=============================================================================*/
static QueueHandle_t prayerQueue;
static SemaphoreHandle_t prayerSemaphore;
static PrayerTimings prayerTimings;
static Prayer nextPrayer;
static bool isNextDayPrayer = false;
/*=============================================================================
                                Private Constants
=============================================================================*/

/*=============================================================================
                                Library Entry Point
=============================================================================*/

/*=============================================================================
                                Public Functions
=============================================================================*/

_Noreturn void modPrayerTaskProcess(void *pvParameters) {
    auto *modPrayerParams = (modPrayerParams_t *) pvParameters;
    prayerQueue = modPrayerParams->prayersQueue;

    modPrayerRegisterCommands();

    while (true) {
        vTaskDelay(1000);
        if (xQueueReceive(prayerQueue, &prayerTimings, (TickType_t) 5)) {
            if (strcmp(prayerTimings.date.c_str(), "0/0/0") == 0) {
                Serial.println("Finished receiving prayer timings");
                xSemaphoreGive(prayerSemaphore);
                continue;
            }
            processPrayerTimings();
        }
    }
}

/*=============================================================================
                                Private Functions
=============================================================================*/

void processPrayerTimings() {
    // Get the current time
    tm currentTime;
    time_t now;
    time(&now);
    localtime_r(&now, &currentTime);

    Serial.printf("Processing prayer timings for %s\n", prayerTimings.date.c_str());

    // Get the next prayer
    if (currentTime.tm_hour < prayerTimings.fajr.hour ||
        (currentTime.tm_hour == prayerTimings.fajr.hour && currentTime.tm_min < prayerTimings.fajr.minute)) {
        nextPrayer = prayerTimings.fajr;
    } else if (currentTime.tm_hour < prayerTimings.dhuhr.hour ||
               (currentTime.tm_hour == prayerTimings.dhuhr.hour && currentTime.tm_min < prayerTimings.dhuhr.minute)) {
        nextPrayer = prayerTimings.dhuhr;
    } else if (currentTime.tm_hour < prayerTimings.asr.hour ||
               (currentTime.tm_hour == prayerTimings.asr.hour && currentTime.tm_min < prayerTimings.asr.minute)) {
        nextPrayer = prayerTimings.asr;
    } else if (currentTime.tm_hour < prayerTimings.maghrib.hour || (currentTime.tm_hour == prayerTimings.maghrib.hour &&
                                                                    currentTime.tm_min <
                                                                    prayerTimings.maghrib.minute)) {
        nextPrayer = prayerTimings.maghrib;
    } else if (currentTime.tm_hour < prayerTimings.isha.hour ||
               (currentTime.tm_hour == prayerTimings.isha.hour && currentTime.tm_min < prayerTimings.isha.minute)) {
        nextPrayer = prayerTimings.isha;
    } else if (isNextDayPrayer) {
        nextPrayer = prayerTimings.fajr;
    } else {
        Serial.println("No more prayers for today");
        isNextDayPrayer = true;
        return;
    }

    Serial.printf("Next prayer at %02d:%02d\n", nextPrayer.hour, nextPrayer.minute);

    svcDisplayNextPrayer(nextPrayer);
    waitUntilNextPrayer();
    processPrayerTimings();
}

void waitUntilNextPrayer() {
    // Get the current time
    tm currentTime;
    time_t now;
    time(&now);
    localtime_r(&now, &currentTime);
    Serial.printf("Current time is %02d:%02d\n", currentTime.tm_hour, currentTime.tm_min);

    tm nextPrayerTime;
    localtime_r(&now, &nextPrayerTime);
    nextPrayerTime.tm_hour = nextPrayer.hour;
    nextPrayerTime.tm_min = nextPrayer.minute;
    if (isNextDayPrayer) {
        nextPrayerTime.tm_mday++;
        isNextDayPrayer = false;
    }

    time_t nextPrayerTimestamp = mktime(&nextPrayerTime);
    time_t currentTimestamp = mktime(&currentTime);

    int delay = difftime(nextPrayerTimestamp, currentTimestamp);
    Serial.printf("Waiting for %d seconds\n", delay);

    while (delay > 0) {
        time(&now);
        localtime_r(&now, &currentTime);
        currentTimestamp = mktime(&currentTime);
        delay = difftime(nextPrayerTimestamp, currentTimestamp);
        vTaskDelay(1000);
    }
}

void setCurrentTime(cmd *c) {
    Command cmd(c);
    if (cmd.countArgs() != 3) {
        Serial.println("Usage: settime <day> <hour> <minute>");
        return;
    }
    tm currentTime;
    time_t now;
    time(&now);
    localtime_r(&now, &currentTime);

    currentTime.tm_mday = cmd.getArgument(0).getValue().toInt();
    currentTime.tm_hour = cmd.getArgument(1).getValue().toInt();
    currentTime.tm_min = cmd.getArgument(2).getValue().toInt();
    if (!isTimeValid(currentTime)) {
        Serial.println("Invalid time");
        return;
    }


    time_t newTime = mktime(&currentTime);
    Serial.printf("Setting time to %02d:%02d\n", currentTime.tm_hour, currentTime.tm_min);
    struct timeval tv = {newTime, 0};
    settimeofday(&tv, nullptr);
}

void printCurrentTime(cmd *c) {
    tm currentTime;
    time_t now;
    time(&now);
    localtime_r(&now, &currentTime);
    Serial.printf("Current time is %02d:%02d\n", currentTime.tm_hour, currentTime.tm_min);
}

void modPrayerRegisterCommands() {
    SimpleCLI *cli = svcCliGetCli0();
    svcCliAddCmdHelp("settime", "Set the current time <hour> <minute>");
    cli->addBoundlessCommand("settime", setCurrentTime);

    svcCliAddCmdHelp("gettime", "Get the current time");
    cli->addCommand("gettime", printCurrentTime);
}

bool isTimeValid(tm time) {
    return time.tm_mday >= MIN_DAY && time.tm_mday <= MAX_DAY &&
           time.tm_hour >= MIN_HOUR && time.tm_hour <= MAX_HOUR &&
           time.tm_min >= MIN_MINUTE && time.tm_min <= MAX_MINUTE;
}
