#ifdef TWITCH_API_H
#error "File included multiple times in object."
#endif
#define TWITCH_API_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>
    
    typedef struct twitch_ircmessage
    {
        char* nick;
        char* user;
        char* host;
        char* command;
        int paramc;
        char* params[16];
        struct {
            uint8_t isResponse : 1;
        };
    } twitch_ircmessage;
    
    typedef void cmd_recv_fn_t(twitch_ircmessage* msg);
    typedef void msg_recv_fn_t(char* user, char* message);
    
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
        cmd_recv_fn_t* cmdrecvfn;
        
        struct {
            uint8_t isConnected : 1;
        };
        
        int sock;
        // WARNING: Static buffer potentially unsafe.
        char outbuffer[0x420];
        char inbuffer[0x420];
    } twitch_conn;
    
    
    int twitch_connect(twitch_conn* conn,char* name,char* oauthpass);
    int twitch_disconnect(twitch_conn* conn);
    
    int twitch_joinchannel(twitch_conn* conn, char* channel);
    int twitch_leavechannel(twitch_conn* conn, char* channel);
    
    int twitch_sendraw(twitch_conn* conn, char* cmd);
    int twitch_sendrawf(twitch_conn* conn, const char* format, ...);
    
    int twitch_sendmsg(twitch_conn* conn, char* msg);
    int twitch_sendmsgf(twitch_conn* conn,const char* format, ...);
    
    int twitch_mainroutine(twitch_conn* conn);
    
#ifdef __cplusplus
}
    #endif