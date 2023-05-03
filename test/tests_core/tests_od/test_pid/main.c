#include <stdio.h>
#include "unit_test.h"
#include "od_pid.h"

void unittest_Od_pid(void)
{
    NEW_TEST_CASE("pid msg conversion test");
    {
        asserv_pid_t pid;
        asserv_pid_t pid_ref = {1, 2, 3};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("pid msg conversion FROM test");
        msg_ref.header.cmd  = PID;
        msg_ref.header.size = sizeof(asserv_pid_t);
        memcpy(msg_ref.data, &pid_ref, sizeof(asserv_pid_t));
        PidOD_PidFromMsg(&pid, &msg_ref);
        TEST_ASSERT_EQUAL(pid_ref.p, pid.p);
        TEST_ASSERT_EQUAL(pid_ref.i, pid.i);
        TEST_ASSERT_EQUAL(pid_ref.d, pid.d);
        NEW_STEP("pid msg conversion TO test");
        PidOD_PidToMsg(&pid_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL(((asserv_pid_t *)msg_ref.data)->p, ((asserv_pid_t *)msg.data)->p);
        TEST_ASSERT_EQUAL(((asserv_pid_t *)msg_ref.data)->i, ((asserv_pid_t *)msg.data)->i);
        TEST_ASSERT_EQUAL(((asserv_pid_t *)msg_ref.data)->d, ((asserv_pid_t *)msg.data)->d);
    }
    NEW_TEST_CASE("pid msg conversion wrong values test");
    {
        RESET_ASSERT();
        asserv_pid_t pid;
        msg_t msg;
        TRY
        {
            NEW_STEP("pid msg conversion TO wrong msg_t* value test");
            PidOD_PidToMsg(&pid, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("pid msg conversion TO wrong asserv_pid_t* value test");
            PidOD_PidToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("pid msg conversion FROM wrong msg_t* value test");
            PidOD_PidFromMsg(&pid, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("pid msg conversion FROM wrong asserv_pid_t* value test");
            PidOD_PidFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_pid);

    UNITY_END();
}
