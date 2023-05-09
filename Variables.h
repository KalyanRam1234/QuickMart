#ifndef VARIABLE
#define VARIABLE

#include <netinet/in.h>
#include <sys/types.h>        
#include <sys/socket.h>
#include <arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/time.h>
#include <signal.h>
#define MAX_SIZE 1000

struct Cartitem{
    int ProductId;
    int quantity;
};
struct Product{
    int ProductId;
    char ProductName[60];
    int quantity;
    int cost;
};

#endif // !VARIABLE
