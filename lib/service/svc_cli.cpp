/*===========================================================================*/
/// \file svc_cli.cpp
///
/// \brief
///    Brief description of the module purpose goes here
///
/// \details
///    Detailed description of the module purpose goes here
///
/// \author
///    Ayoub Q.
///
/*===========================================================================*/

/*=============================================================================
                                     Includes
=============================================================================*/

#include "svc_cli.h"

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
                                    Structures
=============================================================================*/


/*=============================================================================
                            Private Function Prototypes
=============================================================================*/


/*=============================================================================
                                Private Variables
=============================================================================*/

static SimpleCLI *cli = nullptr;
static modCliCmdHelpInfo_t *cmdHelpData = nullptr;
static uint32_t cmdHelpSize = 0;

/*=============================================================================
                                Private Constants
=============================================================================*/


/*=============================================================================
                                Public Functions
=============================================================================*/

bool svcCliRegisterCmdHelpData(modCliCmdHelpInfo_t *data) {
    if (data == nullptr) {
        return false;
    }
    cmdHelpData = data;
    return true;
}

bool svcCliRegisterCli0(SimpleCLI *cli0) {
    if (cli0 == nullptr) {
        return false;
    }
    cli = cli0;
    return true;
}

SimpleCLI *svcCliGetCli0() {
    return cli;
}

void svcCliAddCmdHelp(const char *name, const char *description) {
    if (cmdHelpData == nullptr) {
        return;
    }

    while (cmdHelpData[cmdHelpSize].isUsed && cmdHelpSize < MOD_CLI0_CMD_HELP_DATA_SIZE) {
        cmdHelpSize++;
    }

    if (cmdHelpSize < MOD_CLI0_CMD_HELP_DATA_SIZE) {
        cmdHelpData[cmdHelpSize] = {
                .isUsed = true,
                .name = name,
                .description = description
        };
    } else {
        Serial.println("[ERROR] : No more space for help data");
    }
}

/*=============================================================================
                                Private Functions
=============================================================================*/
