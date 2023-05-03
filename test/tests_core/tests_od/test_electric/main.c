#include <stdio.h>
#include "unit_test.h"
#include "od_electric.h"

void unittest_Od_electricVoltage(void)
{
    NEW_TEST_CASE("Electric voltage FROM test");
    {
        voltage_t voltage;
        voltage_t voltage_ref = {1.0};

        NEW_STEP("Electric voltage FROM V test");
        voltage = ElectricOD_VoltageFrom_V(1.0);
        TEST_ASSERT_EQUAL((uint32_t)voltage_ref._private, (uint32_t)voltage._private);
        NEW_STEP("Electric voltage FROM mV test");
        voltage = ElectricOD_VoltageFrom_mV(1000);
        TEST_ASSERT_EQUAL((uint32_t)voltage_ref._private, (uint32_t)voltage._private);
    }
    NEW_TEST_CASE("Electric voltage TO test");
    {
        voltage_t voltage = {1.0};

        NEW_STEP("Electric voltage TO V test");
        float V = ElectricOD_VoltageTo_V(voltage);
        TEST_ASSERT_EQUAL(1.0, V);
        NEW_STEP("Electric voltage TO mV test");
        float mV = ElectricOD_VoltageTo_mV(voltage);
        TEST_ASSERT_EQUAL(1000, mV);
    }
    NEW_TEST_CASE("Electric voltage msg conversion test");
    {
        voltage_t voltage;
        voltage_t voltage_ref = {1.0};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Electric voltage msg conversion FROM test");
        msg_ref.header.cmd  = VOLTAGE;
        msg_ref.header.size = sizeof(voltage_t);
        memcpy(msg_ref.data, &voltage_ref, sizeof(voltage_t));
        ElectricOD_VoltageFromMsg(&voltage, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)voltage_ref._private, (uint32_t)voltage._private);
        NEW_STEP("Electric voltage msg conversion TO test");
        ElectricOD_VoltageToMsg(&voltage_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((voltage_t *)msg_ref.data)->_private, (uint32_t)((voltage_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Electric voltage msg conversion wrong values test");
    {
        RESET_ASSERT();
        voltage_t voltage;
        msg_t msg;
        TRY
        {
            NEW_STEP("Electric voltage msg conversion TO wrong msg_t* value test");
            ElectricOD_VoltageToMsg(&voltage, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric voltage msg conversion TO wrong voltage_t* value test");
            ElectricOD_VoltageToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric voltage msg conversion FROM wrong msg_t* value test");
            ElectricOD_VoltageFromMsg(&voltage, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric voltage msg conversion FROM wrong voltage_t* value test");
            ElectricOD_VoltageFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

void unittest_Od_electricCurrent(void)
{
    NEW_TEST_CASE("Electric current FROM test");
    {
        current_t current;
        current_t current_ref = {1.0};

        NEW_STEP("Electric current FROM A test");
        current = ElectricOD_CurrentFrom_A(1.0);
        TEST_ASSERT_EQUAL((uint32_t)current_ref._private, (uint32_t)current._private);
        NEW_STEP("Electric current FROM mA test");
        current = ElectricOD_CurrentFrom_mA(1000);
        TEST_ASSERT_EQUAL((uint32_t)current_ref._private, (uint32_t)current._private);
    }
    NEW_TEST_CASE("Electric current TO test");
    {
        current_t current = {1.0};

        NEW_STEP("Electric current TO A test");
        float A = ElectricOD_CurrentTo_A(current);
        TEST_ASSERT_EQUAL(1.0, A);
        NEW_STEP("Electric current TO mA test");
        float mA = ElectricOD_CurrentTo_mA(current);
        TEST_ASSERT_EQUAL(1000, mA);
    }
    NEW_TEST_CASE("Electric current msg conversion test");
    {
        current_t current;
        current_t current_ref = {1.0};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Electric current msg conversion FROM test");
        msg_ref.header.cmd  = CURRENT;
        msg_ref.header.size = sizeof(current_t);
        memcpy(msg_ref.data, &current_ref, sizeof(current_t));
        ElectricOD_CurrentFromMsg(&current, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)current_ref._private, (uint32_t)current._private);
        NEW_STEP("Electric current msg conversion TO test");
        ElectricOD_CurrentToMsg(&current_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((current_t *)msg_ref.data)->_private, (uint32_t)((current_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Electric current msg conversion wrong values test");
    {
        RESET_ASSERT();
        current_t current;
        msg_t msg;
        TRY
        {
            NEW_STEP("Electric current msg conversion TO wrong msg_t* value test");
            ElectricOD_CurrentToMsg(&current, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric current msg conversion TO wrong current_t* value test");
            ElectricOD_CurrentToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric current msg conversion FROM wrong msg_t* value test");
            ElectricOD_CurrentFromMsg(&current, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric current msg conversion FROM wrong current_t* value test");
            ElectricOD_CurrentFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

void unittest_Od_electricPower(void)
{
    NEW_TEST_CASE("Electric power FROM test");
    {
        power_t power;
        power_t power_ref = {1.0};

        NEW_STEP("Electric power FROM W test");
        power = ElectricOD_PowerFrom_W(1.0);
        TEST_ASSERT_EQUAL((uint32_t)power_ref._private, (uint32_t)power._private);
        NEW_STEP("Electric power FROM mW test");
        power = ElectricOD_PowerFrom_mW(1000);
        TEST_ASSERT_EQUAL((uint32_t)power_ref._private, (uint32_t)power._private);
    }
    NEW_TEST_CASE("Electric power TO test");
    {
        power_t power = {1.0};

        NEW_STEP("Electric power TO W test");
        float W = ElectricOD_PowerTo_W(power);
        TEST_ASSERT_EQUAL(1.0, W);
        NEW_STEP("Electric power TO mW test");
        float mW = ElectricOD_PowerTo_mW(power);
        TEST_ASSERT_EQUAL(1000, mW);
    }
    NEW_TEST_CASE("Electric power msg conversion test");
    {
        power_t power;
        power_t power_ref = {1.0};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Electric power msg conversion FROM test");
        msg_ref.header.cmd  = POWER;
        msg_ref.header.size = sizeof(power_t);
        memcpy(msg_ref.data, &power_ref, sizeof(power_t));
        ElectricOD_PowerFromMsg(&power, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)power_ref._private, (uint32_t)power._private);
        NEW_STEP("Electric power msg conversion TO test");
        ElectricOD_PowerToMsg(&power_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((power_t *)msg_ref.data)->_private, (uint32_t)((power_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Electric power msg conversion wrong values test");
    {
        RESET_ASSERT();
        power_t power;
        msg_t msg;
        TRY
        {
            NEW_STEP("Electric power msg conversion TO wrong msg_t* value test");
            ElectricOD_PowerToMsg(&power, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric power msg conversion TO wrong power_t* value test");
            ElectricOD_PowerToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric power msg conversion FROM wrong msg_t* value test");
            ElectricOD_PowerFromMsg(&power, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Electric power msg conversion FROM wrong power_t* value test");
            ElectricOD_PowerFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_electricVoltage);
    UNIT_TEST_RUN(unittest_Od_electricCurrent);
    UNIT_TEST_RUN(unittest_Od_electricPower);

    UNITY_END();
}
