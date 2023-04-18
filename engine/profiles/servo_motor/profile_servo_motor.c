/******************************************************************************
 * @file profile_motor
 * @brief servo_motor profile
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "profile_servo_motor.h"
#include "luos_hal.h"

/******************************************************************************
 * @brief Initialization function
 * @param handler : Pointer the servo motor handler to initialize
 * @return None
 ******************************************************************************/
void ProfileServo_Init(void *handler)
{
    profile_core_t *profile                    = (profile_core_t *)handler;
    profile_servo_motor_t *servo_motor_profile = (profile_servo_motor_t *)profile->profile_data;

    servo_motor_profile->motor.mode.current        = servo_motor_profile->mode.current;
    servo_motor_profile->motor.mode.mode_compliant = servo_motor_profile->mode.mode_compliant;
    servo_motor_profile->motor.mode.temperature    = servo_motor_profile->mode.temperature;
}

/******************************************************************************
 * @brief Function converting Luos messages into data and reverse.
 * @param service : Service the target service
 * @param msg : The received message
 * @return None
 ******************************************************************************/
void ProfileServo_Handler(service_t *service, const msg_t *msg)
{
    // get profile informations from service
    profile_core_t *profile                    = ProfileCore_GetFromService(service);
    profile_servo_motor_t *servo_motor_profile = (profile_servo_motor_t *)profile->profile_data;

    switch (msg->header.cmd)
    {
        case GET_CMD:
        {
            // Report management
            msg_t pub_msg;
            pub_msg.header.target_mode = msg->header.target_mode;
            pub_msg.header.target      = msg->header.source;
            if (servo_motor_profile->mode.angular_position)
            {
                if (servo_motor_profile->control.rec)
                {
                    LUOS_ASSERT(servo_motor_profile->measurement.data_ptr != 0);
                    // send back a record stream
                    pub_msg.header.cmd = ANGULAR_POSITION;
                    Luos_SendStreaming(service, &pub_msg, &servo_motor_profile->measurement);
                }
                else
                {
                    LuosHAL_SetIrqState(false);
                    AngularOD_PositionToMsg((angular_position_t *)&servo_motor_profile->angular_position, &pub_msg);
                    LuosHAL_SetIrqState(true);
                    Luos_SendMsg(service, &pub_msg);
                }
            }
            if (servo_motor_profile->mode.angular_speed)
            {
                AngularOD_SpeedToMsg((angular_speed_t *)&servo_motor_profile->angular_speed, &pub_msg);
                Luos_SendMsg(service, &pub_msg);
            }
            if (servo_motor_profile->mode.linear_position)
            {
                LinearOD_PositionToMsg((linear_position_t *)&servo_motor_profile->linear_position, &pub_msg);
                Luos_SendMsg(service, &pub_msg);
            }
            if (servo_motor_profile->mode.linear_speed)
            {
                LinearOD_SpeedToMsg((linear_speed_t *)&servo_motor_profile->linear_speed, &pub_msg);
                Luos_SendMsg(service, &pub_msg);
            }
        }
        break;
        case PID:
        {
            // fill the message infos
            if ((servo_motor_profile->mode.mode_angular_position || servo_motor_profile->mode.mode_linear_position) && !(servo_motor_profile->mode.mode_angular_speed || servo_motor_profile->mode.mode_linear_speed))
            {
                // only position control is enable, we can save PID for positioning
                PidOD_PidFromMsg(&servo_motor_profile->position_pid, msg);
            }
            if ((servo_motor_profile->mode.mode_angular_speed || servo_motor_profile->mode.mode_linear_speed) && !(servo_motor_profile->mode.mode_angular_position || servo_motor_profile->mode.mode_linear_position))
            {
                // only speed control is enable, we can save PID for speed
                PidOD_PidFromMsg(&servo_motor_profile->speed_pid, msg);
            }
        }
        break;
        case CONTROL:
        {
            ControlOD_ControlFromMsg(&servo_motor_profile->control, msg);
        }
        break;
        case RESOLUTION:
        {
            // set the encoder resolution
            memcpy((void *)&servo_motor_profile->resolution, msg->data, sizeof(float));
        }
        break;
        case REDUCTION:
        {
            // set the motor reduction
            memcpy((void *)&servo_motor_profile->motor_reduction, msg->data, sizeof(float));
        }
        break;
        case REINIT:
        {
            // set state to 0
            LuosHAL_SetIrqState(false);
            servo_motor_profile->angular_position        = AngularOD_PositionFrom_deg(0.0);
            servo_motor_profile->target_angular_position = AngularOD_PositionFrom_deg(0.0);
            LuosHAL_SetIrqState(true);
        }
        break;
        case DIMENSION:
        {
            // set the wheel diameter m
            LinearOD_PositionFromMsg((linear_position_t *)&servo_motor_profile->wheel_diameter, msg);
        }
        break;
        case ANGULAR_POSITION:
        {
            if (servo_motor_profile->mode.mode_angular_position)
            {
                // Check message size
                if (msg->header.size == sizeof(angular_position_t))
                {
                    // set the motor target angular position
                    LuosHAL_SetIrqState(false);
                    AngularOD_PositionFromMsg((angular_position_t *)&servo_motor_profile->target_angular_position, msg);
                    LuosHAL_SetIrqState(true);
                }
                else
                {
                    LUOS_ASSERT(servo_motor_profile->trajectory.data_ptr != 0);
                    // this is a trajectory, save it into streaming channel.
                    Luos_ReceiveStreaming(service, msg, &servo_motor_profile->trajectory);
                }
            }
        }
        break;
        case ANGULAR_SPEED:
        {
            // set the motor target angular position
            if (servo_motor_profile->mode.mode_angular_speed)
            {
                AngularOD_SpeedFromMsg((angular_speed_t *)&servo_motor_profile->target_angular_speed, msg);
            }
        }
        break;
        case LINEAR_POSITION:
        {
            // set the motor target linear position
            // Check message size
            if (msg->header.size == sizeof(linear_position_t))
            {
                // Get the position
                linear_position_t linear_position = LinearOD_PositionFrom_m(0.0);
                LinearOD_PositionFromMsg(&linear_position, msg);
                // Convert it directly into angle
                servo_motor_profile->target_angular_position = AngularOD_PositionFrom_deg((LinearOD_PositionTo_m(linear_position) * 360.0) / (3.141592653589793 * LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter)));
            }
            else
            {
                LUOS_ASSERT(servo_motor_profile->trajectory.data_ptr != 0);
                // this is a trajectory, save it into ring buffer.
                Luos_ReceiveStreaming(service, msg, &servo_motor_profile->trajectory);
                // values will be converted one by one during trajectory management.
            }
        }
        break;
        case LINEAR_SPEED:
        {
            // set the motor target linear speed
            if (LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter) > 0.0)
            {
                // Get the linear speed
                linear_speed_t linear_speed = LinearOD_SpeedFrom_m_s(0.0);
                LinearOD_SpeedFromMsg(&linear_speed, msg);
                // Convert it directly
                servo_motor_profile->target_angular_speed = AngularOD_SpeedFrom_deg_s((LinearOD_SpeedTo_m_s(linear_speed) * 360.0) / (3.141592653589793 * LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter)));
            }
        }
        break;
        case ANGULAR_POSITION_LIMIT:
        {
            // set the motor limit anglular position
            AngularOD_PositionFromMsg((angular_position_t *)&servo_motor_profile->limit_angular_position, msg);
        }
        break;
        case ANGULAR_SPEED_LIMIT:
        {
            // set the motor angular speed limit
            AngularOD_SpeedFromMsg((angular_speed_t *)&servo_motor_profile->limit_angular_speed, msg);
            return;
        }
        break;
        case LINEAR_POSITION_LIMIT:
        {
            // set the motor target linear position
            if (servo_motor_profile->mode.mode_linear_position & (LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter) != 0))
            {
                // Get the linear positions limit
                linear_position_t linear_position[2] = {{0.0}, {0.0}};
                LinearOD_PositionFromMsg((linear_position_t *)&linear_position[0], msg);
                // Directly convert it
                servo_motor_profile->limit_angular_position[0] = AngularOD_PositionFrom_deg((LinearOD_PositionTo_m(linear_position[0]) * 360.0) / (3.141592653589793 * LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter)));
                servo_motor_profile->limit_angular_position[1] = AngularOD_PositionFrom_deg((LinearOD_PositionTo_m(linear_position[1]) * 360.0) / (3.141592653589793 * LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter)));
            }
        }
        break;
        case LINEAR_SPEED_LIMIT:
        {
            // set the motor linear speed limit
            if (servo_motor_profile->mode.mode_linear_position & (LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter) != 0))
            {
                linear_speed_t linear_speed[2] = {{0.0}, {0.0}};
                LinearOD_SpeedFromMsg((linear_speed_t *)&linear_speed[0], msg);
                // Directly convert it
                servo_motor_profile->limit_angular_speed[0] = AngularOD_SpeedFrom_deg_s((LinearOD_SpeedTo_m_s(linear_speed[0]) * 360.0) / (3.141592653589793 * LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter)));
                servo_motor_profile->limit_angular_speed[1] = AngularOD_SpeedFrom_deg_s((LinearOD_SpeedTo_m_s(linear_speed[1]) * 360.0) / (3.141592653589793 * LinearOD_PositionTo_m(servo_motor_profile->wheel_diameter)));
            }
        }
        break;
        case TIME:
        {
            // save time in ms
            TimeOD_TimeFromMsg((time_luos_t *)&servo_motor_profile->sampling_period, msg);
        }
        break;
        case PARAMETERS:
        {
            // ************************************************************************* //
            // This message overrides connected profile command i.e MOTOR PARAMETERS
            // OVERRIDE_CONNECTED_HANDLER() must be called in this case
            // ************************************************************************* //
            ProfileCore_OverrideConnectHandler();

            // fill the message infos
            memcpy((void *)&servo_motor_profile->mode, msg->data, sizeof(servo_motor_mode_t));

            // get child information from servo
            servo_motor_profile->motor.mode.current        = servo_motor_profile->mode.current;
            servo_motor_profile->motor.mode.mode_compliant = servo_motor_profile->mode.mode_compliant;
            servo_motor_profile->motor.mode.temperature    = servo_motor_profile->mode.temperature;

            // manage specific configuration operations
            if (servo_motor_profile->mode.mode_compliant == 0)
            {
                servo_motor_profile->target_angular_position = servo_motor_profile->angular_position;
            }
        }
        break;
        default:
        {
        }
        break;
    }

    if ((profile->profile_ops.Callback != 0))
    {
        profile->profile_ops.Callback(service, msg);
    }
}

