/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-input-qt - main.cpp                                       *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2008-2011 Richard Goedeken                              *
 *   Copyright (C) 2008 Tillin9                                            *
 *   Copyright (C) 2002 Blight                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_common.h"
#include "m64p_types.h"
#include "m64p_config.h"
#include "m64p_plugin.h"
#include "main.h"
#include "configdialog.h"
#include "osal/osal_dynamiclib.h"

#include <QDir>

#define AXIS_PEAK 32768
#define MAX_AXIS_VALUE 85
#define DEADZONE_DEFAULT 5.0

#define QT_INPUT_PLUGIN_VERSION 0x020500
#define INPUT_PLUGIN_API_VERSION 0x020100
static int l_PluginInit = 0;
int emu_running = 0;
static unsigned char myKeyState[SDL_NUM_SCANCODES];
QSettings* settings;
QSettings* controllerSettings;
SController controller[4];   // 4 controllers

Q_DECLARE_METATYPE(QList<int>)

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle CoreHandle, void *, void (*)(void *, int, const char *))
{
    if (l_PluginInit)
        return M64ERR_ALREADY_INIT;

    ptr_ConfigGetUserConfigPath ConfigGetUserConfigPath = (ptr_ConfigGetUserConfigPath) osal_dynlib_getproc(CoreHandle, "ConfigGetUserConfigPath");

    QDir ini_path(ConfigGetUserConfigPath());
    settings = new QSettings(ini_path.absoluteFilePath("input-profiles.ini"), QSettings::IniFormat);
    controllerSettings = new QSettings(ini_path.absoluteFilePath("input-settings.ini"), QSettings::IniFormat);

    QString section;
    for (int i = 1; i < 5; ++i) {
        section = "Controller" + QString::number(i);
        if (!controllerSettings->childGroups().contains(section)) {
            controllerSettings->setValue(section + "/Profile", "Auto");
            controllerSettings->setValue(section + "/Gamepad", "Auto");
            controllerSettings->setValue(section + "/Pak", "Memory");
        }
    }

    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

    QList<int> values;
    section = "Auto-Keyboard";
    values.insert(0, 0/*blank value*/);
    values.insert(1, 0/*Keyboard*/);
    if (!settings->childGroups().contains(section)) {
        values.replace(0, SDL_SCANCODE_LSHIFT);
        settings->setValue(section + "/A", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_LCTRL);
        settings->setValue(section + "/B", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_Z);
        settings->setValue(section + "/Z", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_X);
        settings->setValue(section + "/L", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_C);
        settings->setValue(section + "/R", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_RETURN);
        settings->setValue(section + "/Start", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_A);
        settings->setValue(section + "/DPadL", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_D);
        settings->setValue(section + "/DPadR", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_W);
        settings->setValue(section + "/DPadU", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_S);
        settings->setValue(section + "/DPadD", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_J);
        settings->setValue(section + "/CLeft", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_L);
        settings->setValue(section + "/CRight", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_I);
        settings->setValue(section + "/CUp", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_K);
        settings->setValue(section + "/CDown", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_LEFT);
        settings->setValue(section + "/AxisLeft", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_RIGHT);
        settings->setValue(section + "/AxisRight", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_UP);
        settings->setValue(section + "/AxisUp", QVariant::fromValue(values));
        values.replace(0, SDL_SCANCODE_DOWN);
        settings->setValue(section + "/AxisDown", QVariant::fromValue(values));

        settings->setValue(section + "/Deadzone", DEADZONE_DEFAULT);
        settings->setValue(section + "/Sensitivity", 100.0);
    }

    section = "Auto-Gamepad";
    values.replace(1, 1/*Button*/);
    if (!settings->childGroups().contains(section)) {
        values.replace(0, SDL_CONTROLLER_BUTTON_A);
        settings->setValue(section + "/A", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_BUTTON_X);
        settings->setValue(section + "/B", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        values.replace(1, 2/*Axis*/);
        values.insert(2, 1 /* positive axis value*/);
        settings->setValue(section + "/Z", QVariant::fromValue(values));
        values.removeAt(2);
        values.replace(1, 1/*Button*/);
        values.replace(0, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        settings->setValue(section + "/L", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        settings->setValue(section + "/R", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_BUTTON_START);
        settings->setValue(section + "/Start", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        settings->setValue(section + "/DPadL", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        settings->setValue(section + "/DPadR", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_BUTTON_DPAD_UP);
        settings->setValue(section + "/DPadU", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        settings->setValue(section + "/DPadD", QVariant::fromValue(values));
        values.replace(1, 2/*Axis*/);
        values.replace(0, SDL_CONTROLLER_AXIS_RIGHTX);
        values.insert(2, -1 /* negative axis value*/);
        settings->setValue(section + "/CLeft", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_AXIS_RIGHTX);
        values.replace(2, 1 /* positive axis value*/);
        settings->setValue(section + "/CRight", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_AXIS_RIGHTY);
        values.replace(2, -1 /* negative axis value*/);
        settings->setValue(section + "/CUp", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_AXIS_RIGHTY);
        values.replace(2, 1 /* positive axis value*/);
        settings->setValue(section + "/CDown", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_AXIS_LEFTX);
        values.replace(2, -1 /* negative axis value*/);
        settings->setValue(section + "/AxisLeft", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_AXIS_LEFTX);
        values.replace(2, 1 /* positive axis value*/);
        settings->setValue(section + "/AxisRight", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_AXIS_LEFTY);
        values.replace(2, -1 /* negative axis value*/);
        settings->setValue(section + "/AxisUp", QVariant::fromValue(values));
        values.replace(0, SDL_CONTROLLER_AXIS_LEFTY);
        values.replace(2, 1 /* positive axis value*/);
        settings->setValue(section + "/AxisDown", QVariant::fromValue(values));

        settings->setValue(section + "/Deadzone", DEADZONE_DEFAULT);
        settings->setValue(section + "/Sensitivity", 100.0);
    }

    if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER))
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

    if (!SDL_WasInit(SDL_INIT_HAPTIC))
        SDL_InitSubSystem(SDL_INIT_HAPTIC);

    l_PluginInit = 1;

    return M64ERR_SUCCESS;
}

void closeControllers()
{
    for (int i = 0; i < 4; ++i) {
        if (controller[i].haptic != NULL)
            SDL_HapticClose(controller[i].haptic);
        if (controller[i].gamepad != NULL)
            SDL_GameControllerClose(controller[i].gamepad);
        else if (controller[i].joystick != NULL)
            SDL_JoystickClose(controller[i].joystick);
        controller[i].haptic = NULL;
        controller[i].gamepad = NULL;
        controller[i].joystick = NULL;
    }
    emu_running = 0;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    if (!l_PluginInit)
        return M64ERR_NOT_INIT;

    closeControllers();

    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    SDL_QuitSubSystem(SDL_INIT_HAPTIC);

    settings->sync();
    controllerSettings->sync();

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

static unsigned char DataCRC( unsigned char *Data, int iLenght )
{
    unsigned char Remainder = Data[0];

    int iByte = 1;
    unsigned char bBit = 0;

    while( iByte <= iLenght )
    {
        int HighBit = ((Remainder & 0x80) != 0);
        Remainder = Remainder << 1;

        Remainder += ( iByte < iLenght && Data[iByte] & (0x80 >> bBit )) ? 1 : 0;

        Remainder ^= (HighBit) ? 0x85 : 0;

        bBit++;
        iByte += bBit/8;
        bBit %= 8;
    }

    return Remainder;
}

EXPORT void CALL ControllerCommand(int Control, unsigned char *Command)
{
    unsigned char *Data = &Command[5];

    if (Control == -1)
        return;

    switch (Command[2])
    {
        case RD_GETSTATUS:
        case RD_READKEYS:
            break;
        case RD_READPAK:
            if (controller[Control].control->Plugin == PLUGIN_RAW)
            {
                unsigned int dwAddress = (Command[3] << 8) + (Command[4] & 0xE0);

                if(( dwAddress >= 0x8000 ) && ( dwAddress < 0x9000 ) )
                    memset( Data, 0x80, 32 );
                else
                    memset( Data, 0x00, 32 );

                Data[32] = DataCRC( Data, 32 );
            }
            break;
        case RD_WRITEPAK:
            if (controller[Control].control->Plugin == PLUGIN_RAW)
            {
                unsigned int dwAddress = (Command[3] << 8) + (Command[4] & 0xE0);
                if(dwAddress == PAK_IO_RUMBLE && controller[Control].haptic) {
                    if (*Data) {
                        SDL_HapticRumblePlay(controller[Control].haptic, 1, SDL_HAPTIC_INFINITY);
                    } else {
                        SDL_HapticRumbleStop(controller[Control].haptic);
                    }
                }
                Data[32] = DataCRC( Data, 32 );
            }
            break;
        case RD_RESETCONTROLLER:
        case RD_READEEPROM:
        case RD_WRITEEPROM:
            break;
        }
}

int modifyAxisValue(int axis_value, int Control, int direction)
{
    axis_value = ((abs(axis_value) - controller[Control].deadzone) * MAX_AXIS_VALUE) / controller[Control].range;
    axis_value *= direction;
    axis_value = (float)axis_value * controller[Control].sensitivity;
    axis_value = std::max(-MAX_AXIS_VALUE, std::min(axis_value, MAX_AXIS_VALUE));

    return axis_value;
}

void setAxis(int Control, int axis, BUTTONS *Keys, QString axis_dir, int direction)
{
    int axis_value;
    QList<int> value = settings->value(controller[Control].profile + "/" + axis_dir).value<QList<int> >();
    switch (value.at(1)) {
        case 0 /*Keyboard*/:
            if (myKeyState[value.at(0)]) {
                if (axis == 0)
                    Keys->X_AXIS = (int8_t)(MAX_AXIS_VALUE * direction);
                else
                    Keys->Y_AXIS = (int8_t)(MAX_AXIS_VALUE * direction);
            }
            break;
        case 1 /*Button*/:
            if (SDL_GameControllerGetButton(controller[Control].gamepad, (SDL_GameControllerButton)value.at(0))) {
                if (axis == 0)
                    Keys->X_AXIS = (int8_t)(MAX_AXIS_VALUE * direction);
                else
                    Keys->Y_AXIS = (int8_t)(MAX_AXIS_VALUE * direction);
            }
            break;
        case 2 /*Axis*/:
            axis_value = SDL_GameControllerGetAxis(controller[Control].gamepad, (SDL_GameControllerAxis)value.at(0));
            if (abs(axis_value) > controller[Control].deadzone && axis_value * value.at(2) > 0) {
                axis_value = modifyAxisValue(axis_value, Control, direction);
                if (axis == 0)
                    Keys->X_AXIS = (int8_t)axis_value;
                else
                    Keys->Y_AXIS = (int8_t)axis_value;
            }
            break;
        case 3 /*Joystick Hat*/:
            if (SDL_JoystickGetHat(controller[Control].joystick, value.at(0)) & value.at(2)) {
                if (axis == 0)
                    Keys->X_AXIS = (int8_t)(MAX_AXIS_VALUE * direction);
                else
                    Keys->Y_AXIS = (int8_t)(MAX_AXIS_VALUE * direction);
            }
            break;
        case 4 /*Joystick Button*/:
            if (SDL_JoystickGetButton(controller[Control].joystick, value.at(0))) {
                if (axis == 0)
                    Keys->X_AXIS = (int8_t)(MAX_AXIS_VALUE * direction);
                else
                    Keys->Y_AXIS = (int8_t)(MAX_AXIS_VALUE * direction);
            }
            break;
        case 5 /*Joystick Axis*/:
            axis_value = SDL_JoystickGetAxis(controller[Control].joystick, value.at(0));
            if (abs(axis_value) > controller[Control].deadzone && axis_value * value.at(2) > 0) {
                axis_value = modifyAxisValue(axis_value, Control, direction);
                if (axis == 0)
                    Keys->X_AXIS = (int8_t)axis_value;
                else
                    Keys->Y_AXIS = (int8_t)axis_value;
            }
            break;
    }
}

void setKey(int Control, uint32_t key, BUTTONS *Keys, QString button)
{
    int axis_value;
    QList<int> value = settings->value(controller[Control].profile + "/" + button).value<QList<int> >();
    switch (value.at(1)) {
        case 0 /*Keyboard*/:
            if (myKeyState[value.at(0)])
                Keys->Value |= key;
            break;
        case 1 /*Button*/:
            if (SDL_GameControllerGetButton(controller[Control].gamepad, (SDL_GameControllerButton)value.at(0)))
                Keys->Value |= key;
            break;
        case 2 /*Axis*/:
            axis_value = SDL_GameControllerGetAxis(controller[Control].gamepad, (SDL_GameControllerAxis)value.at(0));
            if (abs(axis_value) >= (AXIS_PEAK / 2) && axis_value * value.at(2) > 0)
                Keys->Value |= key;
            break;
        case 3 /*Joystick Hat*/:
            if (SDL_JoystickGetHat(controller[Control].joystick, value.at(0)) & value.at(2))
                Keys->Value |= key;
            break;
        case 4 /*Joystick Button*/:
            if (SDL_JoystickGetButton(controller[Control].joystick, value.at(0)))
                Keys->Value |= key;
            break;
        case 5 /*Joystick Axis*/:
            axis_value = SDL_JoystickGetAxis(controller[Control].joystick, value.at(0));
            if (key == 0x0020/*Z*/)
            {
                if (axis_value > 0)
                    Keys->Value |= key;
                break;
            }

            if (abs(axis_value) >= (AXIS_PEAK / 2) && axis_value * value.at(2) > 0)
                Keys->Value |= key;
            break;
    }
}

void setPak(int Control)
{
    QString pak = controllerSettings->value("Controller" + QString::number(Control + 1) + "/Pak").toString();
    if (pak == "Transfer")
        controller[Control].control->Plugin = PLUGIN_TRANSFER_PAK;
    else if (pak == "Rumble") {
        controller[Control].control->Plugin = PLUGIN_RAW;
        if (controller[Control].haptic)
            return;

        if (controller[Control].joystick)
            controller[Control].haptic = SDL_HapticOpenFromJoystick(controller[Control].joystick);
        if (controller[Control].haptic) {
            if (SDL_HapticRumbleInit(controller[Control].haptic) != 0) {
                SDL_HapticClose(controller[Control].haptic);
                controller[Control].haptic = NULL;
            }
        }
    }
    else if (pak == "None")
        controller[Control].control->Plugin = PLUGIN_NONE;
    else
        controller[Control].control->Plugin = PLUGIN_MEMPAK;
}

EXPORT void CALL GetKeys( int Control, BUTTONS *Keys )
{
    if (controller[Control].control->Present == 0)
        return;

    setPak(Control);

    Keys->Value = 0;
    setKey(Control, 0x0001/*R_DPAD*/, Keys, "DPadR");
    setKey(Control, 0x0002/*L_DPAD*/, Keys, "DPadL");
    setKey(Control, 0x0004/*D_DPAD*/, Keys, "DPadD");
    setKey(Control, 0x0008/*U_DPAD*/, Keys, "DPadU");
    setKey(Control, 0x0010/*START_BUTTON*/, Keys, "Start");
    setKey(Control, 0x0020/*Z_TRIG*/, Keys, "Z");
    setKey(Control, 0x0040/*B_BUTTON*/, Keys, "B");
    setKey(Control, 0x0080/*A_BUTTON*/, Keys, "A");
    setKey(Control, 0x0100/*R_CBUTTON*/, Keys, "CRight");
    setKey(Control, 0x0200/*L_CBUTTON*/, Keys, "CLeft");
    setKey(Control, 0x0400/*D_CBUTTON*/, Keys, "CDown");
    setKey(Control, 0x0800/*U_CBUTTON*/, Keys, "CUp");
    setKey(Control, 0x1000/*R_TRIG*/, Keys, "R");
    setKey(Control, 0x2000/*L_TRIG*/, Keys, "L");

    setAxis(Control, 0/*X_AXIS*/, Keys, "AxisLeft", -1);
    setAxis(Control, 0/*X_AXIS*/, Keys, "AxisRight", 1);
    setAxis(Control, 1/*Y_AXIS*/, Keys, "AxisUp", 1);
    setAxis(Control, 1/*Y_AXIS*/, Keys, "AxisDown", -1);
}

EXPORT void CALL InitiateControllers(CONTROL_INFO ControlInfo)
{
    int i, j;

    for (i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        myKeyState[i] = 0;
    }
    // set our CONTROL struct pointers to the array that was passed in to this function from the core
    // this small struct tells the core whether each controller is plugged in, and what type of pak is connected
    QString gamepad;
    int controller_index;
    QString gamepad_name;
    int auto_index = 0;
    int used_index[4] = {-1, -1, -1, -1};
    for (i = 0; i < 4; i++) {
        controller[i].control = ControlInfo.Controls + i;
        controller[i].control->RawData = 0;
        controller[i].control->Present = 0;
        controller[i].gamepad = NULL;
        controller[i].haptic = NULL;
        controller[i].joystick = NULL;
        gamepad = controllerSettings->value("Controller" + QString::number(i + 1) + "/Gamepad").toString();
        if (gamepad == "Keyboard")
            controller[i].control->Present = 1;
        else if (gamepad == "None")
            controller[i].control->Present = 0;
        else if (gamepad == "Auto") {
            for (j = 0; j < 4; ++j) {
                if (auto_index == used_index[j]) {
                    ++auto_index;
                    j = -1;
                }
            }
            if (SDL_IsGameController(auto_index)) {
                controller[i].gamepad = SDL_GameControllerOpen(auto_index);
                if (controller[i].gamepad)
                    controller[i].control->Present = 1;
            }
            else {
                controller[i].joystick = SDL_JoystickOpen(auto_index);
                if (controller[i].joystick)
                    controller[i].control->Present = 1;
            }
            ++auto_index;
            if (i == 0) controller[i].control->Present = 1; //Player 1
        }
        else /*specific gamepad selected*/ {
            controller_index = gamepad.split(":")[0].toInt();
            gamepad_name = gamepad.split(":")[1];
            if (SDL_IsGameController(controller_index) && gamepad_name == QString(SDL_GameControllerNameForIndex(controller_index))) {
                controller[i].gamepad = SDL_GameControllerOpen(controller_index);
                used_index[i] = controller_index;
                if (controller[i].gamepad)
                    controller[i].control->Present = 1;
            }
            else if (gamepad_name == QString(SDL_JoystickNameForIndex(controller_index))) {
                controller[i].joystick = SDL_JoystickOpen(controller_index);
                used_index[i] = controller_index;
                if (controller[i].joystick)
                    controller[i].control->Present = 1;
            }
            if (controller[i].control->Present == 0) {
                controllerSettings->setValue("Controller" + QString::number(i + 1) + "/Gamepad", "Auto");
                --i; //Try again using Auto
                continue;
            }
        }

        if (controller[i].gamepad)
            controller[i].joystick = SDL_GameControllerGetJoystick(controller[i].gamepad);

        controller[i].profile = controllerSettings->value("Controller" + QString::number(i + 1) + "/Profile").toString();
        if (!settings->childGroups().contains(controller[i].profile))
            controller[i].profile = "Auto";

        if (controller[i].profile == "Auto") {
            if (controller[i].gamepad)
                controller[i].profile = "Auto-Gamepad";
            else
                controller[i].profile = "Auto-Keyboard";
        }

        if (!settings->contains(controller[i].profile + "/Deadzone"))
            settings->setValue(controller[i].profile + "/Deadzone", DEADZONE_DEFAULT);
        if (!settings->contains(controller[i].profile + "/Sensitivity"))
            settings->setValue(controller[i].profile + "/Sensitivity", 100.0);

        controller[i].deadzone = AXIS_PEAK * (settings->value(controller[i].profile + "/Deadzone").toFloat() / 100.0);
        controller[i].range = AXIS_PEAK - controller[i].deadzone;
        controller[i].sensitivity = settings->value(controller[i].profile + "/Sensitivity").toFloat() / 100.0;

        setPak(i);
    }
    emu_running = 1;
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
    closeControllers();
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
