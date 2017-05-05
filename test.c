#include <stdio.h>
#include <Windows.h>
#include "twitch_api.h"

twitch_conn twitch;
char inbuffer[0x100];

void msgrecv(char* msg, unsigned len)
{
    fwrite (msg, 1, len, stdout);
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
    twitch_joinchannel(&twitch,"clintstevens");
    while(twitch.isConnected)
    {
        twitch_mainroutine(&twitch);
        Sleep(1);
    }
    return 0;
}