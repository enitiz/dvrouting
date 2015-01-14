/********************************************************************************
*   FILE:   commands.c
*   DESC:   Functions used in main() are defined here
********************************************************************************/
#include "header.h"

int update_index = 0;
int num_packets = 0;

/********************************************************************************
*   Name:   make_incoming_socket
*   Desc:   Takes a port value and creates and binds a socket which can be used
*           for listening incoming UDP connections later
*   Ret:    bound socket
*   Ref:    None
********************************************************************************/
int new_sockin(uint16_t port) {

    int sockfd, rv;
    struct sockaddr_in neighbor_router;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(0 == sockfd) {
        perror("socket");
    }
    memset(&neighbor_router, 0, sizeof(neighbor_router));
    neighbor_router.sin_family = AF_INET;
    neighbor_router.sin_addr.s_addr = htonl(INADDR_ANY);
    neighbor_router.sin_port = port;

    rv = bind(sockfd, (struct sockaddr *) &neighbor_router, sizeof(neighbor_router));
    if(-1 == rv) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

/********************************************************************************
*   Name:   get_args
*   Desc:   Takes command line arguments. Checks for path of topology file,
            router update interval
*   Ret:    Success or Failure
*   Ref:    http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
********************************************************************************/
int get_args(char **topologypath, long int *upintvl, int argc, char** argv)
{
    /***************************************
    * Declarations
    ***************************************/
    char ch;
    char *temp;
    long int updateinterval;
    int has_topologypath = FALSE;
    int has_updateinterval = FALSE;

    /***************************************
    * If there are no arguments then return
    ***************************************/
    if(argc <= 0) {
        fprintf(stdout, "No arguments found!");
        return FAILURE;
    }

    /***************************************
    * Check for -t and -i and their values
    ***************************************/
    while ((ch = (char) getopt(argc, argv, "t:i:")) != -1) {

        switch (ch) {

            case 't':
                temp = optarg;
                fprintf(stdout, "Topology file path: %s\n", optarg);
                has_topologypath = TRUE;
                break;

            case 'i':
                updateinterval = strtol(optarg, NULL, 10);
                if(updateinterval <= 0) {
                    fprintf(stdout, "Invalid update interval!\n");
                }
                else {
                    fprintf(stdout, "Update interval: %ld\n", updateinterval);
                    has_updateinterval = TRUE;
                }
                break;

            case '?':
                if(optopt == 't') {
                  fprintf(stdout, "Option -%c requires an argument.\n", optopt);
                }
                else if(optopt == 'i') {
                  fprintf(stdout, "Option -%c requires an argument.\n", optopt);
                }

            default:
                return FALSE;
      }
    }

    /***************************************
    * If something is missing, let user know and return failure
    ***************************************/
    if(has_topologypath == FALSE) {
        fprintf(stdout, "Path to topology file not found.\n");
    }
    if(has_updateinterval == FALSE) {
        fprintf(stdout, "Routing update interval not found.\n");
    }
    if(has_topologypath == FALSE || has_updateinterval == FALSE) {
        return FAILURE;
    }

    /***************************************
    * If not, pass references back and return success
    ***************************************/
    // Get topologypath from temp
    *topologypath = (char*) malloc(sizeof(temp));
    if(NULL == *topologypath) {
        fprintf(stdout, "memory allocation failed.\n");
        return FAILURE;
    }
    strncpy(*topologypath, temp, strlen(temp));

    // Get upintvl
    *upintvl = updateinterval;

    return SUCCESS;
}

/********************************************************************************
*   Name:   open_file
*   Desc:   Wrapper around fopen in read mode.
*   Ret:    File pointer of opened file or NULL
*   Ref:    None
********************************************************************************/
FILE *open_file(char *path)
{
    FILE *topology_file = NULL;

    topology_file = fopen(path, "r");
    if(NULL == topology_file) {
        perror("fopen");
    }
    else {
        fprintf(stdout, "File %s opened successfully!\n", path);
    }

    return topology_file;
}

/********************************************************************************
*   Name:   close_file
*   Desc:   Wrapper around fclose.
*   Ret:    File pointer of opened file or NULL
*   Ref:    None
********************************************************************************/
int close_file(FILE *openfile)
{
    int rv = EOF;

    rv = fclose(openfile);
    if(0 == rv) {
        fprintf(stdout, "File closed successfully!\n");
        return SUCCESS;
    }
    else {
        return FAILURE;
    }
}

/********************************************************************************
*   Name:   add_routing_table_entry
*   Desc:   Enters given information to routing table.
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void add_routing_table_entry(uint16_t id, uint32_t ip_addr, uint16_t port, uint16_t cost, uint16_t nexthop, int counter) 
{
    int index_base=0, index_target=0;
    struct updates temp;
    struct info temp2;

   // Add entry to update/routing structure
    this_router.routing_table.entry[update_index].ip_addr = ip_addr;
    this_router.routing_table.entry[update_index].port = port;
    this_router.routing_table.entry[update_index].id = id;
    this_router.routing_table.entry[update_index].cost = cost;

    if(this_router.routing_table.entry[update_index].cost == INF) {
        this_router.routing_table.additional_info[update_index].nexthop = -1;    
    }
    else {
        this_router.routing_table.additional_info[update_index].nexthop = nexthop;
    }

    this_router.routing_table.additional_info[update_index].counter = counter;

    // Keep track
    update_index++;

    // Bubble Sort
    for(index_base = 0; index_base < update_index; index_base++) {
        for(index_target = index_base+1; index_target < update_index; index_target++) {

            if(this_router.routing_table.entry[index_target].id < this_router.routing_table.entry[index_base].id) {
                temp = this_router.routing_table.entry[index_target];
                this_router.routing_table.entry[index_target] = this_router.routing_table.entry[index_base];
                this_router.routing_table.entry[index_base] = temp;

                temp2 = this_router.routing_table.additional_info[index_target];
                this_router.routing_table.additional_info[index_target] = this_router.routing_table.additional_info[index_base];
                this_router.routing_table.additional_info[index_base] = temp2;
            }
        }
    }
}

/********************************************************************************
*   Name:   find_entry_by_id
*   Desc:   Finds an entry by id
*   Ret:    index
*   Ref:    None
********************************************************************************/
int find_entry_by_id(uint16_t id) 
{
    int index;

    for(index = 0; index < update_index; index++) 
    {
        if(this_router.routing_table.entry[index].id == id) {    
            return index;
        }
    }

    return FAILURE;
}


/********************************************************************************
*   Name:   find_entry_by_ip
*   Desc:   finds entry by ip
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
int find_entry_by_ip(uint32_t ip) 
{
    int index;

    for(index = 0; index < update_index; index++) 
    {
        if(this_router.routing_table.entry[index].ip_addr == ip) {    
            return index;
        }
    }

    return FAILURE;
}


/********************************************************************************
*   Name:   update_link_routing_table_entry
*   Desc:   Enters given information to routing table.
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void update_link_routing_table_entry(uint16_t id, int nexthop, uint16_t cost) {
    
    int index = 0;

    index = find_entry_by_id(id);
    if(index == FAILURE)
    {
        //printf("No entry found to update!\n");
        return;
    }
    this_router.routing_table.additional_info[index].counter = 0;
    this_router.routing_table.additional_info[index].nexthop = nexthop;
    this_router.routing_table.entry[index].cost = cost;
}

/********************************************************************************
*   Name:   read_topology
*   Desc:   Reads topology file and adds entries to routing table
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void read_topology(FILE *tofile) {

    int index;
    int num_routers;
    int num_neighbors;
    int num_neighbors2;
    int router_ip1;
    int router_ip2;
    int router_ip3;
    int router_ip4;
    char router_ip[14];
    
    uint16_t router_id;
    uint16_t router_id1;
    uint16_t neighbor_id1;
    uint16_t cost1;
    uint16_t router_port;

    fscanf(tofile, "%d\n", &num_routers);
    //printf("%d\n", num_routers);

    fscanf(tofile, "%d\n", &num_neighbors);
    num_neighbors2 = num_neighbors;
    //printf("%d\n", num_neighbors);
   
    // Store information of other routers in routing table
    for(index = 0; index < num_routers; index++) {
        
        fscanf(tofile, "%"SCNu16" %d.%d.%d.%d %"SCNu16"\n", &router_id, &router_ip1, &router_ip2, &router_ip3, &router_ip4, &router_port);
        sprintf(router_ip, "%d.%d.%d.%d", router_ip1, router_ip2, router_ip3, router_ip4);

        //printf("%"PRIu16" %s %"PRIu16"\n", router_id, router_ip, router_port);

        if(inet_addr(router_ip) == this_router.ip_addr) {
            //printf("Self entry found\n");
            this_router.id = router_id;
            this_router.port = router_port;
            add_routing_table_entry(router_id, inet_addr(router_ip), router_port, 0, this_router.id, 0);
        }
        else {
            add_routing_table_entry(router_id, inet_addr(router_ip), router_port, INF, INVALID_ROUTER_ID, COUNTER_DEAD);
        }
    }

    //  Store neighbors in routing table
    for(index = 0; index < num_neighbors2; index++) {
        
        fscanf(tofile, "%"SCNu16" %"SCNu16" %"SCNu16"", &router_id1, &neighbor_id1, &cost1);
        //printf("%"PRIu16" %"PRIu16" %"PRIu16"\n", router_id1, neighbor_id1, cost1);

        update_link_routing_table_entry(neighbor_id1, router_id1, cost1);
    }
}

/********************************************************************************
*   Name:   prepare_message
*   Desc:   Prepares an update message
*   Ret:    update message
*   Ref:    None
********************************************************************************/
char* prepare_message(size_t *msg_size)
{
    char *msg = NULL;
    uint16_t num_updates=0;
    size_t size_count=0;
    int index=0;

    uint16_t port;
    uint32_t ip_addr;
    uint16_t id;
    uint16_t cost;

    /**********************************************************************************
    * Fill header
    ***********************************************************************************/
    msg = (char*) malloc(sizeof(struct update_header) + update_index*sizeof(struct updates));

    num_updates = htons((uint16_t) update_index);
    memcpy(msg, &num_updates, sizeof(num_updates)); 
    size_count += sizeof(num_updates);
    
    port = htons(this_router.port);
    memcpy(msg+size_count, &port, sizeof(port)); 
    size_count += sizeof(port);

    ip_addr = this_router.ip_addr;
    memcpy(msg+size_count, &ip_addr, sizeof(ip_addr));    
    size_count += sizeof(ip_addr);
   
    /**********************************************************************************
    * Fill message
    ***********************************************************************************/
    for(index = 0; index < update_index; index++) {

        memcpy(msg+size_count, &this_router.routing_table.entry[index].ip_addr, sizeof(this_router.routing_table.entry[index].ip_addr));    
        size_count += sizeof(this_router.routing_table.entry[index].ip_addr);

        port = htons(this_router.routing_table.entry[index].port);
        memcpy(msg+size_count, &port, sizeof(port));    
        size_count += sizeof(port);

        size_count += sizeof(this_router.routing_table.entry[index].pad);

        id = htons(this_router.routing_table.entry[index].id);
        memcpy(msg+size_count, &id, sizeof(id));    
        size_count += sizeof(id);

        cost = htons(this_router.routing_table.entry[index].cost);
        memcpy(msg+size_count, &cost, sizeof(cost));    
        size_count += sizeof(cost);
    }

    
    *msg_size = size_count;
    return msg;
}

/********************************************************************************
*   Name:   Increment Counters
*   Desc:   increses counter of each neigbor by 1
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void increment_counters() {
    int this_router_index=0;
    int index=0;

    this_router_index = find_entry_by_id(this_router.id);

    for(index = 0; index < update_index; index++) {

        if(this_router.routing_table.additional_info[index].counter != COUNTER_DEAD) {
            
            if(index == this_router_index) {
                this_router.routing_table.additional_info[index].counter = 0;
            }
            else {
                this_router.routing_table.additional_info[index].counter++;
            }

        }
    }
}


/********************************************************************************
*   Name:   Disable old links
*   Desc:   checks counter of all links and disables those that have more than 3
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void disable_old_links() {
    
    int index=0;

    for(index = 0; index < update_index; index++) {

       // printf("Index %d Counter %d \n", index+1, this_router.routing_table.additional_info[index].counter);

        if(this_router.routing_table.additional_info[index].counter > COUNTER_MAX) {
            
            //this_router.routing_table.entry[index].nexthop = INF;
            //this_router.routing_table.entry[index].cost = INF;
            update_link_routing_table_entry(this_router.routing_table.entry[index].id, -1, INF);
            this_router.routing_table.additional_info[index].counter = COUNTER_DEAD;    
        }
    }
}


/********************************************************************************
*   Name:   send_message_to_neighbors
*   Desc:   sends update message to all neighbors
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void send_message_to_neighbors() {
    int index = 0;
    int rv = 0;

    for(index=0; index < update_index; index++) {

        if(this_router.routing_table.additional_info[index].nexthop == this_router.id && 
                this_router.routing_table.entry[index].id != this_router.id) {
           
            rv = send_message(this_router.routing_table.entry[index].ip_addr,
                                this_router.routing_table.entry[index].port);

            if(-1 == rv) {
                //printf("Failed to send message to router %d\n", this_router.routing_table.entry[index].id);
            }
        }
    }
}

/********************************************************************************
*   Name:   Send Message
*   Desc:   sends update message to the given address
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
int send_message(uint32_t ip_addr, uint16_t port) {

    /***************************************
    * Declarations
    ***************************************/
    int rv;
    int sockfd2;
    struct sockaddr_in neighbor_router2;
    char *message=NULL;
    size_t msg_size;

    sockfd2 = socket(AF_INET, SOCK_DGRAM, 0);
    if(0 == sockfd2) {
        perror("socket");
    }

    memset(&neighbor_router2, 0, sizeof(neighbor_router2));
    neighbor_router2.sin_family = AF_INET;
    neighbor_router2.sin_addr.s_addr = ip_addr;
    neighbor_router2.sin_port = port;    

    message = prepare_message(&msg_size);

    //printf("Sending update message to: %s %d\n", inet_ntoa(neighbor_router2.sin_addr), neighbor_router2.sin_port);
    rv = sendto(sockfd2, message, msg_size, 0, (struct sockaddr*) &neighbor_router2, sizeof(neighbor_router2));

    free(message);
    close(sockfd2);
    return rv;
}

