#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <queue>
#include <thread>
#include <chrono>

typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

inline void err_quit(const char *msg)
{
    char *msgbuf = strerror(errno);
    printf("[%s] %s\n", msg, msgbuf);
    exit(1);
}

inline void err_display(const char *msg)
{
    char *msgbuf = strerror(errno);
    printf("[%s] %s\n", msg, msgbuf);
}
