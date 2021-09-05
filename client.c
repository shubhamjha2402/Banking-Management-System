#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"


int main(){
  //step 1: Connect to server
  struct sockaddr_in serv;
  int sd = socket (AF_INET, SOCK_STREAM, 0);
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = INADDR_ANY;;
  serv.sin_port = htons(5555);

  connect (sd, (struct sockaddr *) &serv, sizeof(serv));

  //step 2: Ask user for type
  int option;
  printf("..................Welcome to IIITB Bank..............\n");
  printf("\t\tPlease enter option to proceed\n");
  printf("Single account --> \t1\nJoint account --> \t2\nAdmin --> \t\t3\n");
  scanf("%d", &option);

  if(option == 1 || option == 2){

    struct request req;
    if(option == 1) req.req_id = SINGLE_ACCOUNT_REQUEST;
    else req.req_id = JOINT_ACCOUNT_REQUEST;
    write(sd, &req, sizeof(req));

    int accno;
    char pin[4];
    printf("Enter account number\n");
    scanf("%d", &accno);
    printf("Enter pin number\n");
    scanf("%s", pin);

    struct account acc;
    acc.accno = accno;
    strcpy(acc.pin, pin);

    write(sd, &acc, sizeof(acc));
    read(sd, &req, sizeof(req));

    if(req.status>=0){
      int choice;
      printf("Authorisation Successful. Please choose an option:\n1. Deposit\n2. Withdraw\n3. Balance Enquiry\n4. Password Change\n5. View details\n6. Exit\n");
      scanf("%d", &choice);

      if(choice ==1 || choice == 2 || choice == 4) req.req_id = UPDATE_REQUEST;
      else req.req_id = VIEW_REQUEST;


      write(sd, &req, sizeof(req));
      read(sd, &req, sizeof(req));

      if(req.status != SUCCESS){
        printf("Failure - Data not accessible\n");
        return 0;
      }

      read(sd, &acc, sizeof(acc));

      if(choice == 1){
        printf("Enter the amount to deposit\n");
        int amt;
        scanf("%d", &amt);
        acc.bal+=amt;
      }
      else if(choice == 2){
        printf("Enter the amount to withdraw\n");
        int amt;
        scanf("%d", &amt);
        if(acc.bal > amt) acc.bal-=amt;
        else{printf("Amount insufficient\n");}
      }
      else if(choice == 3){
        printf("Balance is %lld\n", acc.bal);
      }
      else if(choice == 4){
        printf("Enter new pin\n");
        scanf("%s", pin);
        for(int i=0;i<4;i++) acc.pin[i] = pin[i];
      }
      else if(choice == 5){
        printf("Account Number is %d.\nBalance is %lld.", acc.accno, acc.bal);
      }

      if(choice ==1 || choice == 2 || choice == 4) write(sd, &acc, sizeof(acc));

    } else printf("FAILURE - INVALID CREDENTIALS");
  }
  else if (option == 3){
    int id;
    char pin[4];
    printf("Enter ID\n");
    scanf("%d", &id);
    printf("Enter pin number\n");
    scanf("%s", pin);

    struct request req;
    req.req_id = ADMIN_REQUEST;
    write(sd, &req, sizeof(req));

    struct admin ad;
    ad.id = id;
    strcpy(ad.pin, pin);

    write(sd, &ad, sizeof(ad));
    read(sd, &req, sizeof(req));
    if(req.status == INVALID_CREDENTIALS){
      printf("Invalid credentials\n");
      close(sd);
      return 0;
    }

    printf("Authorisation successfull. Select:\n1. search\n2. add\n3. delete\n4. modify\n");
    scanf("%d", &option);

    if(option == 1){
      req.req_id = ADMIN_SEARCH_REQUEST;
      write(sd, &req, sizeof(req));
      struct account acc;
      printf("Enter account number to search.\n");
      scanf("%d", &acc.accno);
      write(sd, &acc, sizeof(acc));
      read(sd, &req, sizeof(req));
      if(req.req_id == FAILURE) printf("Failure");
      else{
        read(sd, &acc, sizeof(acc));
        printf("Account Number is %d.\nBalance is %lld.\npin is %s\n.", acc.accno, acc.bal, acc.pin);
      }
    }
    if(option == 2){
      req.req_id = ADMIN_ADD_REQUEST;
      write(sd, &req, sizeof(req));
      struct account acc;
      read(sd, &acc, sizeof(acc));
      printf("Account number is %d\n", acc.accno);
      printf("Enter balance\n");
      long long int bal;
      scanf("%lld", &bal);
      printf("Enter pin\n");
      char p[4];
      scanf("%s", p);
      strcpy(acc.pin, p);
      acc.bal = bal;
      write(sd, &acc, sizeof(acc));
    }
    if(option == 3){
      req.req_id = ADMIN_DELETE_REQUEST;
      write(sd, &req, sizeof(req));
      struct account acc;
      printf("Enter account number to search.\n");
      scanf("%d", &acc.accno);
      write(sd, &acc, sizeof(acc));
      read(sd, &req, sizeof(req));
      if(req.req_id == FAILURE) printf("Failure");
    }
    if(option == 4){
      req.req_id = ADMIN_MODIFY_REQUEST;
      write(sd, &req, sizeof(req));
      struct account acc;
      printf("Enter account number to search.\n");
      scanf("%d", &acc.accno);
      write(sd, &acc, sizeof(acc));
      read(sd, &req, sizeof(req));
      if(req.req_id == FAILURE) printf("Failure");
      else{
        read(sd, &acc, sizeof(acc));
        printf("Account Number is %d.\nBalance is %lld.\npin is %s\n.", acc.accno, acc.bal, acc.pin);
        printf("Enter balance to change. -1 to ignore\n");
        long long b;
        scanf("%lld", &b);
        if(b>0) acc.bal = acc.bal + b;
        printf("Enter pin to change. i to ignore\n");
        char p[4];
        scanf("%s", p);
        if(p[0]!='i') strcpy(acc.pin, p);
        write(sd, &acc, sizeof(acc));
      }
    }
  }
  else
  {
    printf("Please enter valid input\n");
  }
  

  close(sd);
}
