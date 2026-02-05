#include "osc_client.h"

// Error codes
#define OSC_ERR_OK 0
#define OSC_ERR_NOT_CONNECTED 1
#define OSC_ERR_INVALID_ADDRESS 2
#define OSC_ERR_BUFFER_OVERFLOW 3
#define OSC_ERR_SEND_FAILED 4
#define OSC_ERR_INVALID_ARGS 5

// Forward declarations
static int osc_strlen(const char *str);
static void osc_strcpy(char *dst, const char *src);
static int osc_strcmp(const char *a, const char *b);

// String utilities
static int osc_strlen(const char *str) {
    int len = 0;
    while (str && *str) {
        len++;
        str++;
    }
    return len;
}

static void osc_strcpy(char *dst, const char *src) {
    while (src && *src) {
        *dst++ = *src++;
    }
    *dst = '\0';
}

static int osc_strcmp(const char *a, const char *b) {
    while (a && b && *a && *b) {
        if (*a != *b) return 1;
        a++;
        b++;
    }
    if ((!a || !*a) && (!b || !*b)) return 0;
    return 1;
}

// Pad to 4-byte boundary for OSC
static int osc_pad_length(int length) {
    return ((length + 3) / 4) * 4;
}

// Initialize OSC client
int osc_client_init(OSCClient *client, const char *ip, int port) {
    if (!client || !ip) {
        return OSC_ERR_INVALID_ARGS;
    }
    
    // Copy IP address
    int ip_len = osc_strlen(ip);
    if (ip_len >= 16) {
        return OSC_ERR_INVALID_ARGS;
    }
    osc_strcpy(client->mixer_ip, ip);
    
    client->mixer_port = port;
    client->connected = 0;
    client->last_error = OSC_ERR_OK;
    
    return OSC_ERR_OK;
}

// Connect to mixer (placeholder - actual networking would use 3DS socket API)
int osc_client_connect(OSCClient *client) {
    if (!client) {
        return OSC_ERR_INVALID_ARGS;
    }
    
    // In a real implementation, this would:
    // 1. Resolve the IP address
    // 2. Create a UDP socket
    // 3. Connect to the mixer at mixer_ip:mixer_port
    
    // For now, just mark as connected (actual networking would go here)
    client->connected = 1;
    client->last_error = OSC_ERR_OK;
    
    return OSC_ERR_OK;
}

// Disconnect from mixer
int osc_client_disconnect(OSCClient *client) {
    if (!client) {
        return OSC_ERR_INVALID_ARGS;
    }
    
    // Would close the UDP socket here
    client->connected = 0;
    client->last_error = OSC_ERR_OK;
    
    return OSC_ERR_OK;
}

// Initialize an OSC message
void osc_message_init(OSCMessage *msg, const char *address) {
    if (!msg || !address) return;
    
    // Copy address pattern
    int addr_len = osc_strlen(address);
    if (addr_len >= 64) addr_len = 63;
    
    int i;
    for (i = 0; i < addr_len; i++) {
        msg->address[i] = address[i];
    }
    msg->address[i] = '\0';
    
    msg->type = OSC_TYPE_INT;
    msg->int_value = 0;
    msg->float_value = 0;
    msg->string_value[0] = '\0';
}

// Set integer value
void osc_message_set_int(OSCMessage *msg, int value) {
    if (!msg) return;
    msg->type = OSC_TYPE_INT;
    msg->int_value = value;
}

// Set float value (stored as 0-100 integer internally)
void osc_message_set_float(OSCMessage *msg, int float_value) {
    if (!msg) return;
    msg->type = OSC_TYPE_FLOAT;
    msg->float_value = float_value;
}

// Set string value
void osc_message_set_string(OSCMessage *msg, const char *value) {
    if (!msg || !value) return;
    msg->type = OSC_TYPE_STRING;
    
    int str_len = osc_strlen(value);
    if (str_len >= 64) str_len = 63;
    
    int i;
    for (i = 0; i < str_len; i++) {
        msg->string_value[i] = value[i];
    }
    msg->string_value[i] = '\0';
}

