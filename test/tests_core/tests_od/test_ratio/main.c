#include <stdio.h>
#include "unit_test.h"
#include "od_ratio.h"

void unittest_Od_ratio(void)
{
    NEW_TEST_CASE("Ratio FROM test");
    {
        ratio_t ratio;
        ratio_t ratio_ref = {95.0f};

        NEW_STEP("Ratio FROM Percent test");
        ratio = RatioOD_RatioFrom_Percent(95.0f);
        TEST_ASSERT_EQUAL((uint32_t)ratio_ref._private, (uint32_t)ratio._private);
    }
    NEW_TEST_CASE("Ratio TO test");
    {
        ratio_t ratio = {95.0f};

        NEW_STEP("Ratio TO Percent test");
        float Percent = RatioOD_RatioTo_Percent(ratio);
        TEST_ASSERT_EQUAL(95.0f, Percent);
    }
    NEW_TEST_CASE("Ratio msg conversion test");
    {
        ratio_t ratio;
        ratio_t ratio_ref = {95.0f};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Ratio msg conversion FROM test");
        msg_ref.header.cmd  = RATIO;
        msg_ref.header.size = sizeof(ratio_t);
        memcpy(msg_ref.data, &ratio_ref, sizeof(ratio_t));
        RatioOD_RatioFromMsg(&ratio, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)ratio_ref._private, (uint32_t)ratio._private);
        NEW_STEP("Ratio msg conversion TO test");
        RatioOD_RatioToMsg(&ratio_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((ratio_t *)msg_ref.data)->_private, (uint32_t)((ratio_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Ratio msg conversion wrong values test");
    {
        RESET_ASSERT();
        ratio_t ratio;
        msg_t msg;
        TRY
        {
            NEW_STEP("Ratio msg conversion TO wrong msg_t* value test");
            RatioOD_RatioToMsg(&ratio, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Ratio msg conversion TO wrong ratio_t* value test");
            RatioOD_RatioToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Ratio msg conversion FROM wrong msg_t* value test");
            RatioOD_RatioFromMsg(&ratio, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Ratio msg conversion FROM wrong ratio_t* value test");
            RatioOD_RatioFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_ratio);

    UNITY_END();
}
