#include "twitch_api.h"


#ifdef __cplusplus
extern "C" {
#endif
    
    // General libraries
#include <stdio.h>
#include <stdlib.h>
    
    // Socket libraries
#ifdef _WIN32
    
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
    
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
    
#define sendpak(s,buf,len) fwrite (buf, 1, len, stdout); send((SOCKET)s,buf,len,0)
#define recvpak(s,buf,len) recv((SOCKET)s,buf,len,0)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
    
#define sendpak(s,buf,len) fwrite (buf, 1, len, stdout); send(s,buf,len,0)
#define recvpak(s,buf,len) recv(s,buf,len,0)
#endif
    
#define TWITCH_CHAT_API_ROOT "irc.chat.twitch.tv"
#define TWITCH_CHAT_API_PORT "6667"
    
    // Utility functions //
    char* append_msg(char* buf,char* msg)
    {
        for(;*msg;++buf,++msg)
        {
            *buf = *msg;
        }
        return buf;
    }
    char* finalize_msg(char* buf)
    {
        // NOTE: May not need null terminator, check on this
        //buf[0] = '\r';
        //buf[1] = '\n';
        //buf[2] = '\0';
        
        return buf;
    }
    
    // Exposed functions //
    
    int twitch_connect(twitch_conn* conn,char* name,char* oauthpass)
    {
        
        WSADATA d;
        WSAStartup(MAKEWORD(2,2), &d);
        
        
        struct addrinfo *result = NULL;
        struct addrinfo *ptr = NULL;
        struct addrinfo hints = { 0 };
        int iresult;
        
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        printf("connecting...\n");
        iresult = getaddrinfo(TWITCH_CHAT_API_ROOT, TWITCH_CHAT_API_PORT, &hints, &result);
        if(iresult != 0 ) 
        {
            return 1;
        }
        
        for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            conn->sock = (int)socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if(conn->sock == -1) 
            {
                return 2;
            }
            
            iresult = connect(conn->sock, ptr->ai_addr, (int)ptr->ai_addrlen);
            if(iresult < 0)
            {
                close(conn->sock);
                continue;
            }
            break;
        }
        
        freeaddrinfo(result);
        
        if(conn->sock == -1) 
        {
            close(conn->sock);
            return 3;
        }
        printf("connected, sending user/pass\n");
        // Send oauth password
        char* seek = conn->outbuffer;
        seek = append_msg(seek, "PASS ");
        seek = append_msg(seek, oauthpass);
        finalize_msg(seek);
        sendpak(conn->sock,conn->outbuffer,(size_t)(seek - conn->outbuffer));
        
        // Send username
        seek = conn->outbuffer;
        seek = append_msg(seek, "NICK ");
        seek = append_msg(seek, name);
        finalize_msg(seek);
        sendpak(conn->sock,conn->outbuffer,(size_t)(seek - conn->outbuffer));
        
        conn->isConnected = 1;
        return 0;
    }
    int twitch_disconnect(twitch_conn* conn)
    {
        conn->isConnected = 0;
        close(conn->sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }
    
    int twitch_joinchannel(twitch_conn* conn, char* channel)
    {
        printf("joining channel\n");
        // set current channel name
        strcpy(conn->curchannel.name,channel);
        char* seek = conn->outbuffer;
        seek = append_msg(seek, "JOIN #");
        seek = append_msg(seek, channel);
        finalize_msg(seek);
        
        return sendpak(conn->sock,conn->outbuffer,(size_t)(seek - conn->outbuffer));
    }
    
    int twitch_leavechannel(twitch_conn* conn, char* channel)
    {
        printf("leaving channel\n");
        char* seek = conn->outbuffer;
        seek = append_msg(seek, "PART #");
        seek = append_msg(seek, channel);
        finalize_msg(seek);
        
        return sendpak(conn->sock,conn->outbuffer,(size_t)(seek - conn->outbuffer));
    }
    
    int twitch_sendmsg(twitch_conn* conn, char* msg)
    {
        char* seek = conn->outbuffer;
        seek = append_msg(seek, "PRIVMSG #");
        seek = append_msg(seek, conn->curchannel.name);
        seek = append_msg(seek, " :");
        seek = append_msg(seek, msg);
        finalize_msg(seek);
        
        return sendpak(conn->sock,conn->outbuffer,(size_t)(seek - conn->outbuffer));
    }
    int twitch_sendmsgf(twitch_conn* conn,const char* format, ...)
    {
        va_list vl;
        va_start(vl,format);
        char* seek = conn->outbuffer;
        seek = append_msg(seek, "PRIVMSG #");
        seek = append_msg(seek, conn->curchannel.name);
        seek = append_msg(seek, " :");
        seek += vsprintf(seek,format,vl);
        finalize_msg(seek);
        
        return sendpak(conn->sock,conn->outbuffer,(size_t)(seek - conn->outbuffer));
    }
    int twitch_mainroutine(twitch_conn* conn)
    {
        int result;
        result = recvpak(conn->sock,conn->inbuffer,0x220);
        // Nothing to process
        if(result == 0)
        {
            return 0;
        }
        printf("recv: %s result=%d\n",conn->inbuffer,result);
        // Error occured
        if(result < 0)
        {
            conn->isConnected = 0;
            return result;
        }
        printf("recv: %s",conn->inbuffer);
        // If this is a heartbeat request, just reply as is
        if(!strcmp(conn->inbuffer,"PING :tmi.twitch.tv\r\n"))
        {
            printf("\n");
            char* seek = conn->outbuffer;
            seek = append_msg(seek,"PONG :tmi.twitch.tv\r\n");
            sendpak(conn->sock,conn->outbuffer,sizeof("PONG :tmi.twitch.tv\r\n")-1);
            return 0;
        }
        if(conn->msgrecvfn != NULL)
        {
            conn->msgrecvfn(conn->inbuffer, result);
        }
        return 0;
    }
    int twitch_setmsgrecvfn(twitch_conn* conn,msg_recv_fn_t* fn )
    {
        conn->msgrecvfn = fn;
        return 0;
    }
    
#ifdef __cplusplus
}
    #endif