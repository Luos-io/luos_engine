#include "main.h"
#include "unit_test.h"

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    ASSERT_ACTIVATION(1);

    // Streaming functions
    UNIT_TEST_RUN(unittest_Streaming_SendStreamingSize);

    UNITY_END();
}