/********************************************************************************
*   Name:   Get message and update
*   Desc:   reads update message and updates routing table
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void get_message_and_update(int sock_in) {

    uint16_t num_updates;
    uint16_t source_port;
    uint32_t source_ip_addr; 
    
    struct updates incoming_message[MAX_ROUTERS];
    
    uint16_t self_id=0;
    uint16_t neighbor_id=0; 

    char msg[1000];

    int index=0;
    int self_index=0;
    int neighbor_index=0;
    int entry_index=0;
    
    int size_count=0;

    socklen_t fromlen;
    struct sockaddr addr;
    struct in_addr ip_addr;

    // Keep msg clear
    memset(&msg, 0, sizeof(msg));
    memset(&incoming_message, 0, sizeof(incoming_message));
    
    //printf("\nGetting message... \n");
    recvfrom(sock_in, &msg, sizeof(msg), 0, &addr, &fromlen);

    memcpy(&num_updates, msg, sizeof(num_updates)); 
    num_updates = ntohs(num_updates);
    size_count += sizeof(num_updates);
    //printf("num updates: %"PRIu16"\n", num_updates);    

    memcpy(&source_port, msg+size_count, sizeof(source_port)); 
    source_port = ntohs(source_port);
    size_count += sizeof(source_port);
    //printf("port: %"PRIu16"\n", source_port);

    memcpy(&source_ip_addr, msg+size_count, sizeof(source_ip_addr));    
    size_count += sizeof(source_ip_addr);
    ip_addr.s_addr = source_ip_addr;
    //printf("ip addr: %s\n\n", inet_ntoa(ip_addr));

    /**********************************************************************************
    * Get message
    ***********************************************************************************/
   //printf("Getting node info...\n");
   for(index = 0; index < num_updates; index++) {

        // Read node information.

        //printf("Entry %d\n", index+1);

        memcpy(&incoming_message[index].ip_addr, msg+size_count, sizeof(incoming_message[index].ip_addr));    
        size_count += sizeof(incoming_message[index].ip_addr);
        ip_addr.s_addr = incoming_message[index].ip_addr;
        //printf("ip addr: %s\n", inet_ntoa(ip_addr));
   
        memcpy(&incoming_message[index].port, msg+size_count, sizeof(incoming_message[index].port)); 
        incoming_message[index].port = ntohs(incoming_message[index].port);
        size_count += sizeof(incoming_message[index].port);
        //printf("port: %"PRIu16"\n", incoming_message[index].port);

        size_count += sizeof(incoming_message[index].pad);

        memcpy(&incoming_message[index].id, msg+size_count, sizeof(incoming_message[index].id)); 
        incoming_message[index].id = ntohs(incoming_message[index].id);
        size_count += sizeof(incoming_message[index].id);
        //printf("id: %"PRIu16"\n", incoming_message[index].id);

        memcpy(&incoming_message[index].cost, msg+size_count, sizeof(incoming_message[index].cost)); 
        incoming_message[index].cost = ntohs(incoming_message[index].cost);
        size_count += sizeof(incoming_message[index].cost);
        //printf("cost: %"PRIu16"\n", incoming_message[index].cost);

        //printf("\n");
    }

    /**********************************************************************************
    * Manage updates
    ***********************************************************************************/
    
    // Is the sender already in routing table?
    for(index = 0; index < num_updates; index++) {
        if(incoming_message[index].ip_addr == source_ip_addr) {   
            // This is the neighbor!
            cse4589_print_and_log("RECEIVED A MESSAGE FROM SERVER %d\n", incoming_message[index].id);
            neighbor_id = incoming_message[index].id;
            neighbor_index = find_entry_by_id(neighbor_id);

            // Update counter to 0
            this_router.routing_table.additional_info[neighbor_index].counter = 0;
        }
        
        if(incoming_message[index].id == this_router.id) {
            // This packet is self!
            self_index = index;
        }
    }

    // Show message on screen and log
    for(index = 0; index < num_updates; index++) {
        cse4589_print_and_log("%­15d%­15d\n", incoming_message[index].id, incoming_message[index].cost);
    }

    // The neighbor can reach me at the cost k, if it is better than what i already know, update!
    if(incoming_message[self_index].cost <= this_router.routing_table.entry[neighbor_index].cost) {
        // Update
        update_link_routing_table_entry(this_router.routing_table.entry[neighbor_index].id, this_router.id, incoming_message[self_index].cost);
    }

    // For other entries
     for(index = 0; index < num_updates; index++) {
        if(incoming_message[index].id != self_id || incoming_message[index].id != this_router.id) {

            // Incoming message has better cost. Update the tuples.
            entry_index = find_entry_by_id(incoming_message[index].id);
            if(this_router.routing_table.entry[entry_index].cost > incoming_message[index].cost + this_router.routing_table.entry[neighbor_index].cost) {

                update_link_routing_table_entry(incoming_message[index].id, neighbor_id, incoming_message[index].cost + this_router.routing_table.entry[neighbor_index].cost);
            }

            // Update counter to 0 regardless
            this_router.routing_table.additional_info[entry_index].counter = 0;
        }
    }

    num_packets++;
}

