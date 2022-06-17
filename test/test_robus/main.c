#include "main.h"
#include "robus.h"
#include "context.h"
#include <stdio.h>

void unittest_Robus_IDMaskCalculation()
{
    NEW_TEST_CASE("ID shift mast test");
    {
        Robus_MaskInit();

        Robus_IDMaskCalculation(7, MAX_SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(0, ctx.IDShiftMask);

        Robus_IDMaskCalculation(8, MAX_SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(0, ctx.IDShiftMask);

        Robus_IDMaskCalculation(9, MAX_SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(1, ctx.IDShiftMask);
    }

    NEW_TEST_CASE("ID mask mast test");
    {
        Robus_MaskInit();

        Robus_IDMaskCalculation(7, MAX_SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(0xC0, ctx.IDMask[0]);
        TEST_ASSERT_EQUAL(0x07, ctx.IDMask[1]);

        Robus_IDMaskCalculation(8, MAX_SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(1, ctx.IDShiftMask);
        TEST_ASSERT_EQUAL(0x80, ctx.IDMask[0]);
        TEST_ASSERT_EQUAL(0x0F, ctx.IDMask[1]);

        Robus_IDMaskCalculation(9, MAX_SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(1, ctx.IDShiftMask);
        TEST_ASSERT_EQUAL(0x00, ctx.IDMask[0]);
        TEST_ASSERT_EQUAL(0x1F, ctx.IDMask[1]);
    }

    NEW_TEST_CASE("Limit 4096");
    {
        Robus_MaskInit();
        Robus_IDMaskCalculation(4096 - MAX_SERVICE_NUMBER, MAX_SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(511, ctx.IDShiftMask);
    }

    NEW_TEST_CASE("Limit 0");
    {
        Robus_MaskInit();
        Robus_IDMaskCalculation(0, MAX_SERVICE_NUMBER);
        TEST_ASSERT_TRUE(IS_ASSERT());
        RESET_ASSERT();
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    ASSERT_ACTIVATION(1);

    // Big data reception
    UNIT_TEST_RUN(unittest_Robus_IDMaskCalculation);

    UNITY_END();
}
