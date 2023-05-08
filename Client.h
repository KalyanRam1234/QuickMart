#ifndef CLIENT

#define CLIENT
#include "Variables.h"

struct Clientdetails{
    int clientFD;
    // struct Product item[1000];
    char username[100];
    // int cartsize;
};

void AdminView(int clientFD);

void ClientView(int clientFD);

int ConnectToServer();

void DisplayProducts(int clientFD);

#endif // !CLIENT