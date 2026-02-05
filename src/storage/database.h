#ifndef DATABASE_H
#define DATABASE_H

#include "../core/constants.h"

// Database file format version
#define DB_VERSION 1
#define DB_MAGIC 0x58313830  // "X180" in hex

// File header structure (32 bytes)
typedef struct {
    unsigned int magic;         // DB_MAGIC for validation
    unsigned int version;       // DB_VERSION
    unsigned int show_id;       // Unique show identifier
    char show_name[32];         // Show name
    char mixer_ip[16];          // Mixer IP address
    unsigned int num_steps;     // Number of steps in this show
    unsigned int created_time;  // Creation timestamp
    unsigned int modified_time; // Last modification time
} DbHeader;

// Step entry in database (variable size, but fixed header)
typedef struct {
    unsigned int step_id;
    unsigned int step_number;
    char step_name[64];
    unsigned int created_time;
    unsigned int modified_time;
    // Followed by 16 ChannelState structures (not inline due to size)
} DbStepHeader;

// Serialized channel state (32 bytes each, 16 per step = 512 bytes)
typedef struct {
    int channel_id;
    float fader_level;
    int mute;
    float eq_low;
    float eq_mid;
    float eq_high;
    char channel_name[32];
} DbChannelState;

// In-memory database representation
typedef struct {
    DbHeader header;
    int num_steps;
    int max_steps;
    DbStepHeader *steps;        // Array of step headers
    DbChannelState **channels;  // Array of channel arrays (16 per step)
} Database;

// Database functions
Database* db_create_new(const char *show_name, const char *mixer_ip);
Database* db_load_from_file(const char *filepath);
int db_save_to_file(Database *db, const char *filepath);
void db_free(Database *db);

// Step management
int db_add_step(Database *db, const char *step_name);
int db_update_step(Database *db, int step_id, const char *step_name);
int db_delete_step(Database *db, int step_id);
DbStepHeader* db_get_step(Database *db, int step_id);

// Channel management
int db_set_channel_state(Database *db, int step_id, int channel_id, DbChannelState *state);
DbChannelState* db_get_channel_state(Database *db, int step_id, int channel_id);

// Utility
int db_get_step_index(Database *db, int step_id);
unsigned int db_get_timestamp();

#endif // DATABASE_H
