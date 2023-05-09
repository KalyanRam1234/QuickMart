#include<stdio.h>
#include<string.h>
#include "Client.h"
#include "Variables.h"

struct Clientdetails cld;

void signal_handler(){
    printf("Interrupt Signal Caught\n");
    send(cld.clientFD,"END", 3,0);
    exit(0);
}

int main(){
    write(1,"Operating System Project\n", 26);

    int clientFD=ConnectToServer();
    cld.clientFD=clientFD;
    signal(SIGINT, &signal_handler);

    while(1){
        write(1,"------------------------------------------------------------------------\n",74);
        write(1,"\n1)Admin\n2)User\n3)Exit from application\n\n", 42);
        write(1,"Select an option: ", 19);
        int choices=0;
        scanf("%d", &choices);
        write(1,"------------------------------------------------------------------------\n",74);
        
        if(choices==1){
            AdminView(cld.clientFD);
        }
        else if(choices==2){
            send(cld.clientFD,"Login", 5, 0);
            char response[100];
            read(cld.clientFD,response,100);
            write(1,response,strlen(response));
            char username[100];
            scanf("%s", username);
            strcpy(cld.username,username);
            send(cld.clientFD,cld.username,strlen(cld.username),0);
            ClientView(cld.clientFD);
        }
        else{
            send(cld.clientFD,"END", 3,0);
            shutdown(cld.clientFD, O_RDWR);
            break;
            
        }
    }
    return 0;
}

//Will implement file locking later
void AdminView(int clientFD){
    write(1,"Admin Operations \n",19);

    while(1){ 

        write(1,"\n1)Add a product\n2)Delete a product\n3)Update the quantity of a product\n4)Update the price of a product\n5)Display All Products\n6)Admin Exit\n\n",141);

        write(1, "Enter an option: ", 18);
        int choice=0;
        char response[MAX_SIZE]={0};
        scanf("%d", &choice);

        if(choice==1){
            int x=send(clientFD, "Add", 3,0);

            if(x>0){
                read(clientFD, response, 100);
                struct Product p;
                write(1, response, strlen(response));

                write(1, "\nEnter Product Name: ",22);
                char name[100];
                scanf("%s",p.ProductName);
                // scanf("\n");
                // fgets(name, 100, stdin);
                // strcpy(p.ProductName,name);

                write(1, "Enter Product Cost: ",21);
                scanf("%d", &p.cost);

                write(1, "Enter Product Quantity: ",25);
                scanf("%d", &p.quantity);

                send(clientFD,&p,sizeof(struct Product), 0);

                memset(response, '\0', sizeof(response));
                read(clientFD,response, 100);
                write(1,response,strlen(response));

            }
            else write(1,"Couldn't Connect to Server\n",28);

        }
        else if(choice==2){ 
            int x=send(clientFD, "Delete", 7,0);

            if(x>0){
                read(clientFD, response, 100);
                write(1, response, strlen(response));

                write(1, "\nEnter Product ID: ",20);
                int id=0;
                scanf("%d", &id);
                send(clientFD,&id,sizeof(int), 0);

                memset(response, '\0', sizeof(response));
                read(clientFD,response, 100);
                write(1,response,strlen(response));

            }
            else write(1,"Couldn't Connect to Server\n",28);

        }
        else if(choice==3){ //just need to overwrite at the correct location , using the product id
            int x=send(clientFD, "UpdateQ", 8,0);
            if(x>0){
                struct Product p;
                read(clientFD, response, 100);
                write(1, response, strlen(response));

                write(1, "\nEnter Product ID: ",20);
                int id=0,quantityP;
                scanf("%d", &id);

                write(1, "\nEnter New Quantity: ",20);
                scanf("%d", &quantityP);

                p.ProductId=id;
                p.quantity=quantityP;
                send(clientFD,&p,sizeof(struct Product), 0);

                memset(response, '\0', sizeof(response));
                read(clientFD,response, 100);
                write(1,response,strlen(response));

            }
            else write(1,"Couldn't Connect to Server\n",28);
        }
        else if(choice==4){
            int x=send(clientFD, "UpdateP", 8,0);

            if(x>0){
                struct Product p;
                read(clientFD, response, 100);
                write(1, response, strlen(response));

                write(1, "\nEnter Product ID: ",20);
                int id=0,cost;
                scanf("%d", &id);

                write(1, "\nEnter New Cost: ",18);
                scanf("%d", &cost);

                p.ProductId=id;
                p.cost=cost;
                send(clientFD,&p,sizeof(struct Product), 0);

                memset(response, '\0', sizeof(response));
                read(clientFD,response, 100);
                write(1,response,strlen(response));

            }
            else write(1,"Couldn't Connect to Server\n",28);
        }
        else if(choice==5){
            DisplayProducts(clientFD);
        }
        else if(choice>=6){
            break;
        } 


        write(1,"------------------------------------------------------------------------\n",74);
    }
}

