#include <stdio.h>
#include <Windows.h>
#include "twitch_api.h"

twitch_conn twitch;
char inbuffer[0x100] = {0};

void msgrecv(char* user, char* msg)
{
    printf("<%s> %s\n", user, msg);
}

DWORD WINAPI threadmain(LPVOID conn)
{
    int result;
    while(twitch.isConnected)
    {
        result = twitch_mainroutine(&twitch);
        if(result < 0)
        {
            printf("ERR: mainroutine = %d", result);
        }
        Sleep(1);
        
    }
    return 0;
}

int main(int argc, char** argv)
{
    int err = 0;
    if(argc < 3)
    {
        puts("Invalid arguments.\n");
        return 1;
    }
    if((err = twitch_connect(&twitch,argv[1],argv[2])) != 0)
    {
        printf("Error occured. err = %d\n", err);
        return 1;
    }
    twitch.msgrecvfn = msgrecv;
    CreateThread(0, 0, threadmain, 0, 0, 0);
    printf("channel? ");
    if(gets_s(inbuffer, 0x100) != NULL)
    {
        twitch_joinchannel(&twitch,inbuffer);
    }
    while(gets_s(inbuffer, 0x100) != NULL)
    {
        twitch_sendmsg(&twitch,inbuffer);
    }
    twitch_disconnect(&twitch);
    return 0;
}