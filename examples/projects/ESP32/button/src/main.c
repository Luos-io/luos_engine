/* button Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "luos_engine.h"
#include "robus.h"
#include "button.h"

void app_main(void)
{
    Luos_Init();
    Robus_Init();
    Button_Init();

    while (1)
    {
        Luos_Loop();
        Button_Loop();
    }
}
