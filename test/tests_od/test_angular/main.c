#include <stdio.h>
#include "unit_test.h"
#include "od_angular.h"

void unittest_Od_angularPosition(void)
{
    NEW_TEST_CASE("Angular position FROM test");
    {
        angular_position_t angular_pos;
        angular_position_t angular_pos_ref = {90};

        NEW_STEP("Angular position FROM deg test");
        angular_pos = AngularOD_PositionFrom_deg(90);
        TEST_ASSERT_EQUAL((uint32_t)angular_pos_ref._private, (uint32_t)angular_pos._private);
        NEW_STEP("Angular position FROM rev test");
        angular_pos = AngularOD_PositionFrom_rev(0.25);
        TEST_ASSERT_EQUAL((uint32_t)angular_pos_ref._private, (uint32_t)angular_pos._private);
        NEW_STEP("Angular position FROM rad test");
        angular_pos = AngularOD_PositionFrom_rad(1.5707963267948966);
        TEST_ASSERT_EQUAL((uint32_t)angular_pos_ref._private, (uint32_t)angular_pos._private);
    }
    NEW_TEST_CASE("Angular position TO test");
    {
        angular_position_t angular_pos = {90};

        NEW_STEP("Angular position TO deg test");
        float deg = AngularOD_PositionTo_deg(angular_pos);
        TEST_ASSERT_EQUAL(90, deg);
        NEW_STEP("Angular position TO rev test");
        float rev = AngularOD_PositionTo_rev(angular_pos);
        TEST_ASSERT_EQUAL(0.25, rev);
        NEW_STEP("Angular position TO rad test");
        float rad = AngularOD_PositionTo_rad(angular_pos);
        TEST_ASSERT_EQUAL(1.5707963267948966, rad);
    }
    NEW_TEST_CASE("Angular position msg conversion test");
    {
        angular_position_t angular_pos;
        angular_position_t angular_pos_ref = {90};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Angular position msg conversion FROM test");
        msg_ref.header.cmd  = ANGULAR_POSITION;
        msg_ref.header.size = sizeof(angular_position_t);
        memcpy(msg_ref.data, &angular_pos_ref, sizeof(angular_position_t));
        AngularOD_PositionFromMsg(&angular_pos, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)angular_pos_ref._private, (uint32_t)angular_pos._private);
        NEW_STEP("Angular position msg conversion TO test");
        AngularOD_PositionToMsg(&angular_pos_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((angular_position_t *)msg_ref.data)->_private, (uint32_t)((angular_position_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Angular position msg conversion wrong values test");
    {
        RESET_ASSERT();
        angular_position_t angular_pos;
        msg_t msg;
        TRY
        {
            NEW_STEP("Angular position msg conversion TO wrong msg_t* value test");
            AngularOD_PositionToMsg(&angular_pos, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Angular position msg conversion TO wrong angular_position_t* value test");
            AngularOD_PositionToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Angular position msg conversion FROM wrong msg_t* value test");
            AngularOD_PositionFromMsg(&angular_pos, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Angular position msg conversion FROM wrong angular_position_t* value test");
            AngularOD_PositionFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

void unittest_Od_angularSpeed(void)
{
    NEW_TEST_CASE("Angular speed FROM test");
    {
        angular_speed_t angular_speed;
        angular_speed_t angular_speed_ref = {90};

        NEW_STEP("Angular speed FROM deg/s test");
        angular_speed = AngularOD_SpeedFrom_deg_s(90);
        TEST_ASSERT_EQUAL((uint32_t)angular_speed_ref._private, (uint32_t)angular_speed._private);
        NEW_STEP("Angular speed FROM rev/s test");
        angular_speed = AngularOD_SpeedFrom_rev_s(0.25);
        TEST_ASSERT_EQUAL((uint32_t)angular_speed_ref._private, (uint32_t)angular_speed._private);
        NEW_STEP("Angular speed FROM rev/min test");
        angular_speed = AngularOD_SpeedFrom_rev_min(0.25 * 60);
        TEST_ASSERT_EQUAL((uint32_t)angular_speed_ref._private, (uint32_t)angular_speed._private);
        NEW_STEP("Angular speed FROM rad/s test");
        angular_speed = AngularOD_SpeedFrom_rad_s(1.5707963267948966);
        TEST_ASSERT_EQUAL((uint32_t)angular_speed_ref._private, (uint32_t)angular_speed._private);
    }
    NEW_TEST_CASE("Angular speed TO test");
    {
        angular_speed_t angular_speed = {90};

        NEW_STEP("Angular speed TO deg/s test");
        float deg_s = AngularOD_SpeedTo_deg_s(angular_speed);
        TEST_ASSERT_EQUAL(90, deg_s);
        NEW_STEP("Angular speed TO rev/s test");
        float rev_s = AngularOD_SpeedTo_rev_s(angular_speed);
        TEST_ASSERT_EQUAL(0.25, rev_s);
        NEW_STEP("Angular speed TO rev/min test");
        float rev_min = AngularOD_SpeedTo_rev_min(angular_speed);
        TEST_ASSERT_EQUAL(0.25 * 60, rev_min);
        NEW_STEP("Angular speed TO rad/s test");
        float rad_s = AngularOD_SpeedTo_rad_s(angular_speed);
        TEST_ASSERT_EQUAL(1.5707963267948966, rad_s);
    }
    NEW_TEST_CASE("Angular speed msg conversion test");
    {
        angular_speed_t angular_speed;
        angular_speed_t angular_speed_ref = {90};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Angular speed msg conversion FROM test");
        msg_ref.header.cmd  = ANGULAR_SPEED;
        msg_ref.header.size = sizeof(angular_speed_t);
        memcpy(msg_ref.data, &angular_speed_ref, sizeof(angular_speed_t));
        AngularOD_SpeedFromMsg(&angular_speed, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)angular_speed_ref._private, (uint32_t)angular_speed._private);
        NEW_STEP("Angular speed msg conversion TO test");
        AngularOD_SpeedToMsg(&angular_speed_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((angular_speed_t *)msg_ref.data)->_private, (uint32_t)((angular_speed_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Angular speed msg conversion wrong values test");
    {
        RESET_ASSERT();
        angular_speed_t angular_speed;
        msg_t msg;
        TRY
        {
            NEW_STEP("Angular speed msg conversion TO wrong msg_t* value test");
            AngularOD_SpeedToMsg(&angular_speed, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Angular speed msg conversion TO wrong angular_speed_t* value test");
            AngularOD_SpeedToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Angular speed msg conversion FROM wrong msg_t* value test");
            AngularOD_SpeedFromMsg(&angular_speed, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Angular speed msg conversion FROM wrong angular_speed_t* value test");
            AngularOD_SpeedFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_angularPosition);
    UNIT_TEST_RUN(unittest_Od_angularSpeed);

    UNITY_END();
}
