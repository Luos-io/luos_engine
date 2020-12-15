#include "template_servo_motor.h"
#include "profile_servo_motor.h"
#include "profile_motor.h"
#include "luos_hal.h"
/******************************************************************************
 * @brief function converting Luos messages innto data and reverse.
 * @param container the target container
 * @param msg the received message
 * @param profile_servo_motor the data struct to update
 * @return None
 ******************************************************************************/
static void ServoMotorConfig_Handler(container_t *container, msg_t *msg, profile_servo_motor_t *profile_servo_motor)
{
    if (msg->header.cmd == PARAMETERS)
    {
        // fill the message infos
        memcpy((void *)&profile_servo_motor->mode, msg->data, sizeof(servo_motor_mode_t));

        // copy motor specific configuration into motor object
        profile_servo_motor->motor.mode.current        = profile_servo_motor->mode.current;
        profile_servo_motor->motor.mode.mode_compliant = profile_servo_motor->mode.mode_compliant;
        profile_servo_motor->motor.mode.temperature    = profile_servo_motor->mode.temperature;

        // manage specific configuration operations
        if (profile_servo_motor->mode.mode_compliant == 0)
        {
            profile_servo_motor->target_angular_position = profile_servo_motor->angular_position;
        }
    }
}
/******************************************************************************
 * @brief Msg Handler call backed by Luos when a msg receive for this container
 * @param Container destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void TemplateServoMotor_MsgHandler(container_t *container, msg_t *msg)
{
    template_servo_motor_t *servo_motor_template = (template_servo_motor_t *)container->template_context;
    ServoMotorConfig_Handler(container, msg, &servo_motor_template->profile);
    ProfileServoMotor_Handler(container, msg, &servo_motor_template->profile);
    if (servo_motor_template->self != 0)
    {
        servo_motor_template->self(container, msg);
    }
}
/******************************************************************************
 * @brief Container creation following the template
 * @param cont_cb is an optional user callback called on every massage for this container
 * @param servo_motor_template template object pointer
 * @param alias for the container string (15 caracters max).
 * @param revision FW for the container (tab[MajorVersion,MinorVersion,Patch])
 * @return None
 ******************************************************************************/
container_t *TemplateServoMotor_CreateContainer(CONT_CB cont_cb, template_servo_motor_t *servo_motor_template, const char *alias, revision_t revision)
{
    servo_motor_template->self = cont_cb;
    // copy motor specific configuration into motor object
    servo_motor_template->profile.motor.mode.current        = servo_motor_template->profile.mode.current;
    servo_motor_template->profile.motor.mode.mode_compliant = servo_motor_template->profile.mode.mode_compliant;
    servo_motor_template->profile.motor.mode.temperature    = servo_motor_template->profile.mode.temperature;

    container_t *container      = Luos_CreateContainer(TemplateServoMotor_MsgHandler, SERVO_MOTOR_TYPE, alias, revision);
    container->template_context = (void *)servo_motor_template;
    return container;
}
/******************************************************************************
 * @brief function converting Luos messages innto data and reverse.
 * @param container the target container
 * @param msg the received message
 * @param servo_motor_profile the data struct to update
 * @return None
 ******************************************************************************/
