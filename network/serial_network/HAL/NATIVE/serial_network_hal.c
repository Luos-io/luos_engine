/******************************************************************************
 * @file serial_hal
 * @brief serial communication hardware abstraction layer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "serial_network_hal.h"
#include "_serial_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luos_utils.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <termios.h>
    #include <errno.h>
    #include <sys/ioctl.h>
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t rx_buffer_size;
#ifdef _WIN32
HANDLE hSerial;
#else
int serial_port;
#endif

char portname[128];

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Initialisation of the Serial communication
 * @param None
 * @return None
 * ****************************************************************************/
char *stripStr(char *string)
{
    // non_space_count to keep the frequency of non space characters
    int ok_char_count = 0;

    // Traverse a string and if it is non space or carriage return character then, place it at index non_space_count
    for (int i = 0; string[i] != '\0'; i++)
    {
        if ((string[i] != ' ') && (string[i] != '\n') && (string[i] != '\r'))
        {
            // This character is OK
            string[ok_char_count] = string[i];
            ok_char_count++; // non_space_count incremented
        }
    }
    // Finally placing final character at the string end
    string[ok_char_count] = '\0';
    return string;
}

#ifdef _WIN32
void set_serial_raw_mode(HANDLE hSerial)
{
    DCB dcbSerialParams       = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        perror("Error getting serial port state");
        LUOS_ASSERT(0);
        return;
    }

    dcbSerialParams.BaudRate     = SERIAL_NETWORK_BAUDRATE;
    dcbSerialParams.ByteSize     = 8;
    dcbSerialParams.StopBits     = ONESTOPBIT;
    dcbSerialParams.Parity       = NOPARITY;
    dcbSerialParams.fBinary      = TRUE;
    dcbSerialParams.fRtsControl  = RTS_CONTROL_DISABLE;
    dcbSerialParams.fOutxCtsFlow = FALSE;
    dcbSerialParams.fDtrControl  = DTR_CONTROL_DISABLE;
    dcbSerialParams.fOutxDsrFlow = FALSE;
    dcbSerialParams.fOutX        = FALSE;
    dcbSerialParams.fInX         = FALSE;
    // dcbSerialParams.XonChar = 17; // ASCII XON (Ctrl-Q)
    // dcbSerialParams.XoffChar = 19; // ASCII XOFF (Ctrl-S)

    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        perror("Error setting serial port state");
        LUOS_ASSERT(0);
        return;
    }

    COMMTIMEOUTS timeouts                = {0};
    timeouts.ReadIntervalTimeout         = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant    = 0;
    timeouts.ReadTotalTimeoutMultiplier  = 0;
    timeouts.WriteTotalTimeoutConstant   = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        perror("Error setting serial port timeouts");
        LUOS_ASSERT(0);
        return;
    }

    // Setup a 4k buffer
    if (!SetupComm(hSerial, 4096, 4096))
    {
        perror("Error setting serial port buffer size");
        return;
    }
}
#else
void set_termios_raw_mode(struct termios *tty)
{
    tty->c_cflag |= (CLOCAL | CREAD);
    tty->c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | IEXTEN);
    #ifdef ECHOCTL
    tty->c_lflag &= ~ECHOCTL; // netbsd workaround for Erk
    #endif
    #ifdef ECHOKE
    tty->c_lflag &= ~ECHOKE;  // netbsd workaround for Erk
    #endif
    tty->c_oflag &= ~(OPOST | ONLCR | OCRNL);
    tty->c_iflag &= ~(INLCR | IGNCR | ICRNL | IGNBRK);
    #ifdef IUCLC
    tty->c_iflag &= ~IUCLC;
    #endif
    #ifdef PARMRK
    tty->c_iflag &= ~PARMRK;
    #endif

    tty->c_cflag |= CS8;
    tty->c_cflag &= ~(CSTOPB);
    tty->c_cflag &= ~(PARENB | PARODD);

    #ifdef CMSPAR
    tty->c_cflag &= ~CMSPAR;
    #endif

    #ifdef IXANY
    tty->c_iflag &= ~(IXON | IXOFF | IXANY);
    #else
    tty->c_iflag &= ~(IXON | IXOFF);
    #endif

    #ifdef CRTSCTS
    tty->c_cflag &= ~(CRTSCTS);
    #elif defined(CNEW_RTSCTS)
    tty->c_cflag &= ~(CNEW_RTSCTS);
    #endif

    tty->c_cc[VMIN]  = 0;
    tty->c_cc[VTIME] = 0;
}
#endif

/******************************************************************************
 * @brief Initialisation of the Serial communication
 * @param None
 * @return None
 * ****************************************************************************/
