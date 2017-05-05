#include <stdio.h>
#include <Windows.h>
#include "twitch_api.h"

twitch_conn twitch;
char inbuffer[0x100];

void msgrecv(char* msg, unsigned len)
{
    fwrite (msg, 1, len, stdout);
}

DWORD WINAPI mainroutine(LPVOID conn)
{
    while(twitch.isConnected)
    {
        twitch_mainroutine(&twitch);
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
    twitch_setmsgrecvfn(&twitch, msgrecv);
    CreateThread(0, 0, mainroutine, 0, 0, 0);
    twitch_setmsgrecvfn(&twitch, msgrecv);
    Sleep(500);
    twitch_joinchannel(&twitch,"kaos4d1");
    while(fgets (inbuffer, 0x100, stdin) != NULL)
    {
        twitch_sendmsg(&twitch,inbuffer);
    }
    
    return 0;
}