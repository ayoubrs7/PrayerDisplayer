/*===========================================================================*/
/// \file svc_display.cpp
///
/// \brief
///    Module for handling data to be displayed on the screen
///
/// \details
///    Handle the data to be displayed on the screen and the logic for the display
///
/// \author
///    Ayoub Q.
///
/*===========================================================================*/

/*=============================================================================
                                     Includes
=============================================================================*/

#include "svc_display.h"
#include "Adafruit_SSD1306.h"
#include "Fonts/FreeSerif9pt7b.h"
#include <Wire.h>

/*=============================================================================
                                     Defines
=============================================================================*/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

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

String svcPrayerNameToString(PrayerName prayerName);

/*=============================================================================
                                Private Variables
=============================================================================*/

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/*=============================================================================
                                Private Constants
=============================================================================*/

/*=============================================================================
                                Library Entry Point
=============================================================================*/

/*=============================================================================
                                Public Functions
=============================================================================*/

bool svcDisplayInit() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        return false;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setFont(&FreeSerif9pt7b);
    // display.setTextSize(1);
    display.setCursor(10, 35);
    display.println("svcDisplayInit");
    display.display();

    return true;
}

void svcDisplayNextPrayer(Prayer nextPrayer) {
    display.clearDisplay(); // Clear the display before drawing new content

    // Display the header
    display.setTextSize(1);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds("Next Prayer:", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 15);
    display.println("Next Prayer:");

    // Display the prayer name
    display.setTextSize(1); // Increase text size for better readability
    display.getTextBounds(svcPrayerNameToString(nextPrayer.name), 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 35); // Adjust cursor position
    display.println(svcPrayerNameToString(nextPrayer.name));

    // Display the prayer time
    display.setTextSize(1); // Keep the same text size for consistency
    char timeBuffer[6]; // Buffer to hold the formatted time string
    sprintf(timeBuffer, "%02d:%02d", nextPrayer.hour, nextPrayer.minute);
    display.getTextBounds(timeBuffer, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 55); // Adjust cursor position
    display.println(timeBuffer);

    display.display(); // Update the display with the new content
}

/*=============================================================================
                                Private Functions
=============================================================================*/

String svcPrayerNameToString(PrayerName prayerName) {
    switch (prayerName) {
        case FAJR:
            return "Fajr";
        case DHUHR:
            return "Dhuhr";
        case ASR:
            return "Asr";
        case MAGHRIB:
            return "Maghrib";
        case ISHA:
            return "Isha";
        default:
            return "None";
    }
}
