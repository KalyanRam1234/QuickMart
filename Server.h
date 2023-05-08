#ifndef SERVER

#define SERVER
#include "Variables.h"
#define max(a,b) a>b ? a: b;

struct Database{
    int Datafd;
    int Deletedfd;
    int productscount;
    int DeletedP[1000];
    int Cartfd;
};

struct Cart{ //need to chage to products
    char username[100];
    struct Cartitem items[1000];
    int userid;
};

int StartServer(void);

void pthread_func(void *arg);

int AddProduct(struct Product p);

int UpdateProductQ(struct Product p, int option);

int UpdateProductP(struct Product p);

struct Product* DisplayProducts(int fd,int opt, struct Cartitem items[]);

void FileOperations();

int DeleteProduct(int id);

#endif

