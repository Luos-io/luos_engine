#include <stdio.h>
#include "unit_test.h"
#include "od_control.h"

void unittest_Od_control(void)
{
    NEW_TEST_CASE("Control msg conversion test");
    {
        control_t control;
        control_t control_ref;
        msg_t msg_ref;
        msg_t msg;

        control_ref.flux = PAUSE;
        control_ref.rec  = false;
        NEW_STEP("Control msg conversion FROM test");
        msg_ref.header.cmd  = CONTROL;
        msg_ref.header.size = sizeof(control_t);
        memcpy(msg_ref.data, &control_ref, sizeof(control_t));
        ControlOD_ControlFromMsg(&control, &msg_ref);
        TEST_ASSERT_EQUAL(control_ref.unmap, control.unmap);
        NEW_STEP("Control msg conversion TO test");
        ControlOD_ControlToMsg(&control_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL(((control_t *)msg_ref.data)->unmap, ((control_t *)msg.data)->unmap);
    }
    NEW_TEST_CASE("Control msg conversion wrong values test");
    {
        RESET_ASSERT();
        control_t control;
        msg_t msg;
        TRY
        {
            NEW_STEP("Control msg conversion TO wrong msg_t* value test");
            ControlOD_ControlToMsg(&control, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Control msg conversion TO wrong control_t* value test");
            ControlOD_ControlToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Control msg conversion FROM wrong msg_t* value test");
            ControlOD_ControlFromMsg(&control, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Control msg conversion FROM wrong control_t* value test");
            ControlOD_ControlFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_control);

    UNITY_END();
}
