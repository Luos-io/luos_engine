/******************************************************************************
 * @file fingerprint_com
 * @brief This is a driver example for a fingerprint sensor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/L432KC-Luos-Biometric-Security-System
 * @author mariebidouille
 * @version 0.0.0
 ******************************************************************************/
#ifndef FINGERPRINT_COM_H
#define FINGERPRINT_COM_H

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_system.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_TRY      500
#define MAX_TEMPLATE 127

#define FINGERPRINT_BAUDRATE 57600
#define FINGERPRINT_CAPACITY 64
#define FINGERPRINT_ADDRESS  0xFFFFFFFF

typedef struct
{
    uint16_t start_code;
    uint8_t address[4];
    uint8_t type;
    uint16_t length;
    uint8_t data[64];
} fingerprint_packet;

/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#define FINGERPRINT_CLOCK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define FINGERPRINT_COM_TX_PIN  GPIO_PIN_2
#define FINGERPRINT_COM_TX_PORT GPIOA
#define FINGERPRINT_COM_TX_AF   GPIO_AF7_USART2

#define FINGERPRINT_COM_RX_PIN  GPIO_PIN_3
#define FINGERPRINT_COM_RX_PORT GPIOA
#define FINGERPRINT_COM_RX_AF   GPIO_AF7_USART2

/*******************************************************************************
 * FINGERPRINT_COM CONFIG
 ******************************************************************************/
#define FINGERPRINT_COM_CLOCK_ENABLE() __HAL_RCC_USART2_CLK_ENABLE()

#define FINGERPRINT_COM              USART2
#define FINGERPRINT_COM_IRQ          USART2_IRQn
#define FINGERPRINT_COM_IRQHANDLER() USART2_IRQHandler()

/*******************************************************************************
 * INSTRUCTIONS CODE
 ******************************************************************************/
#define FINGERPRINT_OK               0x00 //!< Command execution is complete
#define FINGERPRINT_PACKETRECIEVEERR 0x01 //!< Error when receiving data package
#define FINGERPRINT_NOFINGER         0x02 //!< No finger on the sensor
#define FINGERPRINT_IMAGEFAIL        0x03 //!< Failed to enroll the finger
#define FINGERPRINT_IMAGEMESS \
    0x06 //!< Failed to generate character file due to overly disorderly \
         //!< fingerprint image
#define FINGERPRINT_FEATUREFAIL \
    0x07                          //!< Failed to generate character file due to the lack of character point \
                                  //!< or small fingerprint image
#define FINGERPRINT_NOMATCH  0x08 //!< Finger doesn't match
#define FINGERPRINT_NOTFOUND 0x09 //!< Failed to find matching finger
#define FINGERPRINT_ENROLLMISMATCH \
    0x0A //!< Failed to combine the character files
#define FINGERPRINT_BADLOCATION \
    0x0B //!< Addressed PageID is beyond the finger library
#define FINGERPRINT_DBRANGEFAIL \
    0x0C                                   //!< Error when reading template from library or invalid template
#define FINGERPRINT_UPLOADFEATUREFAIL 0x0D //!< Error when uploading template
#define FINGERPRINT_PACKETRESPONSEFAIL \
    0x0E                             //!< Module failed to receive the following data packages
#define FINGERPRINT_UPLOADFAIL  0x0F //!< Error when uploading image
#define FINGERPRINT_DELETEFAIL  0x10 //!< Failed to delete the template
#define FINGERPRINT_DBCLEARFAIL 0x11 //!< Failed to clear finger library
#define FINGERPRINT_PASSFAIL \
    0x13 //!< Find whether the fingerprint passed or failed
#define FINGERPRINT_INVALIDIMAGE \
    0x15                            //!< Failed to generate image because of lac of valid primary image
#define FINGERPRINT_FLASHERR   0x18 //!< Error when writing flash
#define FINGERPRINT_INVALIDREG 0x1A //!< Invalid register number
#define FINGERPRINT_ADDRCODE   0x20 //!< Address code
#define FINGERPRINT_PASSVERIFY 0x21 //!< Verify the fingerprint passed
#define FINGERPRINT_STARTCODE \
    0xEF01 //!< Fixed falue of EF01H; High byte transferred first

#define FINGERPRINT_COMMANDPACKET 0x1 //!< Command packet
#define FINGERPRINT_DATAPACKET \
    0x2                               //!< Data packet, must follow command packet or acknowledge packet
#define FINGERPRINT_ACKPACKET     0x7 //!< Acknowledge packet
#define FINGERPRINT_ENDDATAPACKET 0x8 //!< End of data packet

#define FINGERPRINT_TIMEOUT   0xFF //!< Timeout was reached
#define FINGERPRINT_BADPACKET 0xFE //!< Bad packet was sent

#define FINGERPRINT_GETIMAGE 0x01 //!< Collect finger image
#define FINGERPRINT_IMAGE2TZ 0x02 //!< Generate character file from image
#define FINGERPRINT_SEARCH   0x04 //!< Search for fingerprint in slot
#define FINGERPRINT_REGMODEL \
    0x05                                //!< Combine character files and generate template
#define FINGERPRINT_STORE          0x06 //!< Store template
#define FINGERPRINT_LOAD           0x07 //!< Read/load template
#define FINGERPRINT_UPLOAD         0x08 //!< Upload template
#define FINGERPRINT_DELETE         0x0C //!< Delete templates
#define FINGERPRINT_EMPTY          0x0D //!< Empty library
#define FINGERPRINT_READSYSPARAM   0x0F //!< Read system parameters
#define FINGERPRINT_SETPASSWORD    0x12 //!< Sets passwords
#define FINGERPRINT_VERIFYPASSWORD 0x13 //!< Verifies the password
#define FINGERPRINT_HISPEEDSEARCH \
    0x1B                               //!< Asks the sensor to search for a matching fingerprint template to the \
                                       //!< last model generated
#define FINGERPRINT_TEMPLATECOUNT 0x1D //!< Read finger template numbers
#define FINGERPRINT_AURALEDCONFIG 0x35 //!< Aura LED control
#define FINGERPRINT_LEDON         0x50 //!< Turn on the onboard LED
#define FINGERPRINT_LEDOFF        0x51 //!< Turn off the onboard LED

#define FINGERPRINT_LED_BREATHING   0x01 //!< Breathing light
#define FINGERPRINT_LED_FLASHING    0x02 //!< Flashing light
#define FINGERPRINT_LED_ON          0x03 //!< Always on
#define FINGERPRINT_LED_OFF         0x04 //!< Always off
#define FINGERPRINT_LED_GRADUAL_ON  0x05 //!< Gradually on
#define FINGERPRINT_LED_GRADUAL_OFF 0x06 //!< Gradually off
#define FINGERPRINT_LED_RED         0x01 //!< Red LED
#define FINGERPRINT_LED_BLUE        0x02 //!< Blue LED
#define FINGERPRINT_LED_PURPLE      0x03 //!< Purple LED

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
uint16_t Fingerprint_GetTemplateCount(void);
uint8_t Fingerprint_LEDcontrol(uint8_t on);
uint8_t Fingerprint_GetImage(void);
uint8_t Fingerprint_Image2Tz(uint8_t slot);
uint8_t Fingerprint_CreateModel(void);
uint8_t Fingerprint_StoreModel(uint16_t location);
uint8_t Fingerprint_EmptyDatabase(void);
uint8_t Fingerprint_SearchFinger(void);

void FingerprintCom_Init(uint32_t baudrate);

#endif /* FINGERPRINT_COM_H */
