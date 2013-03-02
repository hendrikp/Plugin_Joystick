/* Joystick_Plugin - for licensing and copyright see license.txt */

#pragma once

#include <Game.h>

#include <IPluginManager.h>
#include <IPluginBase.h>
#include <CPluginBase.hpp>

#include <IPluginJoystick.h>

#include <dinput.h>

#define PLUGIN_NAME "Joystick"
#define PLUGIN_CONSOLE_PREFIX "[" PLUGIN_NAME " " PLUGIN_TEXT "] " //!< Prefix for Logentries by this plugin

namespace JoystickPlugin
{
    extern void* g_hInst;
    extern LPDIRECTINPUT8 g_lpdi;
    typedef std::map<LPDIRECTINPUTDEVICE8, DIDEVCAPS> TJoysticks;
    extern TJoysticks g_joysticks;

    /**
    * @brief Provides information and manages the resources of this plugin.
    */
    class CPluginJoystick :
        public PluginManager::CPluginBase,
        public IPluginJoystick
    {
        public:
            CPluginJoystick();
            ~CPluginJoystick();

            // IPluginBase
            bool Release( bool bForce = false );

            int GetInitializationMode() const
            {
                return int( PluginManager::IM_Default );
            };

            bool Init( SSystemGlobalEnvironment& env, SSystemInitParams& startupParams, IPluginBase* pPluginManager, const char* sPluginDirectory );

            const char* GetVersion() const
            {
                return "1.2.1.0";
            };

            const char* GetName() const
            {
                return PLUGIN_NAME;
            };

            const char* GetCategory() const
            {
                return "Hardware";
            };

            const char* ListAuthors() const
            {
                return "James-Ryan <neo-ryan at web dot de>"
                       ",\nHendrik Polczynski - Plugin SDK Port <hendrikpolczyn at gmail dot com>";
            };

            const char* ListCVars() const;

            const char* GetStatus() const;

            const char* GetCurrentConcreteInterfaceVersion() const
            {
                return "1.0";
            };

            void* GetConcreteInterface( const char* sInterfaceVersion )
            {
                return static_cast < IPluginJoystick* > ( this );
            };

            // IPluginJoystick
            IPluginBase* GetBase()
            {
                return static_cast<IPluginBase*>( this );
            };

            // TODO: Add your concrete interface implementation
    };

    extern CPluginJoystick* gPlugin;
}

/**
* @brief This function is required to use the Autoregister Flownode without modification.
* Include the file "CPluginJoystick.h" in front of flownode.
*/
inline void GameWarning( const char* sFormat, ... ) PRINTF_PARAMS( 1, 2 );
inline void GameWarning( const char* sFormat, ... )
{
    va_list ArgList;
    va_start( ArgList, sFormat );
    JoystickPlugin::gPlugin->LogV( ILog::eWarningAlways, sFormat, ArgList );
    va_end( ArgList );
};
