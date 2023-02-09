#include <stdio.h>
#include "unit_test.h"
#include "od_linear.h"

void unittest_Od_linearPosition(void)
{
    NEW_TEST_CASE("Linear position FROM test");
    {
        linear_position_t linear_pos;
        linear_position_t linear_pos_ref = {90};

        NEW_STEP("Linear position FROM m test");
        linear_pos = LinearOD_PositionFrom_m(90);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position FROM nm test");
        linear_pos = LinearOD_PositionFrom_nm(90000000000);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position FROM um test");
        linear_pos = LinearOD_PositionFrom_um(90000000);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position FROM mm test");
        linear_pos = LinearOD_PositionFrom_mm(90000);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position FROM cm test");
        linear_pos = LinearOD_PositionFrom_cm(9000);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position FROM km test");
        linear_pos = LinearOD_PositionFrom_km(0.09);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position FROM in test");
        linear_pos = LinearOD_PositionFrom_in(3543.31);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position FROM ft test");
        linear_pos = LinearOD_PositionFrom_ft(295.275833333);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position FROM mi test");
        linear_pos = LinearOD_PositionFrom_mi(0.05592345328276514954);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
    }
    NEW_TEST_CASE("Linear position TO test");
    {
        linear_position_t linear_pos = {90.5};

        NEW_STEP("Linear position TO m test");
        float m = LinearOD_PositionTo_m(linear_pos);
        TEST_ASSERT_EQUAL(90.5, m);
        NEW_STEP("Linear position TO nm test");
        float nm = LinearOD_PositionTo_nm(linear_pos);
        TEST_ASSERT_EQUAL(90.5, nm / 1000000000.0f);
        NEW_STEP("Linear position TO um test");
        float um = LinearOD_PositionTo_um(linear_pos);
        TEST_ASSERT_EQUAL(90.5, um / 1000000.0f);
        NEW_STEP("Linear position TO mm test");
        float mm = LinearOD_PositionTo_mm(linear_pos);
        TEST_ASSERT_EQUAL(90.5, mm / 1000.0f);
        NEW_STEP("Linear position TO cm test");
        float cm = LinearOD_PositionTo_cm(linear_pos);
        TEST_ASSERT_EQUAL(9050, cm);
        NEW_STEP("Linear position TO km test");
        float km = LinearOD_PositionTo_km(linear_pos);
        TEST_ASSERT_EQUAL(0.095, km);
        NEW_STEP("Linear position TO in test");
        float in = LinearOD_PositionTo_in(linear_pos);
        TEST_ASSERT_EQUAL(3562.991999999942891, in);
        NEW_STEP("Linear position TO ft test");
        float ft = LinearOD_PositionTo_ft(linear_pos);
        TEST_ASSERT_EQUAL(296.916, ft);
        NEW_STEP("Linear position TO mi test");
        float mi = LinearOD_PositionTo_mi(linear_pos);
        TEST_ASSERT_EQUAL(0.05623409090909, mi);
    }
    NEW_TEST_CASE("Linear position msg conversion test");
    {
        linear_position_t linear_pos;
        linear_position_t linear_pos_ref = {90};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Linear position msg conversion FROM test");
        msg_ref.header.cmd  = LINEAR_POSITION;
        msg_ref.header.size = sizeof(linear_position_t);
        memcpy(msg_ref.data, &linear_pos_ref, sizeof(linear_position_t));
        LinearOD_PositionFromMsg(&linear_pos, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)linear_pos_ref._private, (uint32_t)linear_pos._private);
        NEW_STEP("Linear position msg conversion TO test");
        LinearOD_PositionToMsg(&linear_pos_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((linear_position_t *)msg_ref.data)->_private, (uint32_t)((linear_position_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Linear position msg conversion wrong values test");
    {
        RESET_ASSERT();
        linear_position_t linear_pos;
        msg_t msg;
        TRY
        {
            NEW_STEP("Linear position msg conversion TO wrong msg_t* value test");
            LinearOD_PositionToMsg(&linear_pos, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Linear position msg conversion TO wrong linear_position_t* value test");
            LinearOD_PositionToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Linear position msg conversion FROM wrong msg_t* value test");
            LinearOD_PositionFromMsg(&linear_pos, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Linear position msg conversion FROM wrong linear_position_t* value test");
            LinearOD_PositionFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

void unittest_Od_linearSpeed(void)
{
    NEW_TEST_CASE("Linear speed FROM test");
    {
        linear_speed_t linear_speed;
        linear_speed_t linear_speed_ref = {90.5};

        NEW_STEP("Linear speed FROM m/s test");
        linear_speed = LinearOD_SpeedFrom_m_s(90.5);
        TEST_ASSERT_EQUAL((uint32_t)linear_speed_ref._private, (uint32_t)linear_speed._private);
        NEW_STEP("Linear speed FROM mm/s test");
        linear_speed = LinearOD_SpeedFrom_mm_s(90500);
        TEST_ASSERT_EQUAL((uint32_t)linear_speed_ref._private, (uint32_t)linear_speed._private);
        NEW_STEP("Linear speed FROM km/h test");
        linear_speed = LinearOD_SpeedFrom_km_h(324);
        TEST_ASSERT_EQUAL((uint32_t)linear_speed_ref._private, (uint32_t)linear_speed._private);
        NEW_STEP("Linear speed FROM in/s test");
        linear_speed = LinearOD_SpeedFrom_in_s(3543.307086614173);
        TEST_ASSERT_EQUAL((uint32_t)linear_speed_ref._private, (uint32_t)linear_speed._private);
        NEW_STEP("Linear speed FROM mph test");
        linear_speed = LinearOD_SpeedFrom_mi_h(202.4427);
        TEST_ASSERT_EQUAL((uint32_t)linear_speed_ref._private, (uint32_t)linear_speed._private);
    }
    NEW_TEST_CASE("Linear speed TO test");
    {
        linear_speed_t linear_speed = {90.5};

        NEW_STEP("Linear speed TO m/s test");
        float m_s = LinearOD_SpeedTo_m_s(linear_speed);
        TEST_ASSERT_EQUAL(90.5, m_s);
        NEW_STEP("Linear speed TO mm/s test");
        float mm_s = LinearOD_SpeedTo_mm_s(linear_speed);
        TEST_ASSERT_EQUAL(90500, mm_s);
        NEW_STEP("Linear speed TO km/h test");
        float km_h = LinearOD_SpeedTo_km_h(linear_speed);
        TEST_ASSERT_EQUAL(325.799944588774, km_h);
        NEW_STEP("Linear speed TO in/s test");
        float in_s = LinearOD_SpeedTo_in_s(linear_speed);
        TEST_ASSERT_EQUAL(3562.992, in_s);
        NEW_STEP("Linear speed TO mph test");
        float mph = LinearOD_SpeedTo_mi_h(linear_speed);
        TEST_ASSERT_EQUAL(202.4427272727, mph);
    }
    NEW_TEST_CASE("Linear speed msg conversion test");
    {
        linear_speed_t linear_speed;
        linear_speed_t linear_speed_ref = {90.5};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Linear speed msg conversion FROM test");
        msg_ref.header.cmd  = LINEAR_SPEED;
        msg_ref.header.size = sizeof(linear_speed_t);
        memcpy(msg_ref.data, &linear_speed_ref, sizeof(linear_speed_t));
        LinearOD_SpeedFromMsg(&linear_speed, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)linear_speed_ref._private, (uint32_t)linear_speed._private);
        NEW_STEP("Linear speed msg conversion TO test");
        LinearOD_SpeedToMsg(&linear_speed_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((linear_speed_t *)msg_ref.data)->_private, (uint32_t)((linear_speed_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Linear speed msg conversion wrong values test");
    {
        RESET_ASSERT();
        linear_speed_t linear_speed;
        msg_t msg;
        TRY
        {
            NEW_STEP("Linear speed msg conversion TO wrong msg_t* value test");
            LinearOD_SpeedToMsg(&linear_speed, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Linear speed msg conversion TO wrong linear_speed_t* value test");
            LinearOD_SpeedToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Linear speed msg conversion FROM wrong msg_t* value test");
            LinearOD_SpeedFromMsg(&linear_speed, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Linear speed msg conversion FROM wrong linear_speed_t* value test");
            LinearOD_SpeedFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_linearPosition);
    UNIT_TEST_RUN(unittest_Od_linearSpeed);

    UNITY_END();
}