void ClientView(int clientFD){
    write(1,"Client Operations \n",20);

    while(1){
        write(1,"\n1)Display all Products\n2)Display Cart\n3)Update Cart\n4)Buy items in Cart\n5)Client Exit\n\n",89);
        write(1, "Enter an option: ", 18);
        int choice=0;
        scanf("%d", &choice);
        char response[MAX_SIZE];
        if(choice==1){
            DisplayProducts(clientFD);
        }
        else if(choice==2){
            
            write(1,"Client Cart\n",13);

            int x=send(clientFD, "DisplayCart", 12,0);
            if(x>0){
                
                memset(response, '\0', sizeof(response));
                
                struct Product *p=(struct Product *)malloc(sizeof(struct Product)*MAX_SIZE);
                read(clientFD, p,sizeof(struct Product)*MAX_SIZE);
                write(1,"-------------------------------------------------------------------------\n",75);
        // write(1, "P_ID, P_Name, Cost, Quantity\n",30);
                write(1,"| P_ID\t| P_Name\t\t| Cost\t  | Quantity | SubTotal\t\t|\n",52);
                write(1,"-------------------------------------------------------------------------\n",75);
                int i=0;
                int totalcost=0;
                while(p[i].ProductId!=0 && i<MAX_SIZE){
                    printf("| %d\t| %-22s| %-8d| %-9d| %-17d|\n", p[i].ProductId, p[i].ProductName, p[i].cost, p[i].quantity, p[i].cost*p[i].quantity);
                    totalcost+=p[i].quantity*p[i].cost;
                    write(1,"-------------------------------------------------------------------------\n",75);
                    i++;
                }

                printf("\nTotal Cart Size: %d\n", i);
                printf("Total Cost: %d\n\n", totalcost);
            }
            else write(1,"Couldn't Connect to Server\n",28);

        }
        else if(choice==3){
            send(clientFD,"UpdateCart",11,0);
            int subchoice;
            write(1,"1)Add new item to Cart\n2)Delete item from Cart\n3)Modify quantity of item in Cart\n\nEnter choice: ", 97);
            scanf("%d", &subchoice);
            memset(response, '\0', sizeof(response));

            if(subchoice==1){
                send(clientFD,"AddCart",8,0);
                write(1,"Enter ProductID: ", 18);
                struct Cartitem select;
                scanf("%d", &select.ProductId);
                write(1,"Enter quantity: ", 17);
                scanf("%d", &select.quantity);
                send(clientFD,&select,sizeof(struct Cartitem),0);
                read(clientFD,&response,100);
                write(1,&response,strlen(response));
            }

            else if(subchoice==2){
                send(clientFD,"DeleteCart",11,0);
                write(1,"Enter ProductID: ", 18);
                struct Cartitem select;
                scanf("%d", &select.ProductId);
                send(clientFD,&select,sizeof(struct Cartitem),0);
                read(clientFD,&response,100);
                write(1,&response,strlen(response));

            }

            else if(subchoice==3){
                send(clientFD,"UpdateCart",11,0);
                write(1,"Enter ProductID: ", 18);
                struct Cartitem select;
                scanf("%d", &select.ProductId);
                write(1,"Enter quantity: ", 17);
                scanf("%d", &select.quantity);
                send(clientFD,&select,sizeof(struct Cartitem),0);
                read(clientFD,&response,100);
                write(1,&response,strlen(response));
            }
            else send(clientFD,"Failed",7,0);

        }
        else if(choice==4){
            //need lock here
            memset(response, '\0', sizeof(response));
            send(clientFD, "BuyNow", 7,0);
            int quantity[MAX_SIZE],invalid[100];
            struct Product *cart=(struct Product *)malloc(sizeof(struct Product)*MAX_SIZE);
            read(clientFD, quantity,sizeof(int)*MAX_SIZE);
            read(clientFD,cart,sizeof(struct Product)*MAX_SIZE);
            
            write(1,"\nItems ready for buying: \n",27);
             write(1,"-------------------------------------------------------------------------\n",75);
        // write(1, "P_ID, P_Name, Cost, Quantity\n",30);
                write(1,"| P_ID\t| P_Name\t\t| Cost\t  | Quantity | SubTotal\t\t|\n",52);
                write(1,"-------------------------------------------------------------------------\n",75);
            int totalcost=0,size=0,esize=0;
            for(int i=0;i<MAX_SIZE;i++){
                if(cart[i].ProductId!=0){
                    if(cart[i].quantity<=quantity[cart[i].ProductId]){

                        printf("| %d\t| %-22s| %-8d| %-9d| %-17d|\n", cart[i].ProductId, cart[i].ProductName, cart[i].cost, cart[i].quantity, cart[i].cost*cart[i].quantity);

                        write(1,"-------------------------------------------------------------------------\n",75);
                        totalcost+=cart[i].quantity*cart[i].cost;
                        size++;
                    }
                    else invalid[esize++]=i;
                } 
            }
            if(esize>0){
                write(1,"The following products can't be bought as the quantity asked is more than the quantity available\n",98);
                for(int i=0;i<esize;i++){
                    printf("%s , quantity asked: %d, quantity available: %d\n", cart[invalid[i]].ProductName, cart[invalid[i]].quantity, quantity[cart[invalid[i]].ProductId]);
                }
            }

            printf("\nTotal Cart Size: %d\n", size);
            printf("Total Cost: %d\n", totalcost);

            write(1,"The items listed first are available, enter the total amount to verify payment\nEnter amount: ",94);

            int amount=0;
            scanf("%d",&amount);

            if(amount==totalcost){
                send(clientFD,"ResetCart",10,0);

                read(clientFD,response,100);
                write(1,response,strlen(response));


                char filename[100]="log";
                strcat(filename,cld.username);
                strcat(filename,".txt");
                int logfd=open(filename,O_CREAT|O_WRONLY,0666);
                write(logfd,"Receipt\n---------------------------------------------------------------------\n",78);

                write(logfd,"| P_ID\t| P_Name\t\t\t\t| Cost\t  | Quantity | SubTotal\t\t|\n",53);
                write(logfd,"---------------------------------------------------------------------\n",70);
                char output[200], result[200];
                for(int i=0;i<MAX_SIZE;i++){
                    if(cart[i].ProductId!=0){
                        if(cart[i].quantity<=quantity[cart[i].ProductId]){
                            
                            sprintf(output, "| %-5d\t| %-22s| %-8d| %-9d| %-13d|\n", cart[i].ProductId, cart[i].ProductName, cart[i].cost, cart[i].quantity, cart[i].cost*cart[i].quantity);
                            write(logfd,output,strlen(output));
                            write(logfd,"---------------------------------------------------------------------\n",70);
                        }
                    } 
                }
                sprintf(result,"\nTotal Cart Size: %d\nTotal Cost: %d\n", size,totalcost);
                write(logfd,result,strlen(result));
                
            }
            else {
                send(clientFD,"Failed",8,0);
                write(1,"Payment failed. Please try again!!\n", 36);
            } 
            // make log file
        }
        else if(choice>=5)break;

        write(1,"------------------------------------------------------------------------\n",74);
    }

    return ;
}

