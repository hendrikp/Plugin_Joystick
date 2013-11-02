/* Joystick_Plugin - for licensing and copyright see license.txt */

#include "StdAfx.h"

#include <CPluginJoystick.h>
#include "Nodes/G2FlowBaseNode.h"

namespace JoystickPlugin
{
    HRESULT PollJoystick( DIJOYSTATE2* js, LPDIRECTINPUTDEVICE8 joystick );

    class CFlowJoystickNode :
        public CFlowBaseNode<eNCT_Instanced>,
        public CScriptableBase
    {
        private:

            SActivationInfo m_actInfo;
            int* m_buttons;
            int m_xAxis;
            int m_yAxis;
            int m_zAxis;
            int m_xRotate;
            int m_yRotate;
            int m_zRotate;
            int m_slider;
            int m_POV;
            bool m_bEnabled;

            LPDIRECTINPUTDEVICE8 m_currentJoystick;
            DIDEVCAPS m_CurrentCapabilities;

        public:

            CFlowJoystickNode( SActivationInfo* pActInfo )
            {
#if CDK_VERSION < 343 || CDK_VERSION >= 350
                Init( gEnv->pSystem->GetIScriptSystem(), gEnv->pSystem );
#else
                Init( gEnv->pSystem->GetIScriptSystem() );
#endif
                SetGlobalName( "Joystick" );

#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CFlowJoystickNode::

                SCRIPT_REG_TEMPLFUNC( GetXAxis, "" );
                SCRIPT_REG_TEMPLFUNC( GetYAxis, "" );
                SCRIPT_REG_TEMPLFUNC( GetZAxis, "" );
                SCRIPT_REG_TEMPLFUNC( GetButtonState, "button" );
                SCRIPT_REG_TEMPLFUNC( SetJoystick, "joystick" );
                SCRIPT_REG_TEMPLFUNC( GetNumButtons, "" );
                SCRIPT_REG_TEMPLFUNC( GetPOV, "" );
                SCRIPT_REG_TEMPLFUNC( GetSlider, "" );
                SCRIPT_REG_TEMPLFUNC( GetXRotate, "" );
                SCRIPT_REG_TEMPLFUNC( GetYRotate, "" );
                SCRIPT_REG_TEMPLFUNC( GetZRotate, "" );

#undef SCRIPT_REG_CLASSNAME

                m_xAxis = 0;
                m_yAxis = 0;
                m_zAxis = 0;
                m_xRotate = 0;
                m_yRotate = 0;
                m_zRotate = 0;
                m_slider = 0;
                m_POV = 0;
                m_buttons = NULL;
                m_currentJoystick = NULL;
                m_bEnabled = true;
            }

            ~CFlowJoystickNode()
            {
                SAFE_DELETE( m_buttons );

                if ( m_currentJoystick )
                {
                    m_currentJoystick->Unacquire();
                }
            }

            int CFlowJoystickNode::GetButtonState( IFunctionHandler* pH, int button )
            {
                if ( !m_buttons || button > m_CurrentCapabilities.dwButtons || button < 1 )
                {
                    return pH->EndFunction();
                }

                return pH->EndFunction( m_buttons[button - 1] );
            }

            int CFlowJoystickNode::GetNumButtons( IFunctionHandler* pH )
            {
                return pH->EndFunction( ( int )m_CurrentCapabilities.dwButtons );
            }

            int CFlowJoystickNode::SetJoystick( IFunctionHandler* pH, int joystick )
            {
                if ( joystick > g_joysticks.size() || joystick < 1 )
                {
                    SetNewJoystick( 1 );
                    return pH->EndFunction();
                }

                SetNewJoystick( joystick );

                return pH->EndFunction();
            }

            int CFlowJoystickNode::GetXAxis( IFunctionHandler* pH )
            {
                return pH->EndFunction( m_xAxis );
            }

            int CFlowJoystickNode::GetYAxis( IFunctionHandler* pH )
            {
                return pH->EndFunction( m_yAxis );
            }

            int CFlowJoystickNode::GetZAxis( IFunctionHandler* pH )
            {
                return pH->EndFunction( m_zAxis );
            }

            int CFlowJoystickNode::GetXRotate( IFunctionHandler* pH )
            {
                return pH->EndFunction( m_xRotate );
            }

            int CFlowJoystickNode::GetYRotate( IFunctionHandler* pH )
            {
                return pH->EndFunction( m_yRotate );
            }

            int CFlowJoystickNode::GetZRotate( IFunctionHandler* pH )
            {
                return pH->EndFunction( m_zRotate );
            }

            int CFlowJoystickNode::GetPOV( IFunctionHandler* pH )
            {
                return pH->EndFunction( m_POV );
            }

            int CFlowJoystickNode::GetSlider( IFunctionHandler* pH )
            {
                return pH->EndFunction( m_slider );
            }

            IFlowNodePtr Clone( SActivationInfo* pActInfo )
            {
                return new CFlowJoystickNode( pActInfo );
            }

            virtual void GetMemoryUsage( ICrySizer* s ) const
            {
                s->Add( *this );
            }

            void Serialize( SActivationInfo* pActInfo, TSerialize ser )
            {
                if ( ser.IsReading() )
                {
                    m_actInfo = *pActInfo;
                }
            }

            enum EInputPorts
            {
                EIP_Enabled = 0,
                EIP_JoystickSelection
            };

            enum EOutputPorts
            {
                EOP_JoystickX = 0,
                EOP_JoystickY,
                EOP_JoystickZ,
                EOP_JoystickRotateX,
                EOP_JoystickRotateY,
                EOP_JoystickRotateZ,
                EOP_Slider,
                EOP_POV,
                EOP_ButtonPressed,
                EOP_ButtonReleased
            };

            virtual void GetConfiguration( SFlowNodeConfig& config )
            {
                string joysticks = "enum_string:";
                int i = 1;

                for ( TJoysticks::const_iterator sit = g_joysticks.begin(); sit != g_joysticks.end(); ++sit )
                {
                    char buffer[32];
                    itoa( i, buffer, 10 );

                    joysticks.append( "Joystick" );
                    joysticks.append( buffer );
                    joysticks.append( "=" );
                    joysticks.append( buffer );
                    joysticks.append( "," );
                    i++;
                }

                static const SInputPortConfig inputs[] =
                {
                    InputPortConfig<bool>( "Enabled", true, _HELP( "Enables/Disables the joystick" ) ),
                    InputPortConfig<string>( "Joystick", "1", _HELP( "Select a joystick" ), "Joystick", _UICONFIG( joysticks ) ),
                    InputPortConfig_Null(),
                };

                static const SOutputPortConfig outputs[] =
                {
                    OutputPortConfig<int>( "X", _HELP( "X-axis, usually the left-right movement of a stick." ) ),
                    OutputPortConfig<int>( "Y", _HELP( "Y-axis, usually the forward-backward movement of a stick." ) ),
                    OutputPortConfig<int>( "Z", _HELP( "Z-axis, often the throttle control. If the joystick does not have this axis, the value is 0." ) ),
                    OutputPortConfig<int>( "X-Rotate", _HELP( "X-axis rotation. If the joystick does not have this axis, the value is 0." ) ),
                    OutputPortConfig<int>( "Y-Rotate", _HELP( "Y-axis rotation. If the joystick does not have this axis, the value is 0." ) ),
                    OutputPortConfig<int>( "Z-Rotate", _HELP( "Z-axis rotation (often called the rudder). If the joystick does not have this axis, the value is 0." ) ),
                    OutputPortConfig<int>( "Slider", _HELP( "Two additional axis values (formerly called the u-axis and v-axis) whose semantics depend on the joystick." ) ),
                    OutputPortConfig<int>( "POV", _HELP( "Direction controllers, such as point-of-view hats. The position is indicated in hundredths of a degree clockwise from north (away from the user). The center position is normally reported as - 1. For indicators that have only five positions, the value for a controller is - 1, 0, 9,000, 18,000, or 27,000." ) ),
                    OutputPortConfig<int>( "ButtonPressed", _HELP( "Joystick button pressed" ) ),
                    OutputPortConfig<int>( "ButtonReleased", _HELP( "Joystick button released" ) ),
                    OutputPortConfig_Null(),
                };

                config.pInputPorts = inputs;
                config.pOutputPorts = outputs;
                config.sDescription = _HELP( "Joystick infos" );
                config.SetCategory( EFLN_APPROVED );
            }

            void SetNewJoystick( int joystick )
            {
                if ( joystick == 0 )
                {
                    joystick = 1;
                }

                TJoysticks::iterator iter = g_joysticks.begin();

                if ( joystick == 1 && g_joysticks.size() == 1 )
                {
                    m_currentJoystick = iter->first;
                    m_CurrentCapabilities.dwSize = sizeof( DIDEVCAPS );
                    m_CurrentCapabilities = iter->second;

                    SAFE_DELETE( m_buttons );
                    m_buttons = new int[m_CurrentCapabilities.dwButtons];
                }

                else if ( g_joysticks.size() > 1 )
                {
                    TJoysticks::iterator end = g_joysticks.end();

                    int i = 2;
                    ++iter;

                    for ( ; iter != end; ++iter )
                    {
                        if ( i == joystick )
                        {
                            break;
                        }

                        i++;
                    }

                    m_currentJoystick = iter->first;
                    m_CurrentCapabilities.dwSize = sizeof( DIDEVCAPS );
                    m_CurrentCapabilities = iter->second;

                    SAFE_DELETE( m_buttons );
                    m_buttons = new int[m_CurrentCapabilities.dwButtons];
                }
            }

            virtual void ProcessEvent( EFlowEvent event, SActivationInfo* pActInfo )
            {
                switch ( event )
                {
                    case eFE_Initialize:
                        {
                            m_actInfo = *pActInfo;
                            m_bEnabled = true;
                            pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, true );
                            SetNewJoystick( atoi( GetPortString( &m_actInfo, EIP_JoystickSelection ) ) );

                            break;
                        }

                    case eFE_Activate:
                        {
                            if ( IsPortActive( pActInfo, EIP_Enabled ) )
                            {
                                m_bEnabled = GetPortBool( pActInfo, EIP_Enabled );
                            }

                            else if ( IsPortActive( pActInfo, EIP_JoystickSelection ) )
                            {
                                SetNewJoystick( atoi( GetPortString( &m_actInfo, EIP_JoystickSelection ) ) );
                            }

                            break;
                        }

                    case eFE_Update:
                        {
                            if ( m_currentJoystick == NULL || m_buttons == NULL || !m_bEnabled )
                            {
                                return;
                            }

                            DIJOYSTATE2 js;

                            if ( PollJoystick( &js, m_currentJoystick ) == DI_OK )
                            {
                                m_xAxis = static_cast<int>( js.lX );
                                m_yAxis = static_cast<int>( js.lY );
                                m_zAxis = static_cast<int>( js.lZ );

                                m_xRotate = static_cast<int>( js.lRx );
                                m_yRotate = static_cast<int>( js.lRy );
                                m_zRotate = static_cast<int>( js.lRz );

                                m_slider = static_cast<int>( js.rglSlider[0] );
                                m_POV = static_cast<int>( js.rgdwPOV[0] );

                                ActivateOutput( pActInfo, EOP_JoystickX, m_xAxis );
                                ActivateOutput( pActInfo, EOP_JoystickY, m_yAxis );
                                ActivateOutput( pActInfo, EOP_JoystickZ, m_zAxis );

                                ActivateOutput( pActInfo, EOP_JoystickRotateX, m_xRotate );
                                ActivateOutput( pActInfo, EOP_JoystickRotateY, m_yRotate );
                                ActivateOutput( pActInfo, EOP_JoystickRotateZ, m_zRotate );

                                ActivateOutput( pActInfo, EOP_Slider, m_slider );
                                ActivateOutput( pActInfo, EOP_POV, m_POV );

                                for ( int i = 0; i < m_CurrentCapabilities.dwButtons; i++ )
                                {
                                    BYTE button = js.rgbButtons[i];

                                    if ( button & 0x80 )
                                    {
                                        if ( m_buttons[i] != 1 )
                                        {
                                            m_buttons[i] = 1;
                                        }

                                        ActivateOutput( pActInfo, EOP_ButtonPressed, i + 1 );
                                    }

                                    else
                                    {
                                        if ( m_buttons[i] == 1 )
                                        {
                                            m_buttons[i] = 0;
                                            ActivateOutput( pActInfo, EOP_ButtonReleased, i + 1 );
                                        }
                                    }
                                }
                            }

                            break;
                        }
                }
            }

            virtual void GetMemoryStatistics( ICrySizer* s )
            {
                s->Add( *this );
            }
    };
}

REGISTER_FLOW_NODE_EX( "Plugin_Joystick:Input", JoystickPlugin::CFlowJoystickNode, CFlowJoystickNode );