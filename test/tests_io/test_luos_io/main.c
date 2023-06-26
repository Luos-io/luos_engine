#include "unit_test.h"
#include "../src/luos_io.c"
#include "../src/luos_phy.c"
#include "../src/msg_alloc.c"
#include "../src/service.c"
#include "../src/routing_table.c"

phy_job_t *Luos_handled_job  = NULL;
phy_job_t *Robus_handled_job = NULL;

static void phy_luos_MsgHandler(luos_phy_t *phy_ptr, phy_job_t *job)
{
    Luos_handled_job = job;
}

static void phy_robus_MsgHandler(luos_phy_t *phy_ptr, phy_job_t *job)
{
    Robus_handled_job = job;
}

error_return_t topo_cb(luos_phy_t *phy_ptr, uint8_t *portId)
{
    return SUCCEED;
}

void reset_cb(luos_phy_t *phy_ptr)
{
}

static void luosIO_reset_overlap_callback(void)
{
    LuosIO_Init();
    // Overlap the normal callbacks
    luos_phy = Phy_Get(0, phy_luos_MsgHandler, topo_cb, reset_cb);
    Phy_Get(1, phy_robus_MsgHandler, topo_cb, reset_cb);
    TEST_ASSERT_EQUAL(&phy_ctx.phy[0], luos_phy);
}

/*******************************************************************************
 * File function
 ******************************************************************************/

