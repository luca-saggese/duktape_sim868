#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>


#include <eat_interface.h>
#include <eat_uart.h>
#include <eat_modem.h>

#include "log.h"


int print(const char* fmt, ...)
{
    char buf[1024] = {0};
    int length = 0;

    va_list arg;
    va_start (arg, fmt);
    vsnprintf(buf, 1024, fmt, arg);
    va_end (arg);

    length = strlen(buf);

    return eat_uart_write(EAT_UART_1,(const unsigned char *)buf, length);
}

void log_hex(const char* data, int length)
{
    int i = 0, j = 0;

    print("    ");
    for (i  = 0; i < 16; i++)
    {
        print("%X  ", i);
    }
    print("    ");
    for (i = 0; i < 16; i++)
    {
        print("%X", i);
    }

    print("\r\n");

    for (i = 0; i < length; i += 16)
    {
        print("%02d  ", i / 16 + 1);
        for (j = i; j < i + 16 && j < length; j++)
        {
            print("%02x ", data[j] & 0xff);
        }
        if (j == length && length % 16)
        {
            for (j = 0; j < (16 - length % 16); j++)
            {
                print("   ");
            }
        }
        print("    ");
        for (j = i; j < i + 16 && j < length; j++)
        {
            if (data[j] < 32 || data[j] >= 127)
            {
                print(".");
            }
            else
            {
                print("%c", data[j] & 0xff);
            }
        }

        print("\r\n");
    }
}