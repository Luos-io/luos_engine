#include <stdio.h>
#include "unit_test.h"
#include "od_force.h"

void unittest_Od_forceMoment(void)
{
    NEW_TEST_CASE("Force moment FROM test");
    {
        moment_t moment;
        moment_t moment_ref = {90.5};

        NEW_STEP("Force moment FROM Nm test");
        moment = ForceOD_MomentFrom_N_m(90.5);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
        NEW_STEP("Force moment FROM Nmm test");
        moment = ForceOD_MomentFrom_N_mm(90500);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
        NEW_STEP("Force moment FROM Ncm test");
        moment = ForceOD_MomentFrom_N_cm(9050);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
        NEW_STEP("Force moment FROM Kgf/mm test");
        moment = ForceOD_MomentFrom_kgf_mm(9228.43172745);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
        NEW_STEP("Force moment FROM Kgf/cm test");
        moment = ForceOD_MomentFrom_kgf_cm(922.843172745);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
        NEW_STEP("Force moment FROM Kgf/m test");
        moment = ForceOD_MomentFrom_kgf_m(9.22843172745);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
        NEW_STEP("Force moment FROM ozf/in test");
        moment = ForceOD_MomentFrom_ozf_in(12815.87956868);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
        NEW_STEP("Force moment FROM lbf/in test");
        moment = ForceOD_MomentFrom_lbf_in(800.9924635902);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
    }
    NEW_TEST_CASE("Force moment TO test");
    {
        moment_t moment = {90.5};

        NEW_STEP("Force moment TO Nm test");
        float n_m = ForceOD_MomentTo_N_m(moment);
        TEST_ASSERT_EQUAL(90.5, n_m);
        NEW_STEP("Force moment TO Nmm test");
        float n_mm = ForceOD_MomentTo_N_mm(moment);
        TEST_ASSERT_EQUAL(90500, n_mm);
        NEW_STEP("Force moment TO Ncm test");
        float n_cm = ForceOD_MomentTo_N_cm(moment);
        TEST_ASSERT_EQUAL(9050, n_cm);
        NEW_STEP("Force moment TO Kgf/mm test");
        float kgf_mm = ForceOD_MomentTo_kgf_mm(moment);
        TEST_ASSERT_EQUAL(9228.43172745, kgf_mm);
        NEW_STEP("Force moment TO Kgf/cm test");
        float kgf_cm = ForceOD_MomentTo_kgf_cm(moment);
        TEST_ASSERT_EQUAL(923, kgf_cm);
        NEW_STEP("Force moment TO Kgf/m test");
        float kgf_m = ForceOD_MomentTo_kgf_m(moment);
        TEST_ASSERT_EQUAL(9.22843172745, kgf_m);
        NEW_STEP("Force moment TO ozf/in test");
        float ozf_in = ForceOD_MomentTo_ozf_in(moment);
        TEST_ASSERT_EQUAL(12815.87956868, ozf_in);
        NEW_STEP("Force moment TO lbf/in test");
        float lbf_in = ForceOD_MomentTo_lbf_in(moment);
        TEST_ASSERT_EQUAL(801, lbf_in);
    }
    NEW_TEST_CASE("Force moment msg conversion test");
    {
        moment_t moment;
        moment_t moment_ref = {90.5};
        msg_t msg_ref;
        msg_t msg;

        NEW_STEP("Force moment msg conversion FROM test");
        msg_ref.header.cmd  = MOMENT;
        msg_ref.header.size = sizeof(moment_t);
        memcpy(msg_ref.data, &moment_ref, sizeof(moment_t));
        ForceOD_MomentFromMsg(&moment, &msg_ref);
        TEST_ASSERT_EQUAL((uint32_t)moment_ref._private, (uint32_t)moment._private);
        NEW_STEP("Force moment msg conversion TO test");
        ForceOD_MomentToMsg(&moment_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((moment_t *)msg_ref.data)->_private, (uint32_t)((moment_t *)msg.data)->_private);
    }
    NEW_TEST_CASE("Force moment msg conversion wrong values test");
    {
        RESET_ASSERT();
        moment_t moment;
        msg_t msg;
        TRY
        {
            NEW_STEP("Force moment msg conversion TO wrong msg_t* value test");
            ForceOD_MomentToMsg(&moment, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force moment msg conversion TO wrong moment_t* value test");
            ForceOD_MomentToMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force moment msg conversion FROM wrong msg_t* value test");
            ForceOD_MomentFromMsg(&moment, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            NEW_STEP("Force moment msg conversion FROM wrong moment_t* value test");
            ForceOD_MomentFromMsg(NULL, &msg);
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
        TEST_ASSERT_EQUAL((uint32_t)force_ref._private, (uint32_t)force._private);
        NEW_STEP("Force force FROM Kgf test");
        force = ForceOD_ForceFrom_kgf(9.22843172745);
        TEST_ASSERT_EQUAL((uint32_t)force_ref._private, (uint32_t)force._private);
        NEW_STEP("Force force FROM ozf test");
        force = ForceOD_ForceFrom_ozf(325.52334961);
        TEST_ASSERT_EQUAL((uint32_t)force_ref._private, (uint32_t)force._private);
        NEW_STEP("Force force FROM lbf test");
        force = ForceOD_ForceFrom_lbf(20.345209351);
        TEST_ASSERT_EQUAL((uint32_t)force_ref._private, (uint32_t)force._private);
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
        TEST_ASSERT_EQUAL((uint32_t)force_ref._private, (uint32_t)force._private);
        NEW_STEP("Force force msg conversion TO test");
        ForceOD_ForceToMsg(&force_ref, &msg);
        TEST_ASSERT_EQUAL(msg_ref.header.cmd, msg.header.cmd);
        TEST_ASSERT_EQUAL(msg_ref.header.size, msg.header.size);
        TEST_ASSERT_EQUAL((uint32_t)((force_t *)msg_ref.data)->_private, (uint32_t)((force_t *)msg.data)->_private);
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
    UNIT_TEST_RUN(unittest_Od_forceMoment);
    UNIT_TEST_RUN(unittest_Od_forceForce);

    UNITY_END();
}