// Serialize message to binary OSC format
// Returns number of bytes written, or negative on error
int osc_serialize_message(OSCMessage *msg, unsigned char *buffer, int buffer_size) {
    if (!msg || !buffer || buffer_size < 32) {
        return OSC_ERR_BUFFER_OVERFLOW;
    }
    
    int offset = 0;
    
    // 1. Address pattern (null-terminated string, padded to 4-byte boundary)
    int addr_len = osc_strlen(msg->address);
    int addr_padded = osc_pad_length(addr_len + 1);
    
    if (offset + addr_padded > buffer_size) {
        return OSC_ERR_BUFFER_OVERFLOW;
    }
    
    // Copy address
    int i;
    for (i = 0; i < addr_len; i++) {
        buffer[offset++] = (unsigned char)msg->address[i];
    }
    buffer[offset++] = '\0';
    
    // Pad with zeros
    while (offset % 4 != 0) {
        buffer[offset++] = '\0';
    }
    
    // 2. Type tag string (starts with ',')
    // For simplicity, only support single-value messages
    int type_start = offset;
    buffer[offset++] = ',';
    buffer[offset++] = (unsigned char)msg->type;
    buffer[offset++] = '\0';
    
    // Pad type tag
    while (offset % 4 != 0) {
        buffer[offset++] = '\0';
    }
    
    // 3. Arguments (only one for now)
    if (msg->type == OSC_TYPE_INT) {
        if (offset + 4 > buffer_size) {
            return OSC_ERR_BUFFER_OVERFLOW;
        }
        // Big-endian 32-bit integer
        int value = msg->int_value;
        buffer[offset++] = (unsigned char)((value >> 24) & 0xFF);
        buffer[offset++] = (unsigned char)((value >> 16) & 0xFF);
        buffer[offset++] = (unsigned char)((value >> 8) & 0xFF);
        buffer[offset++] = (unsigned char)(value & 0xFF);
        
    } else if (msg->type == OSC_TYPE_FLOAT) {
        if (offset + 4 > buffer_size) {
            return OSC_ERR_BUFFER_OVERFLOW;
        }
        // Convert 0-100 integer to 0.0-1.0 float in OSC format
        // OSC uses IEEE 754 single-precision float (big-endian)
        // For simplicity, we'll store as fixed-point (0-100) and let mixer handle conversion
        // In real implementation, would convert to proper IEEE 754
        
        int int_val = msg->float_value;
        // Simple conversion: treat as 0-100 integer, convert to "float"
        // This is a simplified approach - proper OSC would use IEEE 754
        
        // For now, send as big-endian integer representation of 0.0-1.0
        // Assuming float_value is 0-100, convert to 0x00000000 to 0x3F800000
        unsigned int float_bits;
        if (int_val <= 0) {
            float_bits = 0x00000000;  // 0.0
        } else if (int_val >= 100) {
            float_bits = 0x3F800000;  // 1.0
        } else {
            // Linear interpolation (simplified)
            float_bits = (int_val * 0x3F800000) / 100;
        }
        
        buffer[offset++] = (unsigned char)((float_bits >> 24) & 0xFF);
        buffer[offset++] = (unsigned char)((float_bits >> 16) & 0xFF);
        buffer[offset++] = (unsigned char)((float_bits >> 8) & 0xFF);
        buffer[offset++] = (unsigned char)(float_bits & 0xFF);
        
    } else if (msg->type == OSC_TYPE_STRING) {
        int str_len = osc_strlen(msg->string_value);
        int str_padded = osc_pad_length(str_len + 1);
        
        if (offset + str_padded > buffer_size) {
            return OSC_ERR_BUFFER_OVERFLOW;
        }
        
        for (i = 0; i < str_len; i++) {
            buffer[offset++] = (unsigned char)msg->string_value[i];
        }
        buffer[offset++] = '\0';
        
        while (offset % 4 != 0) {
            buffer[offset++] = '\0';
        }
    }
    
    return offset;  // Return number of bytes written
}

// Send fader level message
int osc_send_fader_level(OSCClient *client, int channel, int level) {
    if (!client) {
        return OSC_ERR_INVALID_ARGS;
    }
    
    if (channel < 0 || channel >= 16) {
        client->last_error = OSC_ERR_INVALID_ARGS;
        return OSC_ERR_INVALID_ARGS;
    }
    
    if (level < 0 || level > 100) {
        level = (level < 0) ? 0 : 100;
    }
    
    if (!client->connected) {
        client->last_error = OSC_ERR_NOT_CONNECTED;
        return OSC_ERR_NOT_CONNECTED;
    }
    
    // Build address: /fader/ch/X where X is channel number (1-16, not 0-15)
    char address[32];
    address[0] = '/';
    address[1] = 'f';
    address[2] = 'a';
    address[3] = 'd';
    address[4] = 'e';
    address[5] = 'r';
    address[6] = '/';
    address[7] = 'c';
    address[8] = 'h';
    address[9] = '/';
    
    int ch_num = channel + 1;
    int addr_idx = 10;
    
    if (ch_num >= 10) {
        address[addr_idx++] = '0' + (ch_num / 10);
    }
    address[addr_idx++] = '0' + (ch_num % 10);
    address[addr_idx] = '\0';
    
    // Create and send message
    OSCMessage msg;
    osc_message_init(&msg, address);
    osc_message_set_float(&msg, level);
    
    // Serialize message
    unsigned char buffer[OSC_MAX_MESSAGE_SIZE];
    int msg_size = osc_serialize_message(&msg, buffer, OSC_MAX_MESSAGE_SIZE);
    
    if (msg_size < 0) {
        client->last_error = msg_size;
        return msg_size;
    }
    
    // Send via UDP (would call socket API here)
    // For now, just mark as successful
    client->last_error = OSC_ERR_OK;
    return OSC_ERR_OK;
}

