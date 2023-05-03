#include <stdio.h>
#include "unit_test.h"
#include "od_pressure.h"

void unittest_Od_pressure(void)
{
    NEW_TEST_CASE("Pressure FROM test");
    {
        pressure_t pressure;
        pressure_t pressure_ref = {1000.0f};

        NEW_STEP("Pressure FROM Pa test");
        pressure = PressureOD_PressureFrom_Pa(1000.0f);
        TEST_ASSERT_EQUAL((uint32_t)pressure_ref._private, (uint32_t)pressure._private);
        NEW_STEP("Pressure FROM bars test");
        pressure = PressureOD_PressureFrom_Bars(0.01f);
        TEST_ASSERT_EQUAL((uint32_t)pressure_ref._private, (uint32_t)pressure._private);
        NEW_STEP("Pressure FROM hPa test");
        pressure = PressureOD_PressureFrom_hPa(10.0f);
        TEST_ASSERT_EQUAL((uint32_t)pressure_ref._private, (uint32_t)pressure._private);
    }
    NEW_TEST_CASE("Pressure TO test");
    {
        pressure_t pressure = {1000.0f};

        NEW_STEP("Pressure TO Pa test");
        float Pa = PressureOD_PressureTo_Pa(pressure);
        TEST_ASSERT_EQUAL(1000.0f, Pa);
        NEW_STEP("Pressure TO bars test");
        float bars = PressureOD_PressureTo_Bars(pressure);
        TEST_ASSERT_EQUAL(0.01f, bars);
        NEW_STEP("Pressure TO hPa test");
        float hPa = PressureOD_PressureTo_hPa(pressure);
        TEST_ASSERT_EQUAL(10.0f, hPa);
    }
    NEW_TEST_CASE("Pressure msg conversion test");
    {
        pressure_t pressure;
        pressure_t pressure_ref = {1000.0f};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Pressure msg conversion FROM test");
        msg_ref.header.cmd  = PRESSURE;
        msg_ref.header.size = sizeof(pressure_t);
        memcpy(msg_ref.data, &pressure_ref, sizeof(pressure_t));
        PressureOD_PressureFromMsg(&pressure, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)pressure_ref._private, (uint32_t)pressure._private);
        NEW_STEP("Pressure msg conversion TO test");
        PressureOD_PressureToMsg(&pressure_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((pressure_t *)msg_ref.data)->_private, (uint32_t)((pressure_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Pressure msg conversion wrong values test");
    {
        RESET_ASSERT();
        pressure_t pressure;
        msg_t msg;
        TRY
        {
            NEW_STEP("Pressure msg conversion TO wrong msg_t* value test");
            PressureOD_PressureToMsg(&pressure, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Pressure msg conversion TO wrong pressure_t* value test");
            PressureOD_PressureToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Pressure msg conversion FROM wrong msg_t* value test");
            PressureOD_PressureFromMsg(&pressure, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Pressure msg conversion FROM wrong pressure_t* value test");
            PressureOD_PressureFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_pressure);

    UNITY_END();
}
