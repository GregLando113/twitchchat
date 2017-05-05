#ifdef TWITCH_API_H
#error "File included multiple times in object."
#endif
#define TWITCH_API_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>
    
    
    typedef void msg_recv_fn_t(char* message, unsigned len);
    
    typedef struct twitch_viewer
    {
        char name[0x20];
        struct {
            uint8_t isModerator : 1;
            uint8_t isFollower : 1;
            uint8_t isSubscribed : 1;
        };
    } twitch_viewer;
    
    typedef struct twitch_channel
    {
        char name[0x20];
        twitch_viewer* viewers;
        uint32_t viewercount;
    } twitch_channel;
    
    
    typedef struct twitch_conn
    {
        char name[0x20];
        twitch_channel curchannel;
        
        msg_recv_fn_t* msgrecvfn;
        
        struct {
            uint8_t isConnected : 1;
        };
        
        int sock;
        // WARNING: Static buffer potentially unsafe.
        char outbuffer[0x220];
        char inbuffer[0x220];
    } twitch_conn;
    
    
    int twitch_connect(twitch_conn* conn,char* name,char* oauthpass);
    int twitch_disconnect(twitch_conn* conn);
    
    int twitch_joinchannel(twitch_conn* conn, char* channel);
    int twitch_leavechannel(twitch_conn* conn, char* channel);
    
    int twitch_setmsgrecvfn(twitch_conn* conn,msg_recv_fn_t* fn );
    
    int twitch_sendmsg(twitch_conn* conn, char* msg);
    int twitch_sendmsgf(twitch_conn* conn,const char* format, ...);
    
    int twitch_mainroutine(twitch_conn* conn);
    
#ifdef __cplusplus
}
    #endif