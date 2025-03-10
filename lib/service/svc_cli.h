/*===========================================================================*/
/// \file svc_cli.h
///
/// \brief
///    Service for handling the command line interface
///
/// \details
///     Handle the command line interface for the device
///
/// \author
///     Adam Q.
///
/*===========================================================================*/

#ifndef SVC_CLI_H
#define SVC_CLI_H

/*=============================================================================
                                     Includes
=============================================================================*/

#include <mod_cli0.h>
#include <SimpleCLI.h>

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

bool svcCliRegisterCli0(SimpleCLI *cli0);

bool svcCliRegisterCmdHelpData(modCliCmdHelpInfo_t *data);

SimpleCLI *svcCliGetCli0();

void svcCliAddCmdHelp(const char *name, const char *description);

#endif //SVC_CLI_H
