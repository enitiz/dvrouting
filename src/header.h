/********************************************************************************
*   FILE:   headers.c
*   DESC:   Includes, struct / function declarations and preprocessor stuff here
********************************************************************************/
#include <arpa/inet.h>
#include <ctype.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************
* Defines
***************************************/
#define SUCCESS 0
#define FAILURE -1

#define TRUE 0
#define FALSE -1

#define MAX_ROUTERS 30
#define INF 0xFFFF
#define INVALID_ROUTER_ID -1
#define COUNTER_DEAD -1
#define COUNTER_MAX 3

#define CMD_LEN 50

extern int update_index;
extern int num_packets;


/***************************************
* Update Message Structure
***************************************/
struct update_header {
	uint16_t num_updates;               // No of update fields
    uint16_t source_port;               // Server port
    uint32_t source_ip_addr;            // Server IP       
};

struct updates {
    uint32_t ip_addr;                   // ip address of router
    uint16_t port;                      // port number of router
    uint16_t pad;                       // 0x0 padding
    uint16_t id;                        // id of router
    uint16_t cost;                      // cost to reach router
};

/**************************************
* Additional Information structure
**************************************/
struct info {
	int nexthop;                  
	int counter;           	    
};

/**************************************
* Routing table structure
**************************************/
struct rtable {
	struct updates entry[MAX_ROUTERS];  // index corresponds to the dest id
	struct info additional_info[MAX_ROUTERS];
};

/**************************************
* Router structure
**************************************/
struct router {
	uint32_t ip_addr;                   // ip address of this router
	uint16_t port;                      // port of this router
	uint16_t id;                        // id of this 
	struct rtable routing_table;        // routing table for this router
} this_router;


/**************************************
* Externals
**************************************/
void cse4589_print_and_log(char* format, ...);
int cse4589_dump_packet(const void* packet, size_t bytes);


/***************************************
* Function Declarations
***************************************/
int new_sockin(uint16_t port);
int get_args(char **topologypath, long int *upintvl, int argc, char** argv);
FILE *open_file(char *path);
int close_file(FILE *openfile);
void add_routing_table_entry(uint16_t id, uint32_t ip_addr, uint16_t port, uint16_t cost, uint16_t nexthop, int counter);
void update_link_routing_table_entry(uint16_t id, int nexthop, uint16_t cost);
void read_topology(FILE *tofile);
char* prepare_message(size_t *msg_size);
void increment_counters();
void disable_old_links();
void send_message_to_neighbors();
int send_message(uint32_t ip_addr, uint16_t port);
void get_message_and_update(int sock_in);
uint32_t get_this_router_ip_addr();
char *get_command(void);
void string_lowcase(char *string);
void tokenize_command(char *command, char **command_tokens, int *command_token_count);
void kill_connection(int target_index);
int find_entry_by_id(uint16_t id);

/******************************************
* Commands
******************************************/
void update(uint16_t id1, uint16_t id2, uint16_t cost);
void step();
void packets();
void display();
void disable(uint16_t id);
void crash();
void dump();
void academic_integrity();