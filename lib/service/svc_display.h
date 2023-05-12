/*===========================================================================*/
/// \file svc_display.h
///
/// \brief
///    Module for handling data to be displayed on the screen
///
/// \details
///     Handle the data to be displayed on the screen and the logic for the display
///
/// \author
///     Ayoub Q.
///
/*===========================================================================*/

#ifndef SVC_DISPLAY_H
#define SVC_DISPLAY_H

/*=============================================================================
                                     Includes
=============================================================================*/
#include <Arduino.h>
#include <mod_timings.h>
/*=============================================================================
                                     Defines
=============================================================================*/

/*=============================================================================
                                     Macros
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
                                Public Constants
=============================================================================*/

/*=============================================================================
                            Public Function Prototypes
=============================================================================*/

/// \brief Initialize the display
/// \return true if the display was initialized successfully, false otherwise
bool svcDisplayInit(void);

/// \brief Display the prayer timings on the screen
/// \param timings The prayer timings to be displayed
void svcDisplayNextPrayer(Prayer nextPrayer);

#endif // SVC_DISPLAY_H
