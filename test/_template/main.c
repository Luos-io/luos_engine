#include "main.h"
#include "unit_test.h"

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    UNIT_TEST_RUN(unittest_TEMPLATE);
    UNIT_TEST_RUN(unittest_TEMPLATE_2);

    UNITY_END();
}
