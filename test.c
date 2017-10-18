#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <time.h>
#endif
#include "twitch_api.h"

twitch_conn twitch;
char inbuffer[0x100] = {0};

void msgrecv(char* user, char* msg)
{
    printf("<%s> %s\n", user, msg);
}

#ifdef _WIN32
DWORD WINAPI threadmain(LPVOID conn)
#else
void* threadmain(void* conn)
#endif
{
    int result;
    while(twitch.isConnected)
    {
        result = twitch_mainroutine(&twitch);
        if(result < 0)
        {
            printf("ERR: mainroutine = %d", result);
        }
#ifdef _WIN32
        Sleep(1);
#else
        struct timespec tspec = {0, 1000000};
        nanosleep(&tspec, NULL);
#endif      
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
#ifdef _WIN32
    CreateThread(0, 0, threadmain, 0, 0, 0);
#else
    pthread_t mthread;
    pthread_create(&mthread, NULL, threadmain, NULL);
#endif
    printf("channel? ");
    if(fgets(inbuffer, 0x100, stdin) != NULL)
    {
        twitch_joinchannel(&twitch,inbuffer);
    }
    while(fgets(inbuffer, 0x100, stdin) != NULL)
    {
        twitch_sendmsg(&twitch,inbuffer);
    }
    twitch_disconnect(&twitch);
    return 0;
}