/********************************************************************************
*   Name:   Get this router ip addr
*   Desc:   gives int value of router's ip address
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
uint32_t get_this_router_ip_addr()
 {
    int sock_desc, return_value = 0;
    struct sockaddr_in google_addr;
    struct sockaddr_in local_addr;
    
    socklen_t local_addr_len;
    memset(&google_addr, 0, sizeof(google_addr));
    memset(&local_addr, 0, sizeof(local_addr));

    google_addr.sin_family = AF_INET;
    google_addr.sin_port = htons(33);
    return_value =  inet_pton(AF_INET, "8.8.4.4", &google_addr.sin_addr);
    if(-1 == return_value)
    {
      perror("get_ip: inet_pton");
      exit(EXIT_FAILURE);
    }

    sock_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == return_value)
    {
      perror("get_ip: socket");
      exit(EXIT_FAILURE);
    }

    return_value = connect(sock_desc, (struct sockaddr *) &google_addr, sizeof(google_addr));
    if(-1 == return_value)
    {
      perror("get_ip: connect");
      exit(EXIT_FAILURE);
    }

    local_addr_len = sizeof(local_addr);
    return_value = getsockname(sock_desc, (struct sockaddr *) &local_addr, &local_addr_len);
    if(-1 == return_value)
    {
      perror("get_ip: getsockname");
      exit(EXIT_FAILURE);
    }

    close(sock_desc);

    return local_addr.sin_addr.s_addr;
}


/********************************************************************************
*   Name:   string_lowcase
*   Desc:   converts all UPPERCaSE character of string to lowecase
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void string_lowcase(char *string)
{
  int i = 0;

  for(i=0; string[i]; i++)
  {
    string[i] = tolower(string[i]);
  }
}

/********************************************************************************
*   Name:   string_lowcase
*   Desc:   converts all UPPERCaSE character of string to lowecase
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void kill_connection(int target_index) 
{
    int index=0;

    for(index=0; index < update_index; index++) {

        if(index == target_index) {

            // Set cost to INF, nexthop to -1, cost to INF and counter to -1
            update_link_routing_table_entry(this_router.routing_table.entry[index].id, -1, INF);
            this_router.routing_table.additional_info[target_index].counter = -1; 
            break;
        }
    }
}