void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size)
{
    rx_buffer_size = buffer_size;
    // Init the serial peripheral

#ifndef SERIAL_PORT
    #ifdef _WIN32
    printf("Please enter the serial port name (ex: COM1): ");
    #else
    printf("Please enter the serial port name (ex: /dev/ttyUSB0): ");
    #endif
    fgets(portname, sizeof(portname), stdin);
#else
    printf("Using default serial port: %s\n", SERIAL_PORT);
    memcpy(portname, SERIAL_PORT, strlen(SERIAL_PORT));
#endif
    stripStr(portname);
    printf("Connection on serial port %s at %d baud\n", portname, SERIAL_NETWORK_BAUDRATE);

// Open the serial port
#ifdef _WIN32
    HANDLE hSerial;
    hSerial = CreateFile(
        portname,
        GENERIC_READ | GENERIC_WRITE,
        0,    // exclusive access
        NULL, // no security
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        0);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("Error opening serial port\n");
        LUOS_ASSERT(0);
    }
    // Set serial port parameters
    set_serial_raw_mode(hSerial);

    // DCB dcbSerialParams       = {0};
    // dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    // if (!GetCommState(hSerial, &dcbSerialParams))
    // {
    //     printf("Error getting serial port state\n");
    //     CloseHandle(hSerial);
    //     LUOS_ASSERT(0);
    // }
    // dcbSerialParams.BaudRate = SERIAL_NETWORK_BAUDRATE;
    // dcbSerialParams.ByteSize = 8;
    // dcbSerialParams.StopBits = ONESTOPBIT;
    // dcbSerialParams.Parity   = NOPARITY;
    // if (!SetCommState(hSerial, &dcbSerialParams))
    // {
    //     printf("Error setting serial port state\n");
    //     CloseHandle(hSerial);
    //     LUOS_ASSERT(0);
    // }
#else
    serial_port = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_port < 0)
    {
        printf("Error opening serial port\n");
        printf("Error code: %d\n", errno);
        LUOS_ASSERT(0);
        while (1)
            ;
    }
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error getting serial port attributes\n");
        printf("Error code: %d\n", errno);
        close(serial_port);
        LUOS_ASSERT(0);
    }

    // Set raw mode without any special handling of input/output
    // cfmakeraw(&tty);
    set_termios_raw_mode(&tty);

    // Set output/ input to be non-blocking
    // tty.c_cc[VMIN]  = 0;
    // tty.c_cc[VTIME] = 0;

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error setting serial port attributes\n");
        printf("Error code: %d\n", errno);
        close(serial_port);
        LUOS_ASSERT(0);
    }
    tcflush(serial_port, TCIOFLUSH);

    // Bypass baudrate speed limitations of termios by using ioctl
    #define IOSSIOSPEED 0x80045402
    speed_t speed = SERIAL_NETWORK_BAUDRATE;
    if (ioctl(serial_port, IOSSIOSPEED, &speed) < 0)
    {
        printf("Error setting baudrate attributes\n");
        printf("Error code: %d\n", errno);
        close(serial_port);
        LUOS_ASSERT(0);
    }
#endif
    sleep(2);
}

/******************************************************************************
 * @brief Loop of the Serial communication
 * @param None
 * @return None
 ******************************************************************************/
void SerialHAL_Loop(void)
{
    char recvData[1000];
#ifdef _WIN32
    DWORD bytesRead;
    if (!ReadFile(hSerial, recvData, sizeof(recvData), &bytesRead, NULL))
    {
        printf("Error reading from serial port\n");
        CloseHandle(hSerial);
        LUOS_ASSERT(0);
    }
#else
    ssize_t bytesRead;
    bytesRead = read(serial_port, recvData, sizeof(recvData));
#endif
    if (bytesRead > 0)
    {
        Serial_ReceptionWrite((uint8_t *)recvData, (uint32_t)bytesRead);
    }
}

/******************************************************************************
 * @brief Initialisation of the Serial communication
 * @param data pointer of the data to send
 * @param size size of the data to send
 * @return None
 ******************************************************************************/
void SerialHAL_Send(uint8_t *data, uint16_t size)
{
#ifdef _WIN32
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data, size, &bytesWritten, NULL))
    {
        printf("Error writing to serial port\n");
        CloseHandle(hSerial);
        LUOS_ASSERT(0);
    }
#else

    // Check if the output buffer is full
    int bytes_in_buffer;
    ssize_t bytesWritten;
    ioctl(serial_port, TIOCOUTQ, &bytes_in_buffer);
    printf("%d bytes present before adding %d other.\n", bytes_in_buffer, size);
    bytesWritten = write(serial_port, data, size);
    LUOS_ASSERT(bytesWritten == size);
    // tcdrain(serial_port);
    if (bytesWritten < 0)
    {
        printf("Error writing to serial port\n");
        close(serial_port);
        LUOS_ASSERT(0);
    }
#endif
    Serial_TransmissionEnd();
}

uint8_t SerialHAL_GetPort(void)
{
    // We may return the COM port number here
    // For now let's just consider that the port is 0
    return 0;
}
