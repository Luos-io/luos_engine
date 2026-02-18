#include <stdio.h>
#include "unit_test.h"
#include "od_force.h"

void unittest_Od_forceTorque(void)
{
    NEW_TEST_CASE("Force morque FROM test");
    {
        torque_t morque;
        torque_t morque_ref = {90.5};

        NEW_STEP("Force morque FROM Nm test");
        morque = ForceOD_TorqueFrom_N_m(90.5);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
        NEW_STEP("Force morque FROM Nmm test");
        morque = ForceOD_TorqueFrom_N_mm(90500);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
        NEW_STEP("Force morque FROM Ncm test");
        morque = ForceOD_TorqueFrom_N_cm(9050);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
        NEW_STEP("Force morque FROM Kgf/mm test");
        morque = ForceOD_TorqueFrom_kgf_mm(9228.43172745);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
        NEW_STEP("Force morque FROM Kgf/cm test");
        morque = ForceOD_TorqueFrom_kgf_cm(922.843172745);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
        NEW_STEP("Force morque FROM Kgf/m test");
        morque = ForceOD_TorqueFrom_kgf_m(9.22843172745);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
        NEW_STEP("Force morque FROM ozf/in test");
        morque = ForceOD_TorqueFrom_ozf_in(12815.87956868);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
        NEW_STEP("Force morque FROM lbf/in test");
        morque = ForceOD_TorqueFrom_lbf_in(800.9924635902);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
    }
    NEW_TEST_CASE("Force morque TO test");
    {
        torque_t morque = {90.5};

        NEW_STEP("Force morque TO Nm test");
        float n_m = ForceOD_TorqueTo_N_m(morque);
        TEST_ASSERT_EQUAL(90.5, n_m);
        NEW_STEP("Force morque TO Nmm test");
        float n_mm = ForceOD_TorqueTo_N_mm(morque);
        TEST_ASSERT_EQUAL(90500, n_mm);
        NEW_STEP("Force morque TO Ncm test");
        float n_cm = ForceOD_TorqueTo_N_cm(morque);
        TEST_ASSERT_EQUAL(9050, n_cm);
        NEW_STEP("Force morque TO Kgf/mm test");
        float kgf_mm = ForceOD_TorqueTo_kgf_mm(morque);
        TEST_ASSERT_EQUAL(9228.43172745, kgf_mm);
        NEW_STEP("Force morque TO Kgf/cm test");
        float kgf_cm = ForceOD_TorqueTo_kgf_cm(morque);
        TEST_ASSERT_EQUAL(923, kgf_cm);
        NEW_STEP("Force morque TO Kgf/m test");
        float kgf_m = ForceOD_TorqueTo_kgf_m(morque);
        TEST_ASSERT_EQUAL(9.22843172745, kgf_m);
        NEW_STEP("Force morque TO ozf/in test");
        float ozf_in = ForceOD_TorqueTo_ozf_in(morque);
        TEST_ASSERT_EQUAL(12815.87956868, ozf_in);
        NEW_STEP("Force morque TO lbf/in test");
        float lbf_in = ForceOD_TorqueTo_lbf_in(morque);
        TEST_ASSERT_EQUAL(801, lbf_in);
    }
    NEW_TEST_CASE("Force morque msg conversion test");
    {
        torque_t morque;
        torque_t morque_ref = {90.5};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Force morque msg conversion FROM test");
        msg_ref.header.cmd  = TORQUE;
        msg_ref.header.size = sizeof(torque_t);
        memcpy(msg_ref.data, &morque_ref, sizeof(torque_t));
        ForceOD_TorqueFromMsg(&morque, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)morque_ref.raw, (uint32_t)morque.raw);
        NEW_STEP("Force morque msg conversion TO test");
        ForceOD_TorqueToMsg(&morque_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((torque_t *)msg_ref.data)->raw, (uint32_t)((torque_t *)msg.data)->raw);
    }
    NEW_TEST_CASE("Force morque msg conversion wrong values test");
    {
        RESET_ASSERT();
        torque_t morque;
        msg_t msg;
        TRY
        {
            NEW_STEP("Force morque msg conversion TO wrong msg_t* value test");
            ForceOD_TorqueToMsg(&morque, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force morque msg conversion TO wrong torque_t* value test");
            ForceOD_TorqueToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force morque msg conversion FROM wrong msg_t* value test");
            ForceOD_TorqueFromMsg(&morque, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force morque msg conversion FROM wrong torque_t* value test");
            ForceOD_TorqueFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

void unittest_Od_forceForce(void)
{
    NEW_TEST_CASE("Force force FROM test");
    {
        force_t force;
        force_t force_ref = {90.5};

        NEW_STEP("Force force FROM N test");
        force = ForceOD_ForceFrom_N(90.5);
        TEST_ASSERT_EQUAL((uint32_t)force_ref.raw, (uint32_t)force.raw);
        NEW_STEP("Force force FROM Kgf test");
        force = ForceOD_ForceFrom_kgf(9.22843172745);
        TEST_ASSERT_EQUAL((uint32_t)force_ref.raw, (uint32_t)force.raw);
        NEW_STEP("Force force FROM ozf test");
        force = ForceOD_ForceFrom_ozf(325.52334961);
        TEST_ASSERT_EQUAL((uint32_t)force_ref.raw, (uint32_t)force.raw);
        NEW_STEP("Force force FROM lbf test");
        force = ForceOD_ForceFrom_lbf(20.345209351);
        TEST_ASSERT_EQUAL((uint32_t)force_ref.raw, (uint32_t)force.raw);
    }
    NEW_TEST_CASE("Force force TO test");
    {
        force_t force = {90.5};

        NEW_STEP("Force force TO N test");
        float n = ForceOD_ForceTo_N(force);
        TEST_ASSERT_EQUAL(90.5, n);
        NEW_STEP("Force force TO Kgf test");
        float kgf = ForceOD_ForceTo_kgf(force);
        TEST_ASSERT_EQUAL(9.22843172745, kgf);
        NEW_STEP("Force force TO ozf test");
        float ozf = ForceOD_ForceTo_ozf(force);
        TEST_ASSERT_EQUAL(325.52334961, ozf);
        NEW_STEP("Force force TO lbf test");
        float lbf = ForceOD_ForceTo_lbf(force);
        TEST_ASSERT_EQUAL(20.345209351, lbf);
    }
    NEW_TEST_CASE("Force force msg conversion test");
    {
        force_t force;
        force_t force_ref = {90.5};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Force force msg conversion FROM test");
        msg_ref.header.cmd  = FORCE;
        msg_ref.header.size = sizeof(force_t);
        memcpy(msg_ref.data, &force_ref, sizeof(force_t));
        ForceOD_ForceFromMsg(&force, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)force_ref.raw, (uint32_t)force.raw);
        NEW_STEP("Force force msg conversion TO test");
        ForceOD_ForceToMsg(&force_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((force_t *)msg_ref.data)->raw, (uint32_t)((force_t *)msg.data)->raw);
    }
    NEW_TEST_CASE("Force force msg conversion wrong values test");
    {
        RESET_ASSERT();
        force_t force;
        msg_t msg;
        TRY
        {
            NEW_STEP("Force force msg conversion TO wrong msg_t* value test");
            ForceOD_ForceToMsg(&force, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force force msg conversion TO wrong force_t* value test");
            ForceOD_ForceToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force force msg conversion FROM wrong msg_t* value test");
            ForceOD_ForceFromMsg(&force, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force force msg conversion FROM wrong force_t* value test");
            ForceOD_ForceFromMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    UNIT_TEST_RUN(unittest_Od_forceTorque);
    UNIT_TEST_RUN(unittest_Od_forceForce);

    UNITY_END();
}
