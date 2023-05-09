#include<stdio.h>
#include<string.h>
#include "Server.h"
#include "Variables.h"
struct Database *DB;

void signal_handler(){
    printf("Interrupt signal caught\n");
    close(DB->Datafd);
    close(DB->Deletedfd);
    close(DB->Cartfd);
    exit(0);
}

int main()
{
    DB=(struct Database *)malloc(sizeof(struct Database));
    signal(SIGINT, &signal_handler);
    int serverFD = StartServer();
    
    int i=0;
    struct sockaddr_in serv;
    int addrlen=sizeof(serv);

    listen (serverFD, 100);

    while(1){
        int nsd = accept (serverFD, (struct sockaddr*)&serv, (socklen_t*)&addrlen);
        if(nsd==-1) break;
        
        if(!fork()){
            pthread_func(&nsd);
        }
        else close(nsd);

    }
    shutdown(serverFD,SHUT_RDWR);
    return 0;
}

void FileOperations(){

    int fd=open("Product.dat",O_CREAT|O_RDWR,0666);
    if(fd>0){
        DB->Datafd=fd;
    } 

    int fd1=open("Deleted.dat", O_CREAT|O_RDWR, 0666);
    if(fd1>0){
        DB->Deletedfd=fd1;
        int arr[MAX_SIZE];
        for(int i=0;i<MAX_SIZE;i++) arr[i]=-1;
        read(DB->Deletedfd,arr,MAX_SIZE);
        memcpy(DB->DeletedP,arr,sizeof(DB->DeletedP));
    } 

    int fd2=open("Cart.dat", O_CREAT|O_RDWR, 0666);
    if(fd2>0){
        DB->Cartfd=fd2;
    } 

    DB->productscount=lseek(DB->Datafd,0,SEEK_END)/sizeof(struct Product);

}


int StartServer(void){
    struct sockaddr_in serv;
    int addrlen=sizeof(serv);
    int sd = socket (AF_INET, SOCK_STREAM, 0);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr =INADDR_ANY;
    serv.sin_port = htons (8080);

    if(sd==-1){
        write(1,"The Server has failed to start\n",32);
        exit(0);
    }
    bind (sd, (struct sockaddr*)&serv, sizeof (serv));

    FileOperations();

    write(1,"The Server has started successfully\n", 37);
    return sd;
}

