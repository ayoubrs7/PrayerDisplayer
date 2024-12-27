/*===========================================================================*/
/// \file mod_cli0.cpp
///
/// \brief
///    Brief description of the module purpose goes here
///
/// \details
///    Detailed description of the module purpose goes here
///
/// \author
///    Adam Q.
///
/*===========================================================================*/

/*=============================================================================
                                     Includes
=============================================================================*/

#include "mod_cli0.h"

#include "svc_cli.h"

/*=============================================================================
                                     Defines
=============================================================================*/

#define MOD_CLI0_CMD_BUFFER_SIZE 128

/*=============================================================================
                                     Macros
=============================================================================*/

/*=============================================================================
                                 Type definitions
=============================================================================*/

/*=============================================================================
                                    Structures
=============================================================================*/

typedef struct {
    bool echo;
} modCli0Config_t;

/*=============================================================================
                            Private Function Prototypes
=============================================================================*/

void modCLi0RegisterCommands();

void commandEcho(cmd *c);

void commandHelp(cmd *c);

size_t readUntilEOL(char *buffer, size_t size);

bool isEOL(char key);

bool isArrowKeySequence(char key);

bool isDeleteOrBackspace(char key);

void echo(char key);

/*=============================================================================
                                Private Variables
=============================================================================*/

static SimpleCLI cli0;
static unsigned long previousTime = 0;

static char cmdBuffer[MOD_CLI0_CMD_BUFFER_SIZE];

static modCli0Config_t modCli0Config = {
    .echo = true,
};

static modCliCmdHelpInfo_t cmdHelpInformations[MOD_CLI0_CMD_HELP_DATA_SIZE];

/*=============================================================================
                                Private Constants
=============================================================================*/

static constexpr int COMMAND_QUEUE_SIZE = 10;
static constexpr int ERROR_QUEUE_SIZE = 10;

/*=============================================================================
                                Public Functions
=============================================================================*/

bool modCli0Init() {
    cli0 = SimpleCLI(COMMAND_QUEUE_SIZE, ERROR_QUEUE_SIZE);
    bool status = svcCliRegisterCli0(&cli0);
    status = svcCliRegisterCmdHelpData(cmdHelpInformations);
    modCLi0RegisterCommands();
    return status;
}


_Noreturn void modCli0EntryPoint(void *pvParameters) {
    Serial.write("\r\n> ");
    while (true) {
        // Read the command
        size_t length = readUntilEOL(cmdBuffer, MOD_CLI0_CMD_BUFFER_SIZE);

        // Parse the command
        cli0.parse(cmdBuffer, length);

        // New Line
        Serial.write("\r\n> ");

        // Delay
        vTaskDelay(10);
    }
}

/*=============================================================================
                                Private Functions
=============================================================================*/

void commandEcho(cmd *c) {
    Command cmd(c);
    Serial.write("\r\n");
    for (int i = 0; i < cmd.countArgs(); i++) {
        Argument arg = cmd.getArgument(i);
        Serial.print(arg.getValue());
        Serial.print(" ");
    }
}

void commandHelp(cmd *c) {
    Serial.write("\r\nAvailable commands:\r\n");

    for (const modCliCmdHelpInfo_t &helpInfo: cmdHelpInformations) {
        if (helpInfo.isUsed) {
            Serial.write(helpInfo.name);
            Serial.write(" - ");
            Serial.write(helpInfo.description);
            Serial.write("\r\n");
        } else {
            break;
        }
    }
}

void modCLi0RegisterCommands() {
    SimpleCLI *cli = svcCliGetCli0();
    // Register the commands
    svcCliAddCmdHelp("help", "Displays the help menu");
    cli->addCommand("help", commandHelp);
    svcCliAddCmdHelp("echo", "Echoes the arguments back to the console");
    cli->addBoundlessCommand("echo", commandEcho);
}

size_t readUntilEOL(char *buffer, size_t size) {
    size_t cmdLength = 0;
    while (true) {
        if (Serial.available() > 0) {
            const char key = static_cast<char>(Serial.read());

            if (isEOL(key)) {
                return cmdLength;
            }

            // Check for delete or backsapce
            if (isDeleteOrBackspace(key)) {
                if (cmdLength > 0) {
                    cmdLength--;
                    echo(key);
                }
                continue;
            }

            // Check if arrow keys are pressed
            if (isArrowKeySequence(key)) {
                continue;
            }

            // Check if the buffer is full
            if (cmdLength >= size) {
                Serial.print("Buffer full!!");
                continue;
            }

            buffer[cmdLength] = key;
            cmdLength++;

            // Echo Back
            echo(key);
        }
    }
}

void echo(const char key) {
    if (modCli0Config.echo) {
        Serial.write(key);
    }
}

bool isEOL(const char key) {
    return key == '\r' || key == '\n';
}


bool isArrowKeySequence(const char key) {
    // up: 27, 91, 65 - down: 27, 91, 66 - left: 27, 91, 68 - right: 27, 91, 67
    static char lastKeyBuffer[3] = {0, 0, 0};
    constexpr char ARROW_SEQUENCE[3] = {27, 91, 0};
    constexpr char LAST_KEYS[4] = {65, 66, 67, 68};

    // Combination - Level 1
    if (key == ARROW_SEQUENCE[0]) {
        lastKeyBuffer[0] = key;
        return true;
    }

    // Combination - Level 2
    if (lastKeyBuffer[0] == ARROW_SEQUENCE[0]) {
        if (key == ARROW_SEQUENCE[1]) {
            lastKeyBuffer[1] = key;
            return true;
        }
    }

    // Combination - Level 3
    if (lastKeyBuffer[1] == ARROW_SEQUENCE[1]) {
        for (const char lastKey: LAST_KEYS) {
            if (key == lastKey) {
                lastKeyBuffer[2] = key;
                return true;
            }
        }
    }

    return false;
}

bool isDeleteOrBackspace(const char key) {
    return key == '\b' || key == 127;
}
