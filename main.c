/*
 *  Copyright (C) 2015 Franz-Josef Anton Friedrich Haider
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define STREAM_PORT_NUM 22468

#define MAX_BUFFER_SIZE 512

int main(int argc, char *argv[])
{
    fd_set readset, tempset;
    int maxfd;
    int srvsock, peersoc, j, result;
    socklen_t len;
    struct timeval tv;
    char buffer[MAX_BUFFER_SIZE+1];
    struct sockaddr_in addr;
    struct sockaddr_in server_addr;

    srvsock = socket(AF_INET, SOCK_STREAM, 0);
    if(srvsock < 0)
    {
        printf("socket() failed\n");
        return EXIT_FAILURE;
    }

    bzero((char*)&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(STREAM_PORT_NUM);

    if(bind(srvsock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("bind() failed\n");
        return EXIT_FAILURE;
    }

    listen(srvsock, 5);
    printf("Listening on port %d\n", STREAM_PORT_NUM);

    FD_ZERO(&readset);
    FD_SET(srvsock, &readset);
    maxfd = srvsock;

    do {
        memcpy(&tempset, &readset, sizeof(tempset));
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        result = select(maxfd + 1, &tempset, NULL, NULL, &tv);

        if (result == 0) {
            printf("select() timed out!\n");
        }
        else if (result < 0 && errno != EINTR) {
            printf("Error in select(): %s\n", strerror(errno));
        }
        else if (result > 0) {

            if (FD_ISSET(srvsock, &tempset)) {
                len = sizeof(addr);
                peersoc = accept(srvsock, (struct sockaddr*)&addr, &len);
                if (peersoc < 0) {
                    printf("Error in accept(): %s\n", strerror(errno));
                }
                else {
                    FD_SET(peersoc, &readset);
                    maxfd = (maxfd < peersoc) ? peersoc : maxfd;
                }
                FD_CLR(srvsock, &tempset);
            }

            for (j=0; j<maxfd+1; j++) {
                if (FD_ISSET(j, &tempset)) {

                    do {
                        result = recv(j, buffer, MAX_BUFFER_SIZE, 0);
                    } while (result == -1 && errno == EINTR);

                    if (result > 0) {
                        buffer[result] = 0;
                        printf("RECEIVED: %d\n", result);

                    }
                    else if (result == 0) {
                        close(j);
                        FD_CLR(j, &readset);
                    }
                    else {
                        printf("Error in recv(): %s\n", strerror(errno));
                    }
                }
            }
        }
    } while (1);

    return EXIT_SUCCESS;
}