void pthread_func(void *arg){

    int *nsd=(int *) arg;
    struct Cart session={0};
    while(1){//Add other options

        char buffer[MAX_SIZE]={0};
        read(*nsd,buffer,MAX_SIZE);
        struct Product p={0};
        write(1,buffer,strlen(buffer));
        lseek(DB->Datafd,0,SEEK_SET);
        DB->productscount=lseek(DB->Datafd,0,SEEK_END)/sizeof(struct Product);
        write(1,"\n", 1);

        if(strcmp(buffer,"Add")==0){
            send(*nsd, "Please send Product Details", 28, 0);
            read(*nsd,&p,sizeof(struct Product));
            if(!AddProduct(p)) send(*nsd,"Successfully added product to Database\n", 40,0);

            else send(*nsd, "Failed to add product to Database\n",35,0);
        }

        else if(strcmp(buffer, "Delete")==0){
            send(*nsd, "Please send Product ID", 23, 0);

            int pid;
            read(*nsd, &pid, sizeof(int));
            write(1,&pid,4);
            if(!DeleteProduct(pid)) send(*nsd,"Successfully deleted product from Database\n", 44,0);

            else send(*nsd, "Failed to delete product from Database\n",40,0);
        }

        else if(strcmp(buffer, "UpdateQ")==0){

            send(*nsd, "Please send Updated Product Details", 36, 0);
            read(*nsd,&p,sizeof(struct Product));
            if(!UpdateProductQ(p,0)) send(*nsd,"Successfully updated product in Database\n", 44,0);

            else send(*nsd, "Failed to update product in Database\n",40,0);

        }

        else if(strcmp(buffer, "UpdateP")==0){
            send(*nsd, "Please send Updated Product Details", 36, 0);
            read(*nsd,&p,sizeof(struct Product));

            if(!UpdateProductP(p)) send(*nsd,"Successfully updated product in Database\n", 44,0);

            else send(*nsd, "Failed to update product in Database\n",40,0);
        }

        else if(strcmp(buffer, "DisplayP")==0){

            struct Product *ps=(struct Product *)malloc(sizeof(struct Product)*MAX_SIZE);
            ps=DisplayProducts(DB->Datafd, 0,(void *)0);
            int i=0;
            send(*nsd,ps,sizeof(struct Product)*MAX_SIZE,0);
        }

        else if(strcmp(buffer, "DisplayCart")==0){
            lseek(DB->Cartfd,(session.userid-1)*sizeof(struct Cart),SEEK_SET);
            read(DB->Cartfd,&session,sizeof(struct Cart));
            struct Product *ps=(struct Product *)malloc(sizeof(struct Product)*MAX_SIZE);

            ps=DisplayProducts(DB->Datafd,1,session.items);

            send(*nsd,ps,sizeof(struct Product)*MAX_SIZE,0);

        }

        else if(strcmp(buffer,"BuyNow")==0){
            struct Product *ps=(struct Product *)malloc(sizeof(struct Product)*MAX_SIZE);
            int quantity[MAX_SIZE];
            lseek(DB->Cartfd,(session.userid-1)*(sizeof(struct Cart)),SEEK_SET);
            ps=DisplayProducts(DB->Datafd, 0,(void *)0);

            for(int i=0;i<MAX_SIZE;i++){
                if(ps[i].ProductId<=MAX_SIZE) quantity[ps[i].ProductId]=ps[i].quantity;
            }
            send(*nsd,quantity,sizeof(int)*MAX_SIZE,0);

            ps=DisplayProducts(DB->Datafd,1,session.items);
            send(*nsd,ps,sizeof(struct Product)*MAX_SIZE,0);

            struct flock lock[MAX_SIZE];
            for(int i=0;i<MAX_SIZE;i++){
                if(ps[i].ProductId>0 && ps[i].quantity<=quantity[ps[i].ProductId]){
                    int q=ps[i].ProductId;
                    memset (&lock[q], 0, sizeof(lock[q]));
                    lock[q].l_type=F_WRLCK;
                    lock[q].l_whence=SEEK_SET;
                    lock[q].l_start=(q-1)*sizeof(struct Product);
                    lock[q].l_len=sizeof(struct Product);
                    lock[q].l_pid=getpid();
                    fcntl(DB->Datafd, F_SETLKW, &lock[q]);
                } 
            }

            char buf[100];
            read(*nsd,buf,100);
            if(strcmp(buf,"ResetCart")==0){

                for(int i=0;i<MAX_SIZE;i++){
                    if(ps[i].ProductId>0 && ps[i].quantity<=quantity[ps[i].ProductId]){
                        int x=UpdateProductQ(ps[i],1);
                        int q=ps[i].ProductId;
                        
                        if(!x){

                            session.items[q].ProductId=-1;
                            session.items[q].quantity=0;
                            lseek(DB->Cartfd,(session.userid-1)*(sizeof(struct Cart)),SEEK_SET);
                            write(DB->Cartfd,&session, sizeof(struct Cart));
                        }

                        lock[q].l_type = F_UNLCK;
                        fcntl(DB->Datafd, F_SETLKW, &lock[q]); 
                    } 
                }
                
                send(*nsd,"Payment is successful, the receipt is available in pwd.\nYour cart is updated\n",78,0);
            }

            else if(strcmp(buf,"Failed")==0){

                for(int i=0;i<MAX_SIZE;i++){
                    if(ps[i].ProductId>0 && ps[i].quantity<=quantity[ps[i].ProductId]){
                        int q=ps[i].ProductId;
                        lock[q].l_type = F_UNLCK;
                        fcntl(DB->Datafd, F_SETLKW, &lock[q]); 
                    } 
                }
                
            }
        }

        else if(strcmp(buffer, "UpdateCart")==0){
        
            lseek(DB->Cartfd,(session.userid-1)*(sizeof(struct Cart)),SEEK_SET);

            char choice[50];
            read(*nsd, choice,20);
            struct Cartitem item;

            if(strcmp(choice,"AddCart")==0){

                read(*nsd,&item,sizeof(struct Cartitem));
                if(item.ProductId<=MAX_SIZE){
                    if(DB->DeletedP[item.ProductId]==-1 && session.items[item.ProductId].ProductId<=0 && item.ProductId<=DB->productscount)
                    {
                        lseek(DB->Datafd,(item.ProductId-1)*sizeof(struct Product),SEEK_SET);
                        read(DB->Datafd,&p,sizeof(struct Product));
                        if(p.quantity>=item.quantity){
                            session.items[item.ProductId].ProductId=item.ProductId;
                            session.items[item.ProductId].quantity=item.quantity;

                            write(DB->Cartfd,&session, sizeof(struct Cart)); //check if quantity is available
                            send(*nsd, "Added to Cart!!\n",17,0);
                        }
                        else send(*nsd, "The quantity requested isn't available!!\n",42,0);

                    } 
                    else send(*nsd, "This Product doesn't exist or is already in cart!\n",51,0);
                }
                else send(*nsd, "Invalid Product Id sent!!\n",27,0);

            }

            else if(strcmp(choice,"DeleteCart")==0){

                read(*nsd,&item,sizeof(struct Cartitem));
                if(item.ProductId<=MAX_SIZE){
                    if(DB->DeletedP[item.ProductId]==-1 && session.items[item.ProductId].ProductId >0 && item.ProductId<=DB->productscount){
                        
                        session.items[item.ProductId].ProductId=-1;
                        session.items[item.ProductId].quantity=0;
                        write(DB->Cartfd,&session, sizeof(struct Cart)); 
                        send(*nsd, "Deleted from Cart!!\n",21,0);

                    } 
                    else send(*nsd, "This Product doesn't exist or is already deleted!\n",51,0);
                }
                else send(*nsd, "Invalid Product Id sent!!\n",27,0);
            }

            else if(strcmp(choice,"UpdateCart")==0){

                read(*nsd,&item,sizeof(struct Cartitem));
                if(item.ProductId<=MAX_SIZE){
                    if(DB->DeletedP[item.ProductId]==-1 && session.items[item.ProductId].ProductId>0 && item.ProductId<=DB->productscount)
                    {
                        lseek(DB->Datafd,(item.ProductId-1)*sizeof(struct Product),SEEK_SET);
                        read(DB->Datafd,&p,sizeof(struct Product));

                        if(p.quantity>=item.quantity){
                            session.items[item.ProductId].ProductId=item.ProductId;
                            session.items[item.ProductId].quantity=item.quantity;

                            write(DB->Cartfd, &session, sizeof(struct Cart)); //check if quantity is available
                            send(*nsd, "Updated the Cart!!\n",20,0);
                        }
                        else send(*nsd, "The quantity requested isn't available!!\n",42,0);

                    } 
                    else send(*nsd, "This Product doesn't exist or is Deleted!\n",43,0);
                }
                else send(*nsd, "Invalid Product Id sent!!\n",27,0);
            }

        }
        
        else if(strcmp(buffer, "Login")==0){

            send(*nsd,"Enter Username: ", 17, 0);
            char response[100]={0};
            read(*nsd, response,100);
            lseek(DB->Cartfd,0,SEEK_SET);
            struct Cart user={0};//can have error
            strcpy(session.username,response);
            int check=0,newuserid=0;
            while(read(DB->Cartfd,&user,sizeof(struct Cart))>0){
                if(strcmp(user.username,response)==0) {
                    session.userid=user.userid;
                    for(int i=0;i<MAX_SIZE;i++){
                        session.items->ProductId=user.items->ProductId;
                        session.items->quantity=user.items->quantity;
                    }
                    // session.items=user.items;
                    check=1;
                    break;
                }
                newuserid=max(newuserid,user.userid); 
            }
            if(check==0){
                session.userid=newuserid+1;
                memset(session.items,-1,sizeof(struct Cartitem)*MAX_SIZE);
                write(DB->Cartfd,&session,sizeof(struct Cart));
            } 

        }
        else if(strcmp(buffer, "END")==0) break;

    }
    return;
    
}

