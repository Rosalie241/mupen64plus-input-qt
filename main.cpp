#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_common.h"
#include "m64p_types.h"
#include "m64p_plugin.h"
#include "main.h"
#include "configdialog.h"

#include <QGamepad>
#include <QGamepadManager>
#include <SDL2/SDL.h>

#define QT_INPUT_PLUGIN_VERSION 0x020500
#define INPUT_PLUGIN_API_VERSION 0x020100
static int l_PluginInit = 0;
static unsigned char myKeyState[SDL_NUM_SCANCODES];
SController controller[4];   // 4 controllers
QGamepad* pads[4];

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle, void *, void (*)(void *, int, const char *))
{
    if (l_PluginInit)
        return M64ERR_ALREADY_INIT;

    l_PluginInit = 1;

    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    if (!l_PluginInit)
        return M64ERR_NOT_INIT;

    l_PluginInit = 0;

    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    /* set version info */
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_INPUT;

    if (PluginVersion != NULL)
        *PluginVersion = QT_INPUT_PLUGIN_VERSION;

    if (APIVersion != NULL)
        *APIVersion = INPUT_PLUGIN_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = "Mupen64Plus Qt Input Plugin";

    if (Capabilities != NULL)
    {
        *Capabilities = 0;
    }

    return M64ERR_SUCCESS;
}

EXPORT void CALL ControllerCommand(int, unsigned char *)
{
}

EXPORT void CALL GetKeys( int Control, BUTTONS *Keys )
{
    if (pads[Control] == NULL)
        return;

    Keys->R_DPAD       = pads[Control]->buttonRight();
    Keys->L_DPAD       = pads[Control]->buttonLeft();
    Keys->D_DPAD       = pads[Control]->buttonDown();
    Keys->U_DPAD       = pads[Control]->buttonUp();
    Keys->START_BUTTON = pads[Control]->buttonStart();
    Keys->Z_TRIG       = pads[Control]->buttonL2();
    Keys->B_BUTTON     = pads[Control]->buttonX();
    Keys->A_BUTTON     = pads[Control]->buttonA();
    Keys->R_CBUTTON    = pads[Control]->axisRightX() > 0;
    Keys->L_CBUTTON    = pads[Control]->axisRightX() < 0;
    Keys->D_CBUTTON    = pads[Control]->axisRightY() > 0;
    Keys->U_CBUTTON    = pads[Control]->axisRightY() < 0;
    Keys->R_TRIG       = pads[Control]->buttonR1();
    Keys->L_TRIG       = pads[Control]->buttonL1();

//    Keys->X_AXIS       = pads[Control]->axisLeftX();
  //  Keys->Y_AXIS       = pads[Control]->axisLeftY();
}

EXPORT void CALL InitiateControllers(CONTROL_INFO ControlInfo)
{
    int i;

    // reset controllers
    memset( controller, 0, sizeof( SController ) * 4 );
    for ( i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        myKeyState[i] = 0;
    }
    // set our CONTROL struct pointers to the array that was passed in to this function from the core
    // this small struct tells the core whether each controller is plugged in, and what type of pak is connected
    for (i = 0; i < 4; i++)
        controller[i].control = ControlInfo.Controls + i;

    controller[0].control->Present = 1;
    pads[0] = new QGamepad(0);
    pads[1] = NULL;
    pads[2] = NULL;
    pads[3] = NULL;

    QList<int> myList = QGamepadManager::instance()->connectedGamepads();
    for (i = 0; i < myList.size(); i++)
    {
        printf("pad %u\n", myList[i]);
    }
}

EXPORT void CALL ReadController(int, unsigned char *)
{
}

EXPORT int CALL RomOpen(void)
{
    return 1;
}

EXPORT void CALL RomClosed(void)
{

}

EXPORT void CALL SDL_KeyDown(int, int keysym)
{
    myKeyState[keysym] = 1;
}

EXPORT void CALL SDL_KeyUp(int, int keysym)
{
    myKeyState[keysym] = 0;
}

EXPORT void CALL Config_DoConfig()
{
    ConfigDialog config;
    config.exec();
}
