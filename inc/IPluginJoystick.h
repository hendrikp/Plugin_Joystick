/* Joystick_Plugin - for licensing and copyright see license.txt */

#include <IPluginBase.h>

#pragma once

/**
* @brief Joystick Plugin Namespace
*/
namespace JoystickPlugin
{
    /**
    * @brief plugin Joystick concrete interface
    */
    struct IPluginJoystick
    {
        /**
        * @brief Get Plugin base interface
        */
        virtual PluginManager::IPluginBase* GetBase() = 0;

        // TODO: Add your concrete interface declaration here
    };
};