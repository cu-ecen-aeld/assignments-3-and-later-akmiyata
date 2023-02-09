// Socket/server code for assignment 5 part 1
// Author: Adam Miyata
// Date: See github date; started 02/08/2023
// Modified code from: https://beej.us/guide/bgnet/html/ (assignment 5 recommended reading)

//Requirements:
//Open a stream socket bound to port 9000, fail and return -1 if any connection steps fail. | Check.
//Listen for & accept connection. | Done in main() 
//Log message to syslog "Accepted connection from xxx" | Ok.
//Receive data and append to /var/tmp/aesdsocketdata, create file if it doesn't exist | Check.
//Return the content of /var/tmp/aesdsocketdata as soon as data packet completes | Check.
//Log message to syslog "Closed connection from xxx" | Check.
//Restart accepting connections from new clients forever until SIGINT or SIGTERM is rcvd | Check.
//Gracefully exit when SIGINT or SIGTERM is rcvd, complete open connection operations, close open sockets, and delete /var/tmp/aesdsocketdata. Log message to syslog- "Caught signal, exiting" when SIGINT or SIGTERM rcvd. | Check.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>

#define PORT "9000"  // the port users will be connecting to
#define BACKLOG 10
#define MAX_DAT 100  // Maximum no. of bytes receivable, +1
#define OUTPUT_FILE "/var/tmp/aesdsocketdata"

char buf[MAX_DAT];

static volatile int keepRunning = 1;

void write_stream(char* streamdata)
{
	FILE* fstream = fopen(OUTPUT_FILE, "ab+");
//	printf("Writing data to file");	
	fputs(streamdata, fstream);
	fclose(fstream);
}

void sig_intrpt(int s)
{
	keepRunning = 0;
	syslog(LOG_INFO, "Caught signal, exiting\n");
	remove(OUTPUT_FILE);
	printf("Removed data file from /var/tmp/");
	closelog();
	exit(0);
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int c, char **argv)
{
    signal(SIGINT, sig_intrpt);
    signal(SIGTERM, sig_intrpt);

    while(keepRunning==1)
    {
    	openlog(NULL,0,LOG_USER);
    	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
    	struct sockaddr_storage their_addr; // connector's address information
    	socklen_t sin_size;
    	struct sigaction sa;
    	int yes=1;
	int d_test;
	//int count = 0;
    	char s[INET6_ADDRSTRLEN];
    	int rv;
	if (c==2)
	{
	    daemon(0,0);
	    d_test = strcmp(argv[1],"-d");
	    if(d_test != 0)
	    {
		printf("Daemon error");
	    }
	}
	else
	{
	    printf("Server running in daemon");
	}

    	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE; // use my IP

    	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) 
	{
        	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        	return 1;
    	}

    	// loop through all the results and bind to the first we can
    	for(p = servinfo; p != NULL; p = p->ai_next) 
    	{
        	if ((sockfd = socket(p->ai_family, p->ai_socktype,
                	p->ai_protocol)) == -1) 
		{
            	    perror("server: socket");
            	    continue;
        	}

	        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        	        sizeof(int)) == -1) 
		{
	            perror("setsockopt");
        	    exit(1);
        	}

        	if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
            	    close(sockfd);
            	    perror("server: bind");
            	    continue;
        	}

        	break;
    	}

    	freeaddrinfo(servinfo); // all done with this structure

    	if (p == NULL)  
    	{
            fprintf(stderr, "server: failed to bind\n");
            exit(1);
    	}

    	if (listen(sockfd, BACKLOG) == -1) 
    	{
            perror("listen");
            exit(1);
    	}

    	sa.sa_handler = sigchld_handler; // reap all dead processes
    	sigemptyset(&sa.sa_mask);
    	sa.sa_flags = SA_RESTART;
    	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    	{
            perror("sigaction");
            exit(1);
    	}

    	printf("server: waiting for connections...\n");

    	while(1) 
    	{  // main accept() loop
            sin_size = sizeof their_addr;
            new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
            if (new_fd == -1) 
	    {
                perror("accept");
                continue;
            }

            inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
            printf("Accepted connection from %s\n", s);
	    syslog(LOG_INFO, "Accepted connection from %s\n", s);

            if (!fork()) 
	    { // this is the child process
                close(sockfd); // child doesn't need the listener           
	        while(1)
	        {
		    memset(buf, 0, sizeof(buf));
		    long int inc_bytes = recv(new_fd, buf, MAX_DAT-1, 0);
		    if(inc_bytes == -1)
		    {
		        exit(1);
		    }
		    write_stream(buf);
		    if(buf[inc_bytes-1] == '\n')
		    {
			//fclose(OUTPUT_FILE);
			break;
		    }
		}
                FILE* content_send = fopen(OUTPUT_FILE, "r");
		char* x;
		char buf[MAX_DAT];
		while((x=fgets(buf,MAX_DAT,content_send))!=NULL)
                    {
                        send(new_fd, x, strnlen(buf, MAX_DAT), 0);
                    }
	        fclose(content_send);
		
		
		//for (c = getc(content_send); c != EOF; c = getc(content_send))
                //{
                //    x=c;
		//    printf("character is: %s\n",&c);
                //    count = count + 1;
		//    if(c!=' ')
		//    {
		//    printf("Sent character to client");	    
		//    send(new_fd, x, 1, 0);
		//    }
                //}
                //printf("Character length is: %d\n", count);

		//fclose(OUTPUT_FILE);
	        syslog(LOG_INFO, "Closed connection from %s\n", s);
		printf("Closed connection.");
		exit(0);
		close(new_fd);          	
	    }

            close(new_fd);  // parent doesn't need this  
//            syslog(LOG_INFO, "Closed connection from %s\n", s);
        }
    }  //End of 1st while loop
//    remove(OUTPUT_FILE);
    return 0;
}