int AddProduct(struct Product p){
    
    p.ProductId=DB->productscount+1;

    lseek(DB->Datafd,0,SEEK_END);
    if(write(DB->Datafd,&p,sizeof(struct Product))>0){
        // DB->productscount+=1;
        return 0;
    };
    return 1;
}

int DeleteProduct(int id){//Add existing error handling
    // int i=0;
    if(id>=DB->productscount) return 1;
    // while(DB->DeletedP[i]!=-1 && i<MAX_SIZE) i++;
    if(id<MAX_SIZE){
        DB->DeletedP[id]=id;
        lseek(DB->Deletedfd,0,SEEK_SET);
        int w=write(DB->Deletedfd,&DB->DeletedP,sizeof(DB->DeletedP));
        if(w>0){
            return 0;
        }
    } 
    return 1;

}

int UpdateProductQ(struct Product p,int option){

    lseek(DB->Datafd,0,SEEK_SET);
    struct Product fp;
    
    if(p.ProductId > DB->productscount) return 1; //have to update when server starts
    if(DB->DeletedP[p.ProductId]>0) return 1;

    struct flock lock;
    if(!option){
       
        memset (&lock, 0, sizeof(lock));
        lock.l_type=F_WRLCK;
        lock.l_start=(p.ProductId-1)*sizeof(struct Product);
        lock.l_whence=SEEK_SET;
        lock.l_len=sizeof(struct Product);
        lock.l_pid=getpid();
        fcntl(DB->Datafd, F_SETLKW, &lock);
    }

    lseek(DB->Datafd, (p.ProductId-1)*sizeof(struct Product),SEEK_SET);

    read(DB->Datafd,&fp,sizeof(struct Product));
    p.cost=fp.cost;
    strcpy(p.ProductName,fp.ProductName);

    if(option==1){
        if(p.quantity<=fp.quantity){
            p.quantity=fp.quantity-p.quantity;
        }
    }
    lseek(DB->Datafd, (-1)*sizeof(struct Product),SEEK_CUR);
    int x=write(DB->Datafd,&p,sizeof(struct Product));

    if(!option){
        lock.l_type = F_UNLCK;
        fcntl(DB->Datafd, F_SETLKW, &lock); 
    }
    
    if(x>0) return 0;
    return 1;

}