void ProfileServoMotor_Handler(container_t *container, msg_t *msg, profile_servo_motor_t *servo_motor_profile)
{
    switch (msg->header.cmd)
    {
        case ASK_PUB_CMD:
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
                    Luos_SendStreaming(container, &pub_msg, &servo_motor_profile->measurement);
                }
                else
                {
                    LuosHAL_SetIrqState(false);
                    AngularOD_PositionToMsg((angular_position_t *)&servo_motor_profile->angular_position, &pub_msg);
                    LuosHAL_SetIrqState(true);
                    Luos_SendMsg(container, &pub_msg);
                }
            }
            if (servo_motor_profile->mode.angular_speed)
            {
                AngularOD_SpeedToMsg((angular_speed_t *)&servo_motor_profile->angular_speed, &pub_msg);
                Luos_SendMsg(container, &pub_msg);
            }
            if (servo_motor_profile->mode.linear_position)
            {
                LinearOD_PositionToMsg((linear_position_t *)&servo_motor_profile->linear_position, &pub_msg);
                Luos_SendMsg(container, &pub_msg);
            }
            if (servo_motor_profile->mode.linear_speed)
            {
                LinearOD_SpeedToMsg((linear_speed_t *)&servo_motor_profile->linear_speed, &pub_msg);
                Luos_SendMsg(container, &pub_msg);
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
            __disable_irq();
            servo_motor_profile->angular_position        = 0.0;
            servo_motor_profile->target_angular_position = 0.0;
            __enable_irq();
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
                    __disable_irq();
                    AngularOD_PositionFromMsg((angular_position_t *)&servo_motor_profile->target_angular_position, msg);
                    __enable_irq();
                }
                else
                {
                    LUOS_ASSERT(servo_motor_profile->trajectory.data_ptr != 0);
                    // this is a trajectory, save it into streaming channel.
                    Luos_ReceiveStreaming(container, msg, &servo_motor_profile->trajectory);
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
                linear_position_t linear_position = 0.0;
                LinearOD_PositionFromMsg(&linear_position, msg);
                // Convert it directly into angle
                servo_motor_profile->target_angular_position = (linear_position * 360.0) / (3.141592653589793 * servo_motor_profile->wheel_diameter);
            }
            else
            {
                LUOS_ASSERT(servo_motor_profile->trajectory.data_ptr != 0);
                // this is a trajectory, save it into ring buffer.
                Luos_ReceiveStreaming(container, msg, &servo_motor_profile->trajectory);
                // values will be converted one by one during trajectory management.
            }
        }
        break;
        case LINEAR_SPEED:
        {
            // set the motor target linear speed
            if (servo_motor_profile->wheel_diameter > 0.0)
            {
                // Get the linear speed
                linear_speed_t linear_speed = 0.0;
                LinearOD_SpeedFromMsg(&linear_speed, msg);
                // Convert it directly
                servo_motor_profile->target_angular_speed = (linear_speed * 360.0) / (3.141592653589793 * servo_motor_profile->wheel_diameter);
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
            if (servo_motor_profile->mode.mode_linear_position & (servo_motor_profile->wheel_diameter != 0))
            {
                // Get the linear positions limit
                linear_position_t linear_position[2] = {0.0, 0.0};
                LinearOD_PositionFromMsg((linear_position_t *)&linear_position[0], msg);
                // Directly convert it
                servo_motor_profile->limit_angular_position[0] = (linear_position[0] * 360.0) / (3.141592653589793 * servo_motor_profile->wheel_diameter);
                servo_motor_profile->limit_angular_position[1] = (linear_position[1] * 360.0) / (3.141592653589793 * servo_motor_profile->wheel_diameter);
            }
        }
        break;
        case LINEAR_SPEED_LIMIT:
        {
            // set the motor linear speed limit
            if (servo_motor_profile->mode.mode_linear_position & (servo_motor_profile->wheel_diameter != 0))
            {
                linear_speed_t linear_speed[2] = {0.0, 0.0};
                LinearOD_SpeedFromMsg((linear_speed_t *)&linear_speed[0], msg);
                // Directly convert it
                servo_motor_profile->limit_angular_speed[0] = (linear_speed[0] * 360.0) / (3.141592653589793 * servo_motor_profile->wheel_diameter);
                servo_motor_profile->limit_angular_speed[1] = (linear_speed[1] * 360.0) / (3.141592653589793 * servo_motor_profile->wheel_diameter);
            }
        }
        break;
        case TIME:
        {
            // save time in ms
            TimeOD_TimeFromMsg((time_luos_t *)&servo_motor_profile->sampling_period, msg);
        }
        break;
    }
    ProfileMotor_Handler(container, msg, &servo_motor_profile->motor);
}
