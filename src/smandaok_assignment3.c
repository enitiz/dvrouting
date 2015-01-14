/**
 * @smandaok_assignment3
 * @author  Saurabh Sidharth Mandaokar <smandaok@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 */

//#include <stdio.h>
//#include <stdlib.h> 
#include "header.h"
#include "../include/global.h"
#include "../include/logger.h"


/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log();

	/*Clear LOGFILE and DUMPFILE*/
	fclose(fopen(LOGFILE, "w"));
	fclose(fopen(DUMPFILE, "wb"));
    
	/*Start Here*/

    /***************************************
    * Declarations
    ***************************************/
    int rv=0;
    char *topath = NULL;
    long int update_interval=0;
    FILE *tofile;
    int sock_in=0;

    char command[CMD_LEN];
    int count=0;
    char *arg;
    char *command_tokens[60];

    /***************************************
    * Init
    ***************************************/
    
    memset(command, 0, CMD_LEN);

    /***************************************
    * Get path to topology file and router update interval
    ***************************************/
    rv = get_args(&topath, &update_interval, argc, argv);
    if(SUCCESS != rv) {
        fprintf(stderr, "Failed to get one or more required parameters to execute further! Exiting.\n");
        exit(EXIT_FAILURE);
    }

    /***************************************
    * Open topology file
    ***************************************/
    tofile = open_file(topath);
    if(NULL == tofile) {
        fprintf(stderr, "File not found. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    /***************************************
    * Set IP Address
    ***************************************/
    this_router.ip_addr = get_this_router_ip_addr();

    /***************************************
    * Read topology file
    ***************************************/
    read_topology(tofile);


    /***************************************
    * Initialize receiving socket
    ***************************************/
    sock_in = new_sockin(this_router.port);
    
    /***************************************
    * Timeout Implementation
    ****************************************/
    struct timeval perm_timeout, temp_timeout;

    perm_timeout.tv_sec = update_interval;
    perm_timeout.tv_usec = 0;


    /***************************************
    * Select Loop
    ****************************************/    
    fd_set perm_fdset, temp_fdset;

    FD_ZERO(&perm_fdset);
    FD_ZERO(&temp_fdset);

    FD_SET(STDIN_FILENO, &perm_fdset);
    FD_SET(sock_in, &perm_fdset);

    temp_timeout = perm_timeout;
    
    while(1) {
        
        temp_fdset = perm_fdset;
 
        if(select(sock_in+1, &temp_fdset, NULL, NULL, &temp_timeout) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }
            // Incoming message
            if(FD_ISSET(sock_in, &temp_fdset)) {
                get_message_and_update(sock_in);
            }
                    
            // Incoming user input
            if(FD_ISSET(STDIN_FILENO, &temp_fdset)) {
                

                fgets(command, CMD_LEN, stdin);
                count = 0;

                arg = strtok(command, " ");

                while(arg!=NULL) {

                    string_lowcase(arg);
                    command_tokens[count] = malloc(sizeof(arg)*sizeof(char));
                    strcpy(command_tokens[count], arg);
                    count++;
                    arg = strtok(NULL, " ");
                }


                 if(0 == strncmp(command_tokens[0], "academic_integrity", 18)) {
                 
                    academic_integrity();
                 
                 }
                 else if(0 == strncmp(command_tokens[0], "update", 6)) {
                    
                    if((uint16_t) atoi(command_tokens[1]) < 0 || (uint16_t) atoi(command_tokens[2]) < 0 || (uint16_t) atoi(command_tokens[3]) < 0) {

                        cse4589_print_and_log("%s:%s\n", "update", "invalid arguments");
                    }
                    else {
                        update((uint16_t) atoi(command_tokens[1]), (uint16_t) atoi(command_tokens[2]), (uint16_t) atoi(command_tokens[3]));
                    }
                 }
                 else if(0 == strncmp(command_tokens[0], "step", 4)) {
                 
                    step();
                 
                 }
                 else if(0 == strncmp(command_tokens[0], "packets", 7)) {
                 
                    packets();
                 
                 }
                 else if(0 == strncmp(command_tokens[0], "display", 7)) {
                 
                    display();
                 
                 }
                 else if(0 == strncmp(command_tokens[0], "disable", 7)) {
                 
                    if((uint16_t) atoi(command_tokens[1]) < 0) {
                        cse4589_print_and_log("%s:%s\n", "disable", "invalid argument");
                    }
                    else {    
                        disable((uint16_t) atoi(command_tokens[1]));
                    }
                 
                 }
                 else if(0 == strncmp(command_tokens[0], "crash", 5)) {
                 
                    crash();
                 
                 }
                 else if(0 == strncmp(command_tokens[0], "dump", 4)) {
                 
                    dump();
                 
                 }
            printf("Nothing to do!\n");
            }
            
            // Check for timeout
            if(temp_timeout.tv_sec == 0 && temp_timeout.tv_usec == 0) {
                    
                increment_counters();
                disable_old_links();
                send_message_to_neighbors();

                temp_timeout = perm_timeout;
            }
    }
  

    /***************************************
    * Close topology file
    ***************************************/
    rv = close_file(tofile);
    if(SUCCESS != rv) {
        fprintf(stderr, "Failed to close file %s\n", topath);
    }
    free(topath);
    
    /***************************************
    * Return
    ***************************************/
	return EXIT_SUCCESS;
}

  