int UpdateProductP(struct Product p){
    lseek(DB->Datafd,0,SEEK_SET);
    struct Product fp;
    int i=0;
    if(p.ProductId > DB->productscount) return 1;
    // while(DB->DeletedP[i]!=-1){
        if(DB->DeletedP[p.ProductId]>0) return 1;
        // i++;
    // } 
    struct flock lock;
    memset (&lock, 0, sizeof(lock));
    lock.l_type=F_WRLCK;
    lock.l_start=(p.ProductId-1)*sizeof(struct Product);
    lock.l_whence=SEEK_SET;
    lock.l_len=sizeof(struct Product);

    fcntl(DB->Datafd, F_SETLKW, &lock);

    lseek(DB->Datafd, (p.ProductId-1)*sizeof(struct Product),SEEK_SET);
    read(DB->Datafd,&fp,sizeof(struct Product));
    p.quantity=fp.quantity;
    strcpy(p.ProductName,fp.ProductName);
    lseek(DB->Datafd, (-1)*sizeof(struct Product),SEEK_CUR);
    int x=write(DB->Datafd,&p,sizeof(struct Product));

    lock.l_type = F_UNLCK;
    fcntl(DB->Datafd, F_SETLKW, &lock);
    if(x>0) return 0;
    return 1;
}

struct Product* DisplayProducts(int fd, int opt, struct Cartitem items[]){
 
    lseek(fd,0,SEEK_SET);

    int i=0;//number of products

    struct Product p;
    struct Product *products=(struct Product *)malloc(MAX_SIZE*(sizeof(struct Product)));
    while(read(fd,&p,sizeof(struct Product))>0){ 
        if(DB->DeletedP[p.ProductId]==-1){
            products[i]=p;
            if(i<MAX_SIZE) i++;
            else break;
        }
    }

    if(opt==1){
        
        struct Product *carts=(struct Product *)malloc(MAX_SIZE*(sizeof(struct Product)));
        int x=0;
        for(int index=0;index<i;index++){
            if(items[products[index].ProductId].ProductId>0){
                carts[x].cost=products[index].cost;
                carts[x].ProductId=products[index].ProductId;
                strcpy(carts[x].ProductName,products[index].ProductName);
                carts[x].quantity=items[products[index].ProductId].quantity;
                x++;
            } 
        }
        return carts;
    }

    return products;
}