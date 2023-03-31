/******************************************************************************
 * @file alarm controler
 * @brief application example an alarm controler
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "ledstrip_position.h"
#include "main.h"
#include <math.h>
#include "product_config.h"
#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define FRAMERATE_MS           10
#define STRIP_LENGTH           0.433
#define SPACE_BETWEEN_LEDS     0.0067
#define LED_NUMBER             (uint16_t)(STRIP_LENGTH / SPACE_BETWEEN_LEDS)
#define MAXRADIUS              0.05
#define MAX_DISTANCE_UPDATE_MS 30
#define DIST_OFFSET            0.03
#define STARTUP_DELAY_MS       100
/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *app;
uint8_t position = 0;
// distance occupied from sensor variables
linear_position_t distance;
linear_position_t raw_distance;
// image variables
volatile color_t image[LED_NUMBER];
// Display modes
ledstrip_position_OperationMode_t parameter = DISTANCE_DISPLAY;
// Distance display mode variables
float radius             = 0.0;
volatile int motor_found = 0;
bool motor_run_mode      = false;
bool detection_animation = false;
bool end_detection       = false;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void LedStripPosition_MsgHandler(service_t *service, msg_t *msg);
static void distance_filtering(void);
static void distance_frame_compute(void);
static void glowing_fade(float target);
static void sort_motors(void);
// Display modes
static void distance_based_display(int led_strip_id);
static bool detection_display(int led_strip_id);
static void motor_copy_display(int led_strip_id);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void LedStripPosition_Init(void)
{
    distance            = LinearOD_PositionFrom_mm(-1);
    raw_distance        = LinearOD_PositionFrom_mm(-1);
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Create App
    app = Luos_CreateService(LedStripPosition_MsgHandler, LEDSTRIP_POSITION_APP, "ledstrip_pos", revision);
    // initialize image to 0
    memset((void *)image, 0, LED_NUMBER * sizeof(color_t));
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void LedStripPosition_Loop(void)
{
    static uint32_t lastframe_time_ms = 0;
    search_result_t result;
    // Check if we have done the first init or if service Id have changed
    if (Luos_IsDetected())
    {
        if (end_detection)
        {
            search_result_t result;
            motor_run_mode = false;
            sort_motors();
            // A detection just finished
            // Make services configurations
            // try to find a distance sensor
            RTFilter_Type(RTFilter_Reset(&result), DISTANCE_TYPE);
            if (result.result_nbr > 0)
            {
                // Setup auto update each UPDATE_PERIOD_MS on imu
                // This value is resetted on all service at each detection
                // It's important to setting it each time.
                msg_t msg;
                msg.header.target      = result.result_table[0]->id;
                msg.header.target_mode = SERVICEIDACK;
                time_luos_t time       = TimeOD_TimeFrom_ms(MAX_DISTANCE_UPDATE_MS);
                TimeOD_TimeToMsg(&time, &msg);
                msg.header.cmd = UPDATE_PUB;
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
                // Reset detection animation
                detection_display(0);
            }
            // Start the detection animation
            detection_animation = true;
            end_detection       = false;
            lastframe_time_ms   = Luos_GetSystick();
        }
        // ********** frame management ************
        // Update the frame
        if (Luos_GetSystick() - lastframe_time_ms >= FRAMERATE_MS)
        {
            distance_filtering();
            RTFilter_Type(RTFilter_Reset(&result), COLOR_TYPE);
            // Check if there is a led_strip detected
            if (result.result_nbr > 0)
            {
                if (detection_animation)
                {
                    detection_animation = detection_display(result.result_table[0]->id);
                }
                else if (parameter == DISTANCE_DISPLAY)
                {
                    distance_based_display(result.result_table[0]->id);
                }
                else if (parameter == MOTOR_COPY_DISPLAY)
                {
                    motor_copy_display(result.result_table[0]->id);
                }
            }
            lastframe_time_ms = Luos_GetSystick();
        }
    }
    else
    {
        // someone is making a detection, let it finish.
        // reset the init state to be ready to setup service at the end of detection
        parameter = DISTANCE_DISPLAY;
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void LedStripPosition_MsgHandler(service_t *service, msg_t *msg)
{
    search_result_t services_list;
    RTFilter_Reset(&services_list);
    if ((services_list.result_table[msg->header.source]->type == DISTANCE_TYPE)) // && (msg->header.cmd == LINEAR_POSITION))
    {
        if (msg->header.cmd == LINEAR_POSITION)
        {
            // receive the distance sensor value
            LinearOD_PositionFromMsg(&raw_distance, msg);
            raw_distance = LinearOD_PositionFrom_m(LinearOD_PositionTo_m(raw_distance) - DIST_OFFSET);
            if ((LinearOD_PositionTo_m(raw_distance) > STRIP_LENGTH) || (LinearOD_PositionTo_m(raw_distance) < 0))
            {
                raw_distance = LinearOD_PositionFrom_mm(-1);
            }
            return;
        }
    }
    else if ((msg->header.cmd == GET_CMD) && (services_list.result_table[msg->header.source]->type == RUN_MOTOR))
    {
        // motor application asks which position of the led_strip is lightened - respond
        msg_t pub_msg;
        pub_msg.header.target_mode = SERVICEID;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.cmd         = SET_CMD;
        pub_msg.header.size        = 1;
        pub_msg.data[0]            = position;
        while (Luos_SendMsg(app, &pub_msg) == FAILED)
        {
            Luos_Loop();
        }
        motor_run_mode = true;
    }
    else if (msg->header.cmd == PARAMETERS)
    {
        parameter = msg->data[0];
    }
    else if (msg->header.cmd == SET_CMD)
    {
        position = msg->data[0];
        if (position == NO_MOTOR)
        {
            parameter = DISTANCE_DISPLAY;
        }
        else
        {
            parameter = MOTOR_COPY_DISPLAY;
        }
        motor_run_mode = false;
    }
    else if (msg->header.cmd == END_DETECTION)
    {
        end_detection = true;
    }
}

void distance_filtering(void)
{
    // Linear movement
    const float filtering_strength = 0.04;
    const float inertia_strength   = 0.03;
    const float max_speed          = 0.3;

    // Filtering variables
    static linear_position_t prev_distance  = {0.0};
    static linear_position_t inertial_force = {0.0};

    // Clear filter when hand is removed
    if (LinearOD_PositionTo_mm(raw_distance) < 1.0)
    {
        distance       = prev_distance;
        inertial_force = LinearOD_PositionFrom_m(0.0);
        // Glowing fade out
        glowing_fade(0.0);
    }
    // Start filter when hand is present
    else if (radius < 0.00001)
    {
        prev_distance = raw_distance;
        distance      = raw_distance;
        // Glowing fade in
        glowing_fade(MAXRADIUS);
    }
    else
    {
        // Glowing fade in
        glowing_fade(MAXRADIUS);

        // Compute the error between the light and the real hand position
        float position_err = LinearOD_PositionTo_m(raw_distance) - LinearOD_PositionTo_m(prev_distance);

        // Compute inertial delta force (integral)
        inertial_force = LinearOD_PositionFrom_m(LinearOD_PositionTo_m(inertial_force) + position_err);
        // Inertia clamping
        if (LinearOD_PositionTo_m(inertial_force) < -max_speed)
            inertial_force = LinearOD_PositionFrom_m(-max_speed);
        if (LinearOD_PositionTo_m(inertial_force) > max_speed)
            inertial_force = LinearOD_PositionFrom_m(max_speed);

        // Then filter the position to give an inertia effect
        distance = LinearOD_PositionFrom_m(LinearOD_PositionTo_m(prev_distance) + (filtering_strength * position_err) + (inertia_strength * LinearOD_PositionTo_m(inertial_force)));
    }
    prev_distance = distance;
}

void glowing_fade(float target)
{
    const float filtering_strength = 0.06;
    const float inertia_strength   = 0.04;
    // Radial glowing
    const float max_radius_speed = 4.0;

    // Filtering variables
    static float inertial_force = 0.0;
    // Compute the error between the target and the actual radius
    float radius_err = target - radius;

    // Compute inertial delta force (integral)
    inertial_force += radius_err;
    // Inertia clamping
    if (inertial_force < -max_radius_speed)
        inertial_force = -max_radius_speed;
    if (inertial_force > max_radius_speed)
        inertial_force = max_radius_speed;

    // Then filter the radius to give an inertia effect
    radius = radius + (filtering_strength * radius_err) + (inertia_strength * inertial_force);
    if (radius < 0.0)
    {
        radius   = 0.0;
        distance = LinearOD_PositionFrom_m(-0.01);
    }
}

void distance_frame_compute(void)
{
    // memset((void *)&image[(uint16_t)(distance / SPACE_BETWEEN_LEDS)], 200, sizeof(color_t));
    const uint16_t radius_led_number = (uint16_t)round((radius) / SPACE_BETWEEN_LEDS) + 1;
    const int max_intensity          = 200;
    uint16_t middle_led              = (uint16_t)(LinearOD_PositionTo_m(distance) / SPACE_BETWEEN_LEDS);
    for (int i = (middle_led - radius_led_number); i < (middle_led + radius_led_number); i++)
    {
        // Conpute the real position in mm of this led
        float real_position = i * SPACE_BETWEEN_LEDS;
        // Parabolic
        // int intensity = max_intensity * (1 - ((real_position - distance) * (real_position - distance) / (radius * radius)));
        // Linear
        int intensity = max_intensity * (1 - (fabs(real_position - LinearOD_PositionTo_m(distance)) / (radius)));
        if ((intensity > 0) && (i < LED_NUMBER) && (i > 0))
        {
            image[i].b = (uint8_t)intensity;
            image[i].g = (uint8_t)intensity;
            image[i].r = (uint8_t)intensity;
        }
    }
    // Add a light indicating the activ motor
    float motor_position = (position * (STRIP_LENGTH / 3.0)) - (STRIP_LENGTH / 6.0);
    if (motor_position > 0.0)
    {
        if ((position <= motor_found) && (motor_run_mode))
        {
            uint16_t motor_led = (uint16_t)(motor_position / SPACE_BETWEEN_LEDS);
            image[motor_led].b = 0;
            image[motor_led].g = (uint8_t)(81.0 * ((float)max_intensity / 255.0));
            image[motor_led].r = (uint8_t)(255.0 * ((float)max_intensity / 255.0));
        }
    }
}

void distance_based_display(int led_strip_id)
{
    // Check if the distance is in the led strip length
    if ((LinearOD_PositionTo_m(distance) > 0.0) && (LinearOD_PositionTo_m(distance) < STRIP_LENGTH))
    {
        // Image to light the region of the object detected
        // Compute a frame
        distance_frame_compute();

        // Check in which region there is an object
        if (LinearOD_PositionTo_m(distance) <= (STRIP_LENGTH / 3.0))
        {
            position = MOTOR_1_POSITION;
        }
        else if (LinearOD_PositionTo_m(distance) <= 2 * (STRIP_LENGTH / 3.0))
        {
            position = MOTOR_2_POSITION;
        }
        else if (LinearOD_PositionTo_m(distance) <= STRIP_LENGTH)
        {
            position = MOTOR_3_POSITION;
        }
    }
    else
    {
        // no region should be lighted - sensor has not detected sth
        position = NO_MOTOR;
    }
    // send the created image to the led_strip
    msg_t msg;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.target      = led_strip_id;
    msg.header.cmd         = COLOR;
    Luos_SendData(app, &msg, &image[0], sizeof(color_t) * LED_NUMBER);
    // reinitialize the image so that the led_strip is not lighted by default
    memset((void *)image, 0, LED_NUMBER * sizeof(color_t));
}

bool detection_display(int led_strip_id)
{
    const float start_speed            = 3.0;
    const float minimal_speed          = 0.5;
    const float speed_evolution_factor = 0.95;
    const linear_position_t trail_size = {0.2};
    const int max_intensity            = 200;

    static linear_speed_t speed = {3.0};
    static float light_position = 0.0;

    if (led_strip_id == 0)
    {
        // This is not a good value just reset state
        speed          = LinearOD_SpeedFrom_m_s(start_speed);
        light_position = 0.0;
        return false;
    }

    // Compute the new light _position based on the current speed
    light_position = light_position + (LinearOD_SpeedTo_m_s(speed) * ((float)FRAMERATE_MS / 1000.0));
    // Compute new speed
    speed = LinearOD_SpeedFrom_m_s(LinearOD_SpeedTo_m_s(speed) * speed_evolution_factor);
    // Speed clamping
    if (LinearOD_SpeedTo_m_s(speed) < minimal_speed)
    {
        speed = LinearOD_SpeedFrom_m_s(minimal_speed);
    }

    const uint16_t led_position = (uint16_t)(light_position / SPACE_BETWEEN_LEDS);

    // Compute the image
    for (int i = 0; i < (LinearOD_PositionTo_m(trail_size) / SPACE_BETWEEN_LEDS); i++)
    {
        // Compute the real position in mm of this led
        float real_position = (led_position - i) * SPACE_BETWEEN_LEDS;
        // Linear
        int intensity = max_intensity * (1 - (fabs(real_position - light_position) / LinearOD_PositionTo_m(trail_size)));
        if ((intensity > 0) && ((led_position - i) < LED_NUMBER) && ((led_position - i) > 0))
        {
            image[led_position - i].b = (uint8_t)intensity;
            image[led_position - i].g = (uint8_t)intensity;
            image[led_position - i].r = (uint8_t)intensity;
        }
    }
    // send the created image to the led_strip
    msg_t msg;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.target      = led_strip_id;
    msg.header.cmd         = COLOR;
    Luos_SendData(app, &msg, &image[0], sizeof(color_t) * LED_NUMBER);
    // reinitialize the image so that the led_strip is not lighted by default
    memset((void *)image, 0, LED_NUMBER * sizeof(color_t));

    if ((light_position - LinearOD_PositionTo_m(trail_size)) > STRIP_LENGTH)
    {
        // This is the end of this animation
        // reset values
        speed          = LinearOD_SpeedFrom_m_s(start_speed);
        light_position = 0.0;
        return false;
    }
    return true;
}

void motor_copy_display(int led_strip_id)
{

    const int max_intensity                = 200;
    const float chenillard_space           = 0.05;
    const float chenillard_ratio_speed     = 0.04;
    static float chenillard_position_ratio = 0.0;

    if (position > 0)
    {

        // Add a light indicating the master motor
        float motor_position = (position * (STRIP_LENGTH / 3.0)) - (STRIP_LENGTH / 6.0);
        if (motor_position > 0.0)
        {
            float last_motor_position = ((motor_found * (STRIP_LENGTH / 3.0)) - (STRIP_LENGTH / 6.0));
            for (float i = (motor_position + (chenillard_space * chenillard_position_ratio)); i < last_motor_position; i += chenillard_space)
            {
                uint16_t motor_led = (uint16_t)(i / SPACE_BETWEEN_LEDS);
                if (motor_led < LED_NUMBER)
                {
                    image[motor_led].b = (uint8_t)(189.0 * ((float)max_intensity / 255.0));
                    image[motor_led].g = (uint8_t)(153.0 * ((float)max_intensity / 255.0));
                    image[motor_led].r = (uint8_t)(225.0 * ((float)max_intensity / 255.0));
                }
            }
            for (float i = (motor_position - (chenillard_space * chenillard_position_ratio)); i > (STRIP_LENGTH / 6.0); i -= chenillard_space)
            {
                int motor_led = (uint16_t)(i / SPACE_BETWEEN_LEDS);
                if (motor_led > 0.0)
                {
                    image[motor_led].b = (uint8_t)(189.0 * ((float)max_intensity / 255.0));
                    image[motor_led].g = (uint8_t)(153.0 * ((float)max_intensity / 255.0));
                    image[motor_led].r = (uint8_t)(225.0 * ((float)max_intensity / 255.0));
                }
            }

            // Set motors positions in red
            uint16_t motor_led;
            if (motor_found > 0)
            {
                motor_led          = (uint16_t)((STRIP_LENGTH / 6.0) / SPACE_BETWEEN_LEDS);
                image[motor_led].b = 0;
                image[motor_led].g = (uint8_t)(81.0 * ((float)max_intensity / 255.0));
                image[motor_led].r = (uint8_t)(255.0 * ((float)max_intensity / 255.0));
            }
            if (motor_found > 1)
            {
                motor_led          = (uint16_t)((3.0 * STRIP_LENGTH / 6.0) / SPACE_BETWEEN_LEDS);
                image[motor_led].b = 0;
                image[motor_led].g = (uint8_t)(81.0 * ((float)max_intensity / 255.0));
                image[motor_led].r = (uint8_t)(255.0 * ((float)max_intensity / 255.0));
            }
            if (motor_found > 2)
            {
                motor_led          = (uint16_t)((5.0 * STRIP_LENGTH / 6.0) / SPACE_BETWEEN_LEDS);
                image[motor_led].b = 0;
                image[motor_led].g = (uint8_t)(81.0 * ((float)max_intensity / 255.0));
                image[motor_led].r = (uint8_t)(255.0 * ((float)max_intensity / 255.0));
            }

            // Overlap the red for the transmiting one
            motor_led = (uint16_t)(motor_position / SPACE_BETWEEN_LEDS);
            if (chenillard_position_ratio < 0.25)
            {
                image[motor_led].b = max_intensity;
                image[motor_led].g = max_intensity / 2;
                image[motor_led].r = max_intensity / 2;
            }
            else
            {
                image[motor_led].b = (uint8_t)(189.0 * ((float)max_intensity / 255.0));
                image[motor_led].g = (uint8_t)(153.0 * ((float)max_intensity / 255.0));
                image[motor_led].r = (uint8_t)(225.0 * ((float)max_intensity / 255.0));
            }
            // Make the chenillard move

            chenillard_position_ratio += chenillard_ratio_speed;
            if (chenillard_position_ratio >= 1.0)
            {
                chenillard_position_ratio -= 1.0;
            }
        }
    }
    // send the created image to the led_strip
    msg_t msg;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.target      = led_strip_id;
    msg.header.cmd         = COLOR;
    Luos_SendData(app, &msg, &image[0], sizeof(color_t) * LED_NUMBER);
    // reinitialize the image so that the led_strip is not lighted by default
    memset((void *)image, 0, LED_NUMBER * sizeof(color_t));
}

void sort_motors(void)
{
    search_result_t result;
    motor_found = 0;
    // Parse routing table to find motors
    RTFilter_Type(RTFilter_Reset(&result), SERVO_MOTOR_TYPE);
    motor_found = result.result_nbr;
}