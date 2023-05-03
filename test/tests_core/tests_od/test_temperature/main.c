#include <stdio.h>
#include "unit_test.h"
#include "od_temperature.h"

void unittest_Od_temperature(void)
{
    NEW_TEST_CASE("Temperature FROM test");
    {
        temperature_t temperature;
        temperature_t temperature_ref = {1000.0f};

        NEW_STEP("Temperature FROM degree C test");
        temperature = TemperatureOD_TemperatureFrom_deg_c(1000.0f);
        TEST_ASSERT_EQUAL((uint32_t)temperature_ref._private, (uint32_t)temperature._private);
        NEW_STEP("Temperature FROM degree F test");
        temperature = TemperatureOD_TemperatureFrom_deg_f(1832.0f);
        TEST_ASSERT_EQUAL((uint32_t)temperature_ref._private, (uint32_t)temperature._private);
        NEW_STEP("Temperature FROM degree K test");
        temperature = TemperatureOD_TemperatureFrom_deg_k(1273.15f);
        TEST_ASSERT_EQUAL((uint32_t)temperature_ref._private, (uint32_t)temperature._private);
    }
    NEW_TEST_CASE("Temperature TO test");
    {
        temperature_t temperature = {1000.0f};

        NEW_STEP("Temperature TO degree C test");
        float deg_c = TemperatureOD_TemperatureTo_deg_c(temperature);
        TEST_ASSERT_EQUAL(1000.0f, deg_c);
        NEW_STEP("Temperature TO degree F test");
        float deg_f = TemperatureOD_TemperatureTo_deg_f(temperature);
        TEST_ASSERT_EQUAL(1832.0f, deg_f);
        NEW_STEP("Temperature TO degree K test");
        float deg_k = TemperatureOD_TemperatureTo_deg_k(temperature);
        TEST_ASSERT_EQUAL(1273.15f, deg_k);
    }
    NEW_TEST_CASE("Temperature msg conversion test");
    {
        temperature_t temperature;
        temperature_t temperature_ref = {1000.0f};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Temperature msg conversion FROM test");
        msg_ref.header.cmd  = TEMPERATURE;
        msg_ref.header.size = sizeof(temperature_t);
        memcpy(msg_ref.data, &temperature_ref, sizeof(temperature_t));
        TemperatureOD_TemperatureFromMsg(&temperature, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)temperature_ref._private, (uint32_t)temperature._private);
        NEW_STEP("Temperature msg conversion TO test");
        TemperatureOD_TemperatureToMsg(&temperature_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((temperature_t *)msg_ref.data)->_private, (uint32_t)((temperature_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Temperature msg conversion wrong values test");
    {
        RESET_ASSERT();
        temperature_t temperature;
        msg_t msg;
        TRY
        {
            NEW_STEP("Temperature msg conversion TO wrong msg_t* value test");
            TemperatureOD_TemperatureToMsg(&temperature, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Temperature msg conversion TO wrong temperature_t* value test");
            TemperatureOD_TemperatureToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Temperature msg conversion FROM wrong msg_t* value test");
            TemperatureOD_TemperatureFromMsg(&temperature, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Temperature msg conversion FROM wrong temperature_t* value test");
            TemperatureOD_TemperatureFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_temperature);

    UNITY_END();
}
