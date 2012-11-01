/* Joystick_Plugin - for licensing and copyright see license.txt */

#include <StdAfx.h>
#include <CPluginJoystick.h>

#include <dinput.h>

namespace JoystickPlugin
{
    CPluginJoystick* gPlugin = NULL;

    void* g_hInst = 0;
    LPDIRECTINPUT8 g_lpdi = NULL;
    TJoysticks g_joysticks;
    LPDIRECTINPUTDEVICE8 currentJoystick;

    BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* instance, VOID* context )
    {
        HWND hDlg = ( HWND )context;

        DIPROPRANGE propRange;
        propRange.diph.dwSize       = sizeof( DIPROPRANGE );
        propRange.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        propRange.diph.dwHow        = DIPH_BYID;
        propRange.diph.dwObj        = instance->dwType;
        propRange.lMin              = -1000;
        propRange.lMax              = +1000;

        if ( FAILED( currentJoystick->SetProperty( DIPROP_RANGE, &propRange.diph ) ) )
        {
            return DIENUM_STOP;
        }

        return DIENUM_CONTINUE;
    }

    BOOL CALLBACK EnumDevicesCallback( const DIDEVICEINSTANCE* instance, VOID* context )
    {
        HRESULT hr;

        LPDIRECTINPUTDEVICE8 joystick;
        hr = g_lpdi->CreateDevice( instance->guidInstance, &joystick, NULL );
        currentJoystick = joystick;

        if ( FAILED( hr ) )
        {
            return DIENUM_CONTINUE;
        }

        if ( FAILED( hr = joystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
        {
            return DIENUM_CONTINUE;
        }

        DIDEVCAPS capabilities;
        capabilities.dwSize = sizeof( DIDEVCAPS );

        if ( FAILED( hr = joystick->GetCapabilities( &capabilities ) ) )
        {
            return DIENUM_CONTINUE;
        }

        if ( FAILED( hr = joystick->EnumObjects( EnumAxesCallback, NULL, DIDFT_AXIS ) ) )
        {
            return DIENUM_CONTINUE;
        }

        g_joysticks.insert( TJoysticks::value_type( joystick, capabilities ) );

        if ( FAILED( hr = joystick->SetCooperativeLevel( NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND ) ) )
        {
            return DIENUM_CONTINUE;
        }

        return DIENUM_STOP;
    }

    HRESULT PollJoystick( DIJOYSTATE2* js, LPDIRECTINPUTDEVICE8 joystick )
    {
        HRESULT hr;

        if ( joystick == NULL )
        {
            return S_OK;
        }

        hr = joystick->Poll();

        if ( FAILED( hr ) )
        {

            hr = joystick->Acquire();

            while ( hr == DIERR_INPUTLOST )
            {
                hr = joystick->Acquire();
            }

            if ( ( hr == DIERR_INVALIDPARAM ) || ( hr == DIERR_NOTINITIALIZED ) )
            {
                return E_FAIL;
            }

            if ( hr == DIERR_OTHERAPPHASPRIO )
            {
                return S_OK;
            }
        }

        if ( FAILED( hr = joystick->GetDeviceState( sizeof( DIJOYSTATE2 ), js ) ) )
        {
            return hr;
        }

        return S_OK;
    }


    CPluginJoystick::CPluginJoystick()
    {
        gPlugin = this;
    }

    CPluginJoystick::~CPluginJoystick()
    {
        Release( true );

        gPlugin = NULL;
    }

    bool CPluginJoystick::Release( bool bForce )
    {
        bool bRet = true;

        if ( !m_bCanUnload )
        {
            // Should be called while Game is still active otherwise there might be leaks/problems
            bRet = CPluginBase::Release( bForce );

            if ( bRet )
            {
                // Depending on your plugin you might not want to unregister anything
                // if the System is quitting.
                if ( gEnv && gEnv->pSystem && !gEnv->pSystem->IsQuitting() )
                {
                    // Unregister CVars
                    if ( gEnv && gEnv->pConsole )
                    {
                        // ...
                    }

                    // Unregister game objects
                    if ( gEnv && gEnv->pGameFramework && gEnv->pGame )
                    {
                        // ...
                    }
                }

                // Cleanup like this always (since the class is static its cleaned up when the dll is unloaded)
                gPluginManager->UnloadPlugin( GetName() );

                // Allow Plugin Manager garbage collector to unload this plugin
                AllowDllUnload();
            }
        }

        return bRet;
    };

    bool CPluginJoystick::Init( SSystemGlobalEnvironment& env, SSystemInitParams& startupParams, IPluginBase* pPluginManager, const char* sPluginDirectory )
    {
        gPluginManager = ( PluginManager::IPluginManager* )pPluginManager->GetConcreteInterface( NULL );
        CPluginBase::Init( env, startupParams, pPluginManager, sPluginDirectory );

        if ( gEnv && gEnv->pSystem && !gEnv->pSystem->IsQuitting() )
        {
            if ( DirectInput8Create( static_cast<HINSTANCE>( g_hInst ), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>( &g_lpdi ), NULL ) == DI_OK )
            {
                g_lpdi->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY );
            }

            else
            {
                gPlugin->LogError( "[DIRECTINPUT] Failed to create input device" );
            }
        }

        // Note: Autoregister Flownodes will be automatically registered

        return true;
    }

    const char* CPluginJoystick::ListCVars() const
    {
        return "..."; // TODO: Enter CVARs/Commands here if you have some
    }

    const char* CPluginJoystick::GetStatus() const
    {
        return "OK";
    }

    // TODO: Add your plugin concrete interface implementation
}