/*===========================================================================*/
/// \file mod_timings.h
///
/// \brief
///    Module for handling Bluetooth communication to the device
///
/// \details
///     Handle the connection and the data transfer between the device and the mobile application
///
/// \author
///     Ayoub Q.
///
/*===========================================================================*/

#ifndef MOD_TIMINGS_H
#define MOD_TIMINGS_H

/*=============================================================================
                                     Includes
=============================================================================*/

#include <Arduino.h>

/*=============================================================================
                                     Defines
=============================================================================*/
#define MAX_DAYS 31
/*=============================================================================
                                     Macros
=============================================================================*/

/*=============================================================================
                                      Enums
=============================================================================*/

typedef enum {
    FAJR,
    DHUHR,
    ASR,
    MAGHRIB,
    ISHA,
    NONE
} PrayerName;

/*=============================================================================
                                 Type definitions
=============================================================================*/

typedef struct {
    QueueHandle_t timingsQueue;
    SemaphoreHandle_t prayerSemaphore;
} modBTEParams_t;

typedef struct {
    uint8_t hour;
    uint8_t minute;
    PrayerName name;
} Prayer;

typedef struct {
    Prayer fajr;
    Prayer dhuhr;
    Prayer asr;
    Prayer maghrib;
    Prayer isha;
    String date;
} PrayerTimings;

/*=============================================================================
                                    Structures
=============================================================================*/

/*=============================================================================
                                Public Constants
=============================================================================*/

/*=============================================================================
                            Public Function Prototypes
=============================================================================*/

/// \brief Entry point for the module
/// \param[in] pvParameters - FreeRTOS task parameters
_Noreturn void modBTETaskProcess(void *pvParameters);

#endif // MOD_TIMINGS_H