/******************************************************************************
 * @brief Link profile to the general profile handler
 * @param profile_mode : HEAD / CONNECT
 * @param profile : profile data structure
 * @return None
 ******************************************************************************/
void ProfileServo_link(uint8_t profile_mode, profile_servo_motor_t *profile_servo_motor)
{
    profile_core_t *profile      = ProfileCore_GetNew(profile_mode);
    profile->type                = SERVO_MOTOR_TYPE;
    profile->profile_data        = (void *)profile_servo_motor;
    profile->profile_ops.Init    = ProfileServo_Init;
    profile->profile_ops.Handler = ProfileServo_Handler;
}

/******************************************************************************
 * @brief Create a service with a linked profile
 * @param profile : Profile data structure
 * @param callback : Callback used by the service
 * @param alias : Alias for the service (15 caracters max string)
 * @param revision : FW for the service (tab[MajorVersion,MinorVersion,Patch])
 * @return service pointer
 ******************************************************************************/
service_t *ProfileServo_CreateService(profile_servo_motor_t *profile_servo_motor, SERVICE_CB callback, const char *alias, revision_t revision)
{
    // link head profile
    ProfileServo_link(HEAD_PROFILE, profile_servo_motor);

    // connect a profile to head profile
    ProfileMotor_link(CONNECT_PROFILE, &profile_servo_motor->motor);

    // Start service with the linked profile
    return ProfileCore_StartService(callback, alias, revision);
}