void unittest_luosIO_init()
{
    NEW_TEST_CASE("Basic init test");
    {
        TRY
        {
            luos_phy = NULL;
            LuosIO_Init();
            TEST_ASSERT_EQUAL(&phy_ctx.phy[0], luos_phy);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_luosIO_loop()
{

    NEW_TEST_CASE("Check that phy treatment is done in loop");
    {
        TRY
        {
            LuosIO_Init();
            phy_ctx.phy[1].rx_alloc_job   = true;
            phy_ctx.phy[1].received_data  = sizeof(header_t);
            phy_ctx.phy[1].rx_buffer_base = (uint8_t *)msg_buffer;
            phy_ctx.phy[1].rx_data        = (uint8_t *)msg_buffer;
            phy_ctx.phy[1].rx_keep        = true;
            // Message computed details
            phy_ctx.phy[1].rx_size       = 10;
            phy_ctx.phy[1].rx_phy_filter = 1;
            LuosIO_Loop();
            TEST_ASSERT_EQUAL(false, phy_ctx.phy[1].rx_alloc_job);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check detection assertion condition in loop");
    {
        TRY
        {
            LuosIO_Init();
            Flag_DetectServices = true;
            detection_service   = NULL;
            LuosIO_Loop();
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check that the detection treatment is done in loop");
    {
        TRY
        {
            LuosIO_Init();
            Flag_DetectServices = true;
            detection_service   = &service_ctx.list[0];
            LuosIO_Loop();
            TEST_ASSERT_EQUAL(NULL, detection_service);
            TEST_ASSERT_EQUAL(false, Flag_DetectServices);
            TEST_ASSERT_EQUAL(1, service_ctx.list[0].id);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_luosIO_TransmitLocalRoutingTable()
{
    NEW_TEST_CASE("Check assert condition for TransmitLocalRoutingTable");
    {
        TRY
        {
            LuosIO_Init();
            LuosIO_TransmitLocalRoutingTable(NULL, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check that the routing table is sent");
    {
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Luos_handled_job       = NULL;
            Node_Get()->node_id    = 1;
            service_ctx.number     = 2;
            msg.header.cmd         = RTB;
            msg.header.target_mode = NODEIDACK;
            msg.header.target      = 1;

            LuosIO_TransmitLocalRoutingTable(NULL, &msg);
            TEST_ASSERT_NOT_EQUAL(NULL, Luos_handled_job);
            TEST_ASSERT_EQUAL(RTB, Luos_handled_job->msg_pt->header.cmd);
            TEST_ASSERT_EQUAL(1, Luos_handled_job->msg_pt->header.target);
            TEST_ASSERT_EQUAL(3 * sizeof(routing_table_t), Luos_handled_job->msg_pt->header.size);
            TEST_ASSERT_EQUAL(NODEIDACK, Luos_handled_job->msg_pt->header.target_mode);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_luosIO_ConsumeMsg()
{
    connection_t con_table[5];

    NEW_TEST_CASE("Check assert condition for ConsumeMsg");
    {
        TRY
        {
            LuosIO_Init();
            LuosIO_ConsumeMsg(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            LuosIO_Init();
            msg_t msg;
            msg.header.cmd       = CONNECTION_DATA;
            connection_table_ptr = NULL;
            LuosIO_ConsumeMsg(&msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            LuosIO_Init();
            msg_t msg;
            msg.header.cmd       = PORT_DATA;
            connection_table_ptr = NULL;
            LuosIO_ConsumeMsg(&msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            LuosIO_Init();
            msg_t msg;
            msg.header.cmd              = PORT_DATA;
            msg.header.size             = 0;
            connection_table_ptr        = con_table;
            con_table[0].parent.node_id = 1;
            LuosIO_ConsumeMsg(&msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            LuosIO_Init();
            msg_t msg;
            msg.header.cmd                          = PORT_DATA;
            msg.header.size                         = sizeof(port_t);
            connection_table_ptr                    = con_table;
            con_table[last_node - 1].parent.node_id = 0xFFFF;
            LuosIO_ConsumeMsg(&msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            LuosIO_Init();
            msg_t msg;
            msg.header.cmd                      = PORT_DATA;
            msg.header.size                     = sizeof(connection_t) + 1;
            connection_table_ptr                = con_table;
            con_table[last_node].parent.node_id = 20;
            LuosIO_ConsumeMsg(&msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check CONNECTION_DATA size of a port_ treatment (topology retriving)");
    {
        TRY
        {
            luosIO_reset_overlap_callback();
            last_node           = 1;
            Node_Get()->node_id = 1;
            msg_t msg;
            msg.header.cmd  = CONNECTION_DATA;
            msg.header.size = sizeof(port_t);
            port_t port;
            port.node_id = 1;
            port.phy_id  = 2;
            port.port_id = 3;
            memcpy(msg.data, &port, sizeof(port_t));
            memset(connection_table_ptr, 0xFF, sizeof(connection_t) * 2);

            LuosIO_ConsumeMsg(&msg);
            TEST_ASSERT_EQUAL(2, last_node);

            TEST_ASSERT_EQUAL(1, connection_table_ptr[1].parent.node_id);
            TEST_ASSERT_EQUAL(2, connection_table_ptr[1].parent.phy_id);
            TEST_ASSERT_EQUAL(3, connection_table_ptr[1].parent.port_id);

            TEST_ASSERT_EQUAL(0xFFFF, connection_table_ptr[0].parent.node_id);
            TEST_ASSERT_EQUAL(0xFF, connection_table_ptr[0].parent.phy_id);
            TEST_ASSERT_EQUAL(0xFF, connection_table_ptr[0].parent.port_id);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check CONNECTION_DATA size of a connection_t treatment (topology retriving)");
    {
        TRY
        {
            luosIO_reset_overlap_callback();
            last_node           = 1;
            Node_Get()->node_id = 1;
            msg_t msg;
            msg.header.cmd  = CONNECTION_DATA;
            msg.header.size = 2 * sizeof(connection_t);
            connection_t con[2];
            con[0].parent.node_id = 2;
            con[0].parent.phy_id  = 1;
            con[0].parent.port_id = 3;
            con[0].child.node_id  = 4;
            con[0].child.phy_id   = 5;
            con[0].child.port_id  = 6;
            con[1].parent.node_id = 7;
            con[1].parent.phy_id  = 8;
            con[1].parent.port_id = 9;
            con[1].child.node_id  = 3;
            con[1].child.phy_id   = 11;
            con[1].child.port_id  = 12;

            memcpy(msg.data, con, 2 * sizeof(connection_t));
            memset(connection_table_ptr, 0xFF, sizeof(connection_t) * 2);

            LuosIO_ConsumeMsg(&msg);
            TEST_ASSERT_EQUAL(3, last_node);

            TEST_ASSERT_EQUAL(2, connection_table_ptr[1].parent.node_id);
            TEST_ASSERT_EQUAL(1, connection_table_ptr[1].parent.phy_id);
            TEST_ASSERT_EQUAL(3, connection_table_ptr[1].parent.port_id);

            TEST_ASSERT_EQUAL(3, connection_table_ptr[2].child.node_id);
            TEST_ASSERT_EQUAL(11, connection_table_ptr[2].child.phy_id);
            TEST_ASSERT_EQUAL(12, connection_table_ptr[2].child.port_id);

            TEST_ASSERT_EQUAL(0xFFFF, connection_table_ptr[0].parent.node_id);
            TEST_ASSERT_EQUAL(0xFF, connection_table_ptr[0].parent.phy_id);
            TEST_ASSERT_EQUAL(0xFF, connection_table_ptr[0].parent.port_id);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check NODE_ID reception");
    {
        TRY
        {
            luosIO_reset_overlap_callback();
            Luos_handled_job    = NULL;
            Robus_handled_job   = NULL;
            Node_Get()->node_id = 0;
            msg_t msg;
            msg.header.cmd    = NODE_ID;
            msg.header.size   = 2;
            msg.header.source = 1;
            uint16_t node_id  = 2;
            memcpy(msg.data, &node_id, sizeof(uint16_t));

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_EQUAL(NULL, Luos_handled_job);
            TEST_ASSERT_EQUAL(NULL, Robus_handled_job);
            TEST_ASSERT_EQUAL(2, Node_Get()->node_id);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check PORT_DATA treatment (topology retriving)");
    {
        TRY
        {
            luosIO_reset_overlap_callback();
            Luos_handled_job                       = NULL;
            last_node                              = 2;
            connection_table_ptr[1].parent.node_id = 2;
            Node_Get()->node_id                    = 1;
            msg_t msg;
            msg.header.cmd  = PORT_DATA;
            msg.header.size = sizeof(port_t);
            port_t port;
            port.node_id = 1;
            port.phy_id  = 2;
            port.port_id = 3;
            memcpy(msg.data, &port, sizeof(port_t));

            LuosIO_ConsumeMsg(&msg);

            TEST_ASSERT_EQUAL(1, connection_table_ptr[1].child.node_id);
            TEST_ASSERT_EQUAL(2, connection_table_ptr[1].child.phy_id);
            TEST_ASSERT_EQUAL(3, connection_table_ptr[1].child.port_id);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check LOCAL_RTB assert condition");
    {
        TRY
        {
            luosIO_reset_overlap_callback();
            Luos_handled_job    = NULL;
            last_node           = 1;
            Node_Get()->node_id = 1;
            msg_t msg;
            msg.header.cmd    = LOCAL_RTB;
            msg.header.size   = 3;
            msg.header.source = 1;

            LuosIO_ConsumeMsg(&msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check LOCAL_RTB size 0 treatment (send of the local rtb)");
    {
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Luos_handled_job       = NULL;
            Robus_handled_job      = NULL;
            Node_Get()->node_id    = 1;
            service_ctx.number     = 2;
            service_ctx.list[0].id = 1;
            // Generate the filters
            Service_GenerateId(1);

            msg.header.cmd    = LOCAL_RTB;
            msg.header.size   = 0;
            msg.header.source = 1;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_NOT_EQUAL(NULL, Luos_handled_job);
            TEST_ASSERT_EQUAL(NULL, Robus_handled_job);
            TEST_ASSERT_EQUAL(RTB, Luos_handled_job->msg_pt->header.cmd);
            TEST_ASSERT_EQUAL(1, Luos_handled_job->msg_pt->header.target);
            TEST_ASSERT_EQUAL(3 * sizeof(routing_table_t), Luos_handled_job->msg_pt->header.size);
            TEST_ASSERT_EQUAL(SERVICEIDACK, Luos_handled_job->msg_pt->header.target_mode);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check LOCAL_RTB size 2 treatment (generation of service id + send of local rtb)");
    {
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Luos_handled_job       = NULL;
            Robus_handled_job      = NULL;
            Node_Get()->node_id    = 1;
            service_ctx.number     = 2;
            service_ctx.list[0].id = 1;
            // Generate the filters
            Service_GenerateId(1);

            msg.header.cmd    = LOCAL_RTB;
            msg.header.size   = 2;
            msg.header.source = 1;
            uint16_t first_id = 1;
            memcpy((void *)msg.data, (void *)&first_id, sizeof(uint16_t));

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_NOT_EQUAL(NULL, Luos_handled_job);
            TEST_ASSERT_EQUAL(NULL, Robus_handled_job);
            TEST_ASSERT_EQUAL(RTB, Luos_handled_job->msg_pt->header.cmd);
            TEST_ASSERT_EQUAL(1, Luos_handled_job->msg_pt->header.target);
            TEST_ASSERT_EQUAL(3 * sizeof(routing_table_t), Luos_handled_job->msg_pt->header.size);
            TEST_ASSERT_EQUAL(SERVICEIDACK, Luos_handled_job->msg_pt->header.target_mode);
            routing_table_t rtb[3];
            memcpy((void *)rtb, (void *)Luos_handled_job->msg_pt->data, 3 * sizeof(routing_table_t));
            TEST_ASSERT_EQUAL(NODE, rtb[0].mode);
            TEST_ASSERT_EQUAL(SERVICE, rtb[1].mode);
            TEST_ASSERT_EQUAL(SERVICE, rtb[2].mode);
            TEST_ASSERT_EQUAL(1, rtb[1].id);
            TEST_ASSERT_EQUAL(2, rtb[2].id);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check START_DETECTION");
    {
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            phy_ctx.io_job_nb = 3;
            Node_SetState(DETECTION_OK);
            service_ctx.list[0].id = 1;
            service_ctx.list[1].id = 2;

            msg.header.cmd  = START_DETECTION;
            msg.header.size = 0;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_EQUAL(0, phy_ctx.io_job_nb);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check END_DETECTION");
    {
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Node_SetState(EXTERNAL_DETECTION);
            service_ctx.list[0].id = 1;
            service_ctx.list[1].id = 2;

            msg.header.cmd  = END_DETECTION;
            msg.header.size = 0;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
            TEST_ASSERT_EQUAL(DETECTION_OK, Node_GetState());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check ASK_DETECTION");
    {
        // No detection running
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Node_SetState(NO_DETECTION);
            detection_service      = NULL;
            msg.header.cmd         = ASK_DETECTION;
            msg.header.target_mode = BROADCAST;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_EQUAL(true, Flag_DetectServices);
            TEST_ASSERT_EQUAL(&service_ctx.list[0], detection_service);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        // Detection running
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Node_SetState(LOCAL_DETECTION);
            detection_service      = NULL;
            Flag_DetectServices    = false;
            msg.header.cmd         = ASK_DETECTION;
            msg.header.target_mode = BROADCAST;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_EQUAL(false, Flag_DetectServices);
            TEST_ASSERT_EQUAL(NULL, detection_service);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check ASSERT");
    {
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            detection_service      = NULL;
            msg.header.cmd         = ASSERT;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;

            routing_table_t *rtb     = RoutingTB_Get();
            rtb[0].mode              = NODE;
            rtb[0].node_id           = 1;
            rtb[1].mode              = SERVICE;
            rtb[1].id                = 1;
            last_routing_table_entry = 2;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
            TEST_ASSERT_EQUAL(1, last_routing_table_entry);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check DEADTARGET");
    {
        // Dead node
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            detection_service      = NULL;
            msg.header.cmd         = DEADTARGET;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            msg.header.size        = sizeof(dead_target_t);

            dead_target_t *dead_target = (dead_target_t *)msg.data;
            dead_target->node_id       = 1;
            dead_target->service_id    = 0;

            routing_table_t *rtb     = RoutingTB_Get();
            rtb[0].mode              = NODE;
            rtb[0].node_id           = 1;
            rtb[1].mode              = SERVICE;
            rtb[1].id                = 1;
            last_routing_table_entry = 2;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
            TEST_ASSERT_EQUAL(1, last_routing_table_entry);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        // Dead service
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            detection_service      = NULL;
            msg.header.cmd         = DEADTARGET;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            msg.header.size        = sizeof(dead_target_t);

            dead_target_t *dead_target = (dead_target_t *)msg.data;
            dead_target->node_id       = 0;
            dead_target->service_id    = 1;

            routing_table_t *rtb     = RoutingTB_Get();
            rtb[0].mode              = NODE;
            rtb[0].node_id           = 1;
            rtb[1].mode              = SERVICE;
            rtb[1].id                = 1;
            last_routing_table_entry = 2;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
            TEST_ASSERT_EQUAL(1, last_routing_table_entry);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check REVISION");
    {
        // User send a revivision message with a revision on it
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            msg.header.cmd         = REVISION;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            msg.header.size        = 2;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        // manage a revision request
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Luos_handled_job  = NULL;
            Robus_handled_job = NULL;

            Node_Get()->node_id    = 1;
            service_ctx.number     = 2;
            service_ctx.list[0].id = 1;
            // Generate the filters
            Service_GenerateId(1);

            msg.header.cmd         = REVISION;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            msg.header.size        = 0;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_NOT_EQUAL(NULL, Luos_handled_job);
            revision_t revision;
            memcpy(&revision, Luos_handled_job->msg_pt->data, sizeof(revision_t));
            TEST_ASSERT_EQUAL(service_ctx.list[0].revision.major, revision.major);
            TEST_ASSERT_EQUAL(sizeof(revision_t), Luos_handled_job->msg_pt->header.size);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check LUOS_REVISION");
    {
        // User send a revivision message with a revision on it
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            msg.header.cmd         = LUOS_REVISION;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            msg.header.size        = 2;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        // manage a revision request
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Luos_handled_job  = NULL;
            Robus_handled_job = NULL;

            Node_Get()->node_id    = 1;
            service_ctx.number     = 2;
            service_ctx.list[0].id = 1;
            // Generate the filters
            Service_GenerateId(1);

            msg.header.cmd         = LUOS_REVISION;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            msg.header.size        = 0;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_NOT_EQUAL(NULL, Luos_handled_job);
            revision_t revision;
            memcpy(&revision, Luos_handled_job->msg_pt->data, sizeof(revision_t));
            const revision_t *luos_version = Luos_GetVersion();
            TEST_ASSERT_EQUAL(luos_version->major, revision.major);
            TEST_ASSERT_EQUAL(sizeof(revision_t), Luos_handled_job->msg_pt->header.size);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check LUOS_STATISTICS");
    {
        // User send a revivision message with a revision on it
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            msg.header.cmd         = LUOS_STATISTICS;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            msg.header.size        = 2;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        // manage a revision request
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Luos_handled_job  = NULL;
            Robus_handled_job = NULL;

            Node_Get()->node_id    = 1;
            service_ctx.number     = 2;
            service_ctx.list[0].id = 1;
            // Generate the filters
            Service_GenerateId(1);

            msg.header.cmd         = WRITE_ALIAS;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            memcpy(msg.data, "test", 4);
            msg.header.size = 4;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
            TEST_ASSERT_EQUAL_STRING("test", service_ctx.list[0].alias);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check UPDATE_PUB");
    {
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            Luos_handled_job  = NULL;
            Robus_handled_job = NULL;

            Node_Get()->node_id                          = 1;
            service_ctx.number                           = 2;
            service_ctx.list[0].id                       = 1;
            service_ctx.list[0].auto_refresh.target      = 0;
            service_ctx.list[0].auto_refresh.time_ms     = 0;
            service_ctx.list[0].auto_refresh.last_update = 0;
            // Generate the filters
            Service_GenerateId(1);

            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            time_luos_t time       = TimeOD_TimeFrom_s(1);
            TimeOD_TimeToMsg(&time, &msg);
            msg.header.cmd = UPDATE_PUB;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(SUCCEED, ret_val);
            TEST_ASSERT_EQUAL(1, service_ctx.list[0].auto_refresh.target);
            TEST_ASSERT_EQUAL((uint16_t)TimeOD_TimeTo_ms(time), service_ctx.list[0].auto_refresh.time_ms);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check BOOTLOADER_CMD");
    {
        // User send a revivision message with a revision on it
        TRY
        {
            msg_t msg;
            luosIO_reset_overlap_callback();
            msg.header.cmd         = BOOTLOADER_START;
            msg.header.target_mode = BROADCAST;
            msg.header.source      = 1;
            msg.header.size        = 0;

            error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

            // Check received message content
            TEST_ASSERT_EQUAL(FAILED, ret_val);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check any other cmd");
    {
        for (uint8_t i = LUOS_LAST_RESERVED_CMD; i < 0xFF; i++)
        {
            TRY
            {
                msg_t msg;
                luosIO_reset_overlap_callback();
                msg.header.cmd         = i;
                msg.header.target_mode = BROADCAST;
                msg.header.source      = 1;
                msg.header.size        = 0;

                error_return_t ret_val = LuosIO_ConsumeMsg(&msg);

                // Check received message content
                TEST_ASSERT_EQUAL(FAILED, ret_val);
            }
            CATCH
            {
                TEST_ASSERT_TRUE(false);
            }
            END_TRY;
        }
    }
}

void unittest_luosIO_DetectNextNodes()
{
    NEW_TEST_CASE("This function is highly intricated with Robus for now. It makes it difficult to test. We will keep it for later");
    {
    }
}

void unittest_luosIO_GetNextJob()
{
    NEW_TEST_CASE("Check assert condition for GetNextJob");
    {
        TRY
        {
            LuosIO_Init();
            LuosIO_GetNextJob(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check GetNextJob");
    {
        LuosIO_Init();
        phy_job_t *job                = NULL;
        luos_phy->job_nb              = 2;
        luos_phy->oldest_job_index    = 0;
        luos_phy->available_job_index = 2;
        for (uint8_t i = 0; i < 2; i++)
        {
            TRY
            {
                luos_phy->job[0].data_pt = (uint8_t *)&msg_buffer[0];
                luos_phy->job[1].data_pt = (uint8_t *)&msg_buffer[1];

                error_return_t returned_val = LuosIO_GetNextJob(&job);

                TEST_ASSERT_EQUAL(SUCCEED, returned_val);
                TEST_ASSERT_EQUAL(&msg_buffer[i], job->data_pt);
            }
            CATCH
            {
                TEST_ASSERT_TRUE(false);
            }
            END_TRY;
        }

        TRY
        {
            error_return_t returned_val = LuosIO_GetNextJob(&job);

            TEST_ASSERT_EQUAL(FAILED, returned_val);
            TEST_ASSERT_EQUAL(NULL, job);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_luosIO_RmJob()
{
    NEW_TEST_CASE("Check assert condition for RmJob");
    {
        TRY
        {
            LuosIO_Init();
            LuosIO_RmJob(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            LuosIO_Init();
            luos_phy->job_nb              = 3;
            luos_phy->oldest_job_index    = 0;
            luos_phy->available_job_index = 3;
            LuosIO_RmJob(&luos_phy->job[luos_phy->job_nb]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check RmJob");
    {
        TRY
        {
            LuosIO_Init();
            service_filter[0]             = 0x00;
            service_filter[1]             = 0x01;
            phy_job_t *job                = NULL;
            luos_phy->job_nb              = 2;
            luos_phy->oldest_job_index    = 0;
            luos_phy->available_job_index = 2;
            luos_phy->job[0].data_pt      = (uint8_t *)&msg_buffer[0];
            luos_phy->job[0].phy_data     = &service_filter[0];
            luos_phy->job[1].data_pt      = (uint8_t *)&msg_buffer[1];
            luos_phy->job[1].phy_data     = &service_filter[1];

            LuosIO_RmJob(&luos_phy->job[0]);
            TEST_ASSERT_EQUAL(1, luos_phy->job_nb);
            TEST_ASSERT_EQUAL(NULL, luos_phy->job[0].data_pt);
            TEST_ASSERT_EQUAL(&msg_buffer[1], luos_phy->job[1].data_pt);

            // Job 2 is not removed because service_filter is not equal to 0x00
            LuosIO_RmJob(&luos_phy->job[1]);
            TEST_ASSERT_EQUAL(1, luos_phy->job_nb);
            TEST_ASSERT_EQUAL(&msg_buffer[1], luos_phy->job[1].data_pt);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_luosIO_GetJobNb()
{
    NEW_TEST_CASE("Check GetJobNb");
    {
        TRY
        {
            LuosIO_Init();
            luos_phy->job_nb = 3;

            uint16_t jobNb = LuosIO_GetJobNb();
            TEST_ASSERT_EQUAL(luos_phy->job_nb, jobNb);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    UNIT_TEST_RUN(unittest_luosIO_init);
    UNIT_TEST_RUN(unittest_luosIO_loop);
    UNIT_TEST_RUN(unittest_luosIO_TransmitLocalRoutingTable);
    UNIT_TEST_RUN(unittest_luosIO_ConsumeMsg);
    UNIT_TEST_RUN(unittest_luosIO_DetectNextNodes);
    UNIT_TEST_RUN(unittest_luosIO_GetNextJob);
    UNIT_TEST_RUN(unittest_luosIO_RmJob);
    UNIT_TEST_RUN(unittest_luosIO_GetJobNb);

    UNITY_END();
}