int ConnectToServer(){

    write(1, "Connecting to server...\n", 25);
    struct sockaddr_in serv;
    int sd=socket(AF_INET,SOCK_STREAM,0);
    serv.sin_family=AF_INET;
    serv.sin_addr.s_addr=INADDR_ANY;
    // inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);
    serv.sin_port=htons(8080);
    int p=connect(sd,(struct sockaddr*)&serv,sizeof(serv));
    if(!p){
        write(1,"Connected to server successfully\n", 34); 
        return sd;
    }
    else{ 
        write(1, "Connection Failed\n", 19);
        exit(0);
    }
    return 0;
}

void DisplayProducts(int clientFD){
    int x=send(clientFD, "DisplayP", 9,0);
    if(x>0){
        struct Product *p=(struct Product *)malloc(sizeof(struct Product)*MAX_SIZE);
        read(clientFD, p,sizeof(struct Product)*MAX_SIZE);
        write(1,"------------------------------------------------------\n",56);
        // write(1, "P_ID, P_Name, Cost, Quantity\n",30);
        write(1,"| P_ID\t| P_Name\t\t| Cost\t  | Quantity |\n",40);
        write(1,"------------------------------------------------------\n",56);
        int i=0;
        while(p[i].ProductId!=0 && i<MAX_SIZE){
            printf("| %d\t| %-22s| %-8d| %-9d|\n", p[i].ProductId, p[i].ProductName, p[i].cost, p[i].quantity);
            i++;
            write(1,"------------------------------------------------------\n",56);
        }
        write(1,"\n",2);
    }
    else write(1,"Couldn't Connect to Server\n",28);
}