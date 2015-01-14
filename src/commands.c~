#include "header.h"
/********************************************************************************
*   FILE:   commands.c
*   DESC:   Functions for each command are defined here
********************************************************************************/

/********************************************************************************
*   Name:   update
*   Desc:   updates a given link
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void update(uint16_t id1, uint16_t id2, uint16_t cost) {
    cse4589_print_and_log("%s:SUCCESS\n", "update");"
	update_link_routing_table_entry(id2, id1, cost);
}

/********************************************************************************
*   Name:   step
*   Desc:   immediately sends messages to neighbors
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void step() {
    cse4589_print_and_log("%s:SUCCESS\n", "step");
	send_message_to_neighbors();
}

/********************************************************************************
*   Name:   packets
*   Desc:   tells number of packets received since last call
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void packets() {
    cse4589_print_and_log("%s:SUCCESS\n", "packets");
	cse4589_print_and_log("%d\n", num_packets);
	num_packets = 0;
}

/********************************************************************************
*   Name:   display
*   Desc:   shows routing table on screen
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void display() {
	int index;

	cse4589_print_and_log("%s:SUCCESS\n", "display");
	for(index = 0; index < update_index; index++) {
		cse4589_print_and_log("%-15d%-15d%-15d\n", this_router.routing_table.entry[index].id, this_router.routing_table.additional_info[index].nexthop, this_router.routing_table.entry[index].cost);
	}
	
}

/********************************************************************************
*   Name:   disable
*   Desc:   disables a link @ neighbor
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void disable(uint16_t id) {
	
	int target_index;

	target_index = find_entry_by_id(id);

	//Is it a neighbor? Don't close connection of an innocent guy.
	if (this_router.routing_table.additional_info[target_index].nexthop == this_router.id)
	{
		// Set cost to INF, nexthop to -1, cost to INF and counter to -1
        update_link_routing_table_entry(id, -1, INF);
        this_router.routing_table.additional_info[target_index].counter = -1; 
        cse4589_print_and_log("%s:SUCCESS\n", "disable");
        return;
	}

	cse4589_print_and_log("%s:%s\n", "disable", "Can not close connection. Not a neighbor.");
	return;
}

/********************************************************************************
*   Name:   crash
*   Desc:   crashes
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void crash() {

	int index=0;

	for(index=0; index < update_index; index++) {
		kill_connection(index);
	}

	while(1) {

	}

	// Should not reach here
	cse4589_print_and_log("%s:SUCCESS\n", "crash");
}

/********************************************************************************
*   Name:   dump
*   Desc:   dumps a message
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void dump() {
	char *msg;
	size_t msg_size;
    
    cse4589_print_and_log("%s:SUCCESS\n", "dump");
	msg = prepare_message(&msg_size);
	cse4589_dump_packet(msg, msg_size);
	
	free(msg);
}

/********************************************************************************
*   Name:   academic_integrity
*   Desc:   
*   Ret:    Nothing
*   Ref:    None
********************************************************************************/
void academic_integrity() {
	cse4589_print_and_log("I have read and understood the course acacdemic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
	cse4589_print_and_log("%s:SUCCESS\n", "academic_integrity");
}





