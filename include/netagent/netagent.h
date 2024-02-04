#define WIN32_LEAN_AND_MEAN

#pragma once

#include <iostream>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "FastCont/FastCont.h"
// #include <stdlib.h>

#define MAX_BUF_LEN 512

#define DEFAULT_PORT "27015"

class NetClient;

class NetClient {
    SOCKET ConnectSocket;
    int err;

public:
    char recvbuf[MAX_BUF_LEN];
    int init_recvbuflen = MAX_BUF_LEN;
    int recvbuflen = MAX_BUF_LEN;
    NetClient(std::string);
    NetClient();
    ~NetClient();
    int init(std::string);
    void closeConnection();

    int getConnectionStatus();

    int sendData(const char *, int);
    int recieveData();
};


enum recieveData_ret {
    recieveData_SUCCESS = 0,
    recieveData_CONNECTION_CLOSED = -1
};

enum getConnectionStatus_ret {
    getConnectionStatus_GOOD = 0,
    getConnectionStatus_NO_HOST = -1,
    getConnectionStatus_ERR = -2,
};

enum recieveData_err {
    recieveData_OK = 0,
    recieveData_NO_NEW_DATA = -1,
    recieveData_CONN_CLOSED_BY_CLIENT_ERR = -2,
    recieveData_NO_CLIENT_ERR = -3,
    recieveData_ERR = -4,
};

#include "netagent/netagent_c.cpp"