// Send mute message
int osc_send_mute(OSCClient *client, int channel, int mute) {
    if (!client) {
        return OSC_ERR_INVALID_ARGS;
    }
    
    if (channel < 0 || channel >= 16) {
        client->last_error = OSC_ERR_INVALID_ARGS;
        return OSC_ERR_INVALID_ARGS;
    }
    
    if (!client->connected) {
        client->last_error = OSC_ERR_NOT_CONNECTED;
        return OSC_ERR_NOT_CONNECTED;
    }
    
    // Build address: /mute/ch/X
    char address[32];
    address[0] = '/';
    address[1] = 'm';
    address[2] = 'u';
    address[3] = 't';
    address[4] = 'e';
    address[5] = '/';
    address[6] = 'c';
    address[7] = 'h';
    address[8] = '/';
    
    int ch_num = channel + 1;
    int addr_idx = 9;
    
    if (ch_num >= 10) {
        address[addr_idx++] = '0' + (ch_num / 10);
    }
    address[addr_idx++] = '0' + (ch_num % 10);
    address[addr_idx] = '\0';
    
    // Create and send message
    OSCMessage msg;
    osc_message_init(&msg, address);
    osc_message_set_int(&msg, mute ? 1 : 0);
    
    // Serialize and send
    unsigned char buffer[OSC_MAX_MESSAGE_SIZE];
    int msg_size = osc_serialize_message(&msg, buffer, OSC_MAX_MESSAGE_SIZE);
    
    if (msg_size < 0) {
        client->last_error = msg_size;
        return msg_size;
    }
    
    client->last_error = OSC_ERR_OK;
    return OSC_ERR_OK;
}

// Send EQ message
int osc_send_eq(OSCClient *client, int channel, int band, int value) {
    if (!client) {
        return OSC_ERR_INVALID_ARGS;
    }
    
    if (channel < 0 || channel >= 16 || band < 0 || band > 2) {
        client->last_error = OSC_ERR_INVALID_ARGS;
        return OSC_ERR_INVALID_ARGS;
    }
    
    if (!client->connected) {
        client->last_error = OSC_ERR_NOT_CONNECTED;
        return OSC_ERR_NOT_CONNECTED;
    }
    
    // Build address: /eq/ch/X/band/Y where X=channel, Y=band (0=low, 1=mid, 2=high)
    char address[48];
    int idx = 0;
    
    // /eq/ch/
    address[idx++] = '/';
    address[idx++] = 'e';
    address[idx++] = 'q';
    address[idx++] = '/';
    address[idx++] = 'c';
    address[idx++] = 'h';
    address[idx++] = '/';
    
    // Channel number
    int ch_num = channel + 1;
    if (ch_num >= 10) {
        address[idx++] = '0' + (ch_num / 10);
    }
    address[idx++] = '0' + (ch_num % 10);
    
    // /band/
    address[idx++] = '/';
    address[idx++] = 'b';
    address[idx++] = 'a';
    address[idx++] = 'n';
    address[idx++] = 'd';
    address[idx++] = '/';
    
    // Band number (0=low, 1=mid, 2=high)
    address[idx++] = '0' + band;
    address[idx] = '\0';
    
    // Create and send message
    OSCMessage msg;
    osc_message_init(&msg, address);
    osc_message_set_int(&msg, value);  // -20 to +20 dB
    
    // Serialize and send
    unsigned char buffer[OSC_MAX_MESSAGE_SIZE];
    int msg_size = osc_serialize_message(&msg, buffer, OSC_MAX_MESSAGE_SIZE);
    
    if (msg_size < 0) {
        client->last_error = msg_size;
        return msg_size;
    }
    
    client->last_error = OSC_ERR_OK;
    return OSC_ERR_OK;
}

// Send custom message
int osc_send_custom(OSCClient *client, const char *address, int value) {
    if (!client || !address) {
        return OSC_ERR_INVALID_ARGS;
    }
    
    if (!client->connected) {
        client->last_error = OSC_ERR_NOT_CONNECTED;
        return OSC_ERR_NOT_CONNECTED;
    }
    
    // Create and send message
    OSCMessage msg;
    osc_message_init(&msg, address);
    osc_message_set_int(&msg, value);
    
    // Serialize and send
    unsigned char buffer[OSC_MAX_MESSAGE_SIZE];
    int msg_size = osc_serialize_message(&msg, buffer, OSC_MAX_MESSAGE_SIZE);
    
    if (msg_size < 0) {
        client->last_error = msg_size;
        return msg_size;
    }
    
    client->last_error = OSC_ERR_OK;
    return OSC_ERR_OK;
}

// Check if connected
int osc_is_connected(OSCClient *client) {
    if (!client) return 0;
    return client->connected;
}

// Get error string
const char *osc_get_error_string(int error_code) {
    switch (error_code) {
        case OSC_ERR_OK:
            return "OK";
        case OSC_ERR_NOT_CONNECTED:
            return "Not connected";
        case OSC_ERR_INVALID_ADDRESS:
            return "Invalid address";
        case OSC_ERR_BUFFER_OVERFLOW:
            return "Buffer overflow";
        case OSC_ERR_SEND_FAILED:
            return "Send failed";
        case OSC_ERR_INVALID_ARGS:
            return "Invalid arguments";
        default:
            return "Unknown error";
    }
}
