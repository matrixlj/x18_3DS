#ifndef OSC_CLIENT_H
#define OSC_CLIENT_H

#include "../core/state.h"

// OSC networking configuration
#define OSC_MAX_MESSAGE_SIZE 256
#define OSC_DEFAULT_PORT 10023

// OSC message types
typedef enum {
    OSC_TYPE_INT = 'i',
    OSC_TYPE_FLOAT = 'f',
    OSC_TYPE_STRING = 's'
} OSCType;

// OSC message structure
typedef struct {
    char address[64];           // OSC address pattern (e.g., "/fader/ch/1")
    OSCType type;
    int int_value;              // For integer values
    int float_value;             // Float as fixed-point (0-100 scale for faders)
    char string_value[64];       // For string values
} OSCMessage;

// OSC Client state
typedef struct {
    char mixer_ip[16];          // IP address of mixer
    int mixer_port;             // UDP port
    int connected;              // Connection status
    int last_error;             // Last error code
} OSCClient;

// Client lifecycle
int osc_client_init(OSCClient *client, const char *ip, int port);
int osc_client_connect(OSCClient *client);
int osc_client_disconnect(OSCClient *client);

// Message sending functions
int osc_send_fader_level(OSCClient *client, int channel, int level);
int osc_send_mute(OSCClient *client, int channel, int mute);
int osc_send_eq(OSCClient *client, int channel, int band, int value);
int osc_send_custom(OSCClient *client, const char *address, int value);

// Message construction
void osc_message_init(OSCMessage *msg, const char *address);
void osc_message_set_int(OSCMessage *msg, int value);
void osc_message_set_float(OSCMessage *msg, int float_value);
void osc_message_set_string(OSCMessage *msg, const char *value);

// OSC packet serialization (low-level)
int osc_serialize_message(OSCMessage *msg, unsigned char *buffer, int buffer_size);

// Utility
int osc_is_connected(OSCClient *client);
const char *osc_get_error_string(int error_code);

#endif // OSC_CLIENT_H
