#include <stdio.h>
#include "unit_test.h"
#include "od_illuminance.h"

void unittest_Od_illuminance_illuminance(void)
{
    NEW_TEST_CASE("Illuminance FROM test");
    {
        illuminance_t illuminance;
        illuminance_t illuminance_ref = {100};

        NEW_STEP("Illuminance FROM lux test");
        illuminance = IlluminanceOD_IlluminanceFrom_Lux(100);
        TEST_ASSERT_EQUAL((uint32_t)illuminance_ref._private, (uint32_t)illuminance._private);
    }
    NEW_TEST_CASE("Illuminance TO test");
    {
        illuminance_t illuminance = {100};

        NEW_STEP("Illuminance TO lux test");
        float lux = IlluminanceOD_IlluminanceTo_Lux(illuminance);
        TEST_ASSERT_EQUAL(100, lux);
    }
    NEW_TEST_CASE("Illuminance msg conversion test");
    {
        illuminance_t illuminance;
        illuminance_t illuminance_ref = {100};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Illuminance msg conversion FROM test");
        msg_ref.header.cmd  = ILLUMINANCE;
        msg_ref.header.size = sizeof(illuminance_t);
        memcpy(msg_ref.data, &illuminance_ref, sizeof(illuminance_t));
        IlluminanceOD_IlluminanceFromMsg(&illuminance, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)illuminance_ref._private, (uint32_t)illuminance._private);
        NEW_STEP("Illuminance msg conversion TO test");
        IlluminanceOD_IlluminanceToMsg(&illuminance_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((illuminance_t *)msg_ref.data)->_private, (uint32_t)((illuminance_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Illuminance msg conversion wrong values test");
    {
        RESET_ASSERT();
        illuminance_t illuminance;
        msg_t msg;
        TRY
        {
            NEW_STEP("Illuminance msg conversion TO wrong msg_t* value test");
            IlluminanceOD_IlluminanceToMsg(&illuminance, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Illuminance msg conversion TO wrong illuminance_t* value test");
            IlluminanceOD_IlluminanceToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Illuminance msg conversion FROM wrong msg_t* value test");
            IlluminanceOD_IlluminanceFromMsg(&illuminance, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Illuminance msg conversion FROM wrong illuminance_t* value test");
            IlluminanceOD_IlluminanceFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

void unittest_Od_illuminance_color(void)
{
    NEW_TEST_CASE("Color msg conversion test");
    {
        color_t color;
        color_t color_ref = {100, 200, 150};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Color msg conversion FROM test");
        msg_ref.header.cmd  = COLOR;
        msg_ref.header.size = sizeof(color_t);
        memcpy(msg_ref.data, &color_ref, sizeof(color_t));
        IlluminanceOD_ColorFromMsg(&color, &msg_ref);
        TEST_ASSERT_EQUAL(color_ref.r, color.r);
        TEST_ASSERT_EQUAL(color_ref.g, color.g);
        TEST_ASSERT_EQUAL(color_ref.b, color.b);
        NEW_STEP("Color msg conversion TO test");
        IlluminanceOD_ColorToMsg(&color_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL(((color_t *)msg_ref.data)->r, ((color_t *)msg.data)->r);
        TEST_ASSERT_EQUAL(((color_t *)msg_ref.data)->g, ((color_t *)msg.data)->g);
        TEST_ASSERT_EQUAL(((color_t *)msg_ref.data)->b, ((color_t *)msg.data)->b);
    }
    NEW_TEST_CASE("Color msg conversion wrong values test");
    {
        RESET_ASSERT();
        color_t color;
        msg_t msg;
        TRY
        {
            NEW_STEP("Color msg conversion TO wrong msg_t* value test");
            IlluminanceOD_ColorToMsg(&color, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Color msg conversion TO wrong color_t* value test");
            IlluminanceOD_ColorToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Color msg conversion FROM wrong msg_t* value test");
            IlluminanceOD_ColorFromMsg(&color, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Color msg conversion FROM wrong color_t* value test");
            IlluminanceOD_ColorFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_illuminance_illuminance);
    UNIT_TEST_RUN(unittest_Od_illuminance_color);

    UNITY_END();
}
