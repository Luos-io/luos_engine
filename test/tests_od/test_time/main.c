#include <stdio.h>
#include "unit_test.h"
#include "od_time.h"

void unittest_Od_time(void)
{
    NEW_TEST_CASE("Time FROM test");
    {
        time_luos_t time;
        time_luos_t time_ref = {1000.5f};

        NEW_STEP("Time FROM seconds test");
        time = TimeOD_TimeFrom_s(1000.5f);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
        NEW_STEP("Time FROM ms test");
        time = TimeOD_TimeFrom_ms(1000500.0f);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
        NEW_STEP("Time FROM us test");
        time = TimeOD_TimeFrom_us(1000500000.0f);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
        NEW_STEP("Time FROM ns test");
        time = TimeOD_TimeFrom_ns(1000500000000.0f);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
        NEW_STEP("Time FROM min test");
        time = TimeOD_TimeFrom_min(16.675f);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
        NEW_STEP("Time FROM hour test");
        time = TimeOD_TimeFrom_h(0.2779166666666667f);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
        NEW_STEP("Time FROM day test");
        time = TimeOD_TimeFrom_day(0.0115798611f);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
    }
    NEW_TEST_CASE("Time TO test");
    {
        time_luos_t time = {1000.0f};

        NEW_STEP("Time TO seconds test");
        float s = TimeOD_TimeTo_s(time);
        TEST_ASSERT_EQUAL(1000.0f, s);
        NEW_STEP("Time TO ms test");
        float ms = TimeOD_TimeTo_ms(time);
        TEST_ASSERT_EQUAL(1000000.0f, ms);
        NEW_STEP("Time TO us test");
        float us = TimeOD_TimeTo_us(time);
        TEST_ASSERT_EQUAL(1000000000.0f, us);
        NEW_STEP("Time TO ns test");
        float ns = TimeOD_TimeTo_ns(time);
        TEST_ASSERT_EQUAL(1000000000000.0f, ns);
        NEW_STEP("Time TO min test");
        float min = TimeOD_TimeTo_min(time);
        TEST_ASSERT_EQUAL(16.666666666666668f, min);
        NEW_STEP("Time TO hour test");
        float hour = TimeOD_TimeTo_h(time);
        TEST_ASSERT_EQUAL(0.2777777777777778f, hour);
        NEW_STEP("Time TO day test");
        float day = TimeOD_TimeTo_day(time);
        TEST_ASSERT_EQUAL(0.011574074074074074f, day);
    }
    NEW_TEST_CASE("Time msg conversion test");
    {
        time_luos_t time;
        time_luos_t time_ref = {1000.0f};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Time msg conversion FROM test");
        msg_ref.header.cmd  = TIME;
        msg_ref.header.size = sizeof(time_luos_t);
        memcpy(msg_ref.data, &time_ref, sizeof(time_luos_t));
        TimeOD_TimeFromMsg(&time, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
        NEW_STEP("Time msg conversion TO test");
        TimeOD_TimeToMsg(&time, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)time_ref._private, (uint32_t)time._private);
    }
    NEW_TEST_CASE("Time msg conversion wrong values test");
    {
        RESET_ASSERT();
        time_luos_t time;
        msg_t msg;
        TRY
        {
            NEW_STEP("Time msg conversion TO wrong msg_t* value test");
            TimeOD_TimeToMsg(&time, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Time msg conversion TO wrong time_luos_t* value test");
            TimeOD_TimeToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Time msg conversion FROM wrong msg_t* value test");
            TimeOD_TimeFromMsg(&time, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Time msg conversion FROM wrong time_luos_t* value test");
            TimeOD_TimeFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_time);

    UNITY_END();
}
