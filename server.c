#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"


int requestHandler(int nsd){

  int sd = nsd;
  struct request req;
  read(sd, &req, sizeof(req));

  if(req.req_id == JOINT_ACCOUNT_REQUEST){
    struct account acc;
    read(sd, &acc, sizeof(acc));
    int offset = readAccount(&acc);

    req.status = offset;
    write(sd, &req, sizeof(req));
    if(offset<0){
      close(sd);
      return 0;
    }

    read(sd, &req, sizeof(req));

    int fd = open("singleAccounts.txt", O_RDWR);
    int cur_lock = get_lock(fd, offset, sizeof(struct account));

    req.status = SUCCESS;
    if(req.req_id == UPDATE_REQUEST){
      if(apply_read_lock(fd, offset, sizeof(struct account)) != LOCK_SUCCESS) req.status = FAILURE;
    }
    else if(req.req_id == VIEW_REQUEST){
      if(apply_write_lock(fd, offset, sizeof(struct account)) != LOCK_SUCCESS) req.status = FAILURE;
    }

    write(sd, &req, sizeof(req));
    if(req.status == SUCCESS){
      lseek(fd, offset, SEEK_SET);
      read(fd, &acc, sizeof(acc));
      write(sd, &acc, sizeof(acc));

      if(req.req_id == UPDATE_REQUEST){
        read(sd, &acc, sizeof(acc));
        lseek(fd, -(sizeof(acc)), SEEK_CUR);
        write(fd, &acc, sizeof(acc));
      }
    }

    remove_lock(fd, offset, sizeof(struct account));

    close(fd);
    close(sd);

  }
  else if(req.req_id == SINGLE_ACCOUNT_REQUEST){

    struct account acc;
    read(sd, &acc, sizeof(acc));
    int offset = readAccount(&acc);
    req.status = offset;

    int fd = open("singleAccounts.txt", O_RDWR);
    //struct flock lock;
    // if(offset>=0){
    //   lock.l_type = F_WRLCK;
    //   lock.l_whence = SEEK_SET;
    //   lock.l_start = offset;
    //   lock.l_len = sizeof(struct account);
    //   lock.l_pid = getpid();
    //
    //   int ret = fcntl(fd, F_SETLKW, &lock);
    //   ret = fcntl(fd, F_SETLKW, &lock);
    //   //ret = fcntl(fd, F_SETLKW, &lock);
    //   printf("ret %d\n", ret);
    // }
    apply_write_lock(fd, offset, sizeof(struct account));
    write(sd, &req, sizeof(req));
    if(offset<0){
      close(sd);
      return 0;
    }

    read(sd, &req, sizeof(req));

    req.status = SUCCESS;

    write(sd, &req, sizeof(req));
    if(req.status == SUCCESS){
      lseek(fd, offset, SEEK_SET);
      read(fd, &acc, sizeof(acc));
      write(sd, &acc, sizeof(acc));

      if(req.req_id == UPDATE_REQUEST){
        read(sd, &acc, sizeof(acc));
        lseek(fd, -(sizeof(acc)), SEEK_CUR);
        write(fd, &acc, sizeof(acc));
      }
    }

    remove_lock(fd, offset, sizeof(struct account));
    // lock.l_type = F_UNLCK;
    // lock.l_whence = SEEK_SET;
    // lock.l_start = offset;
    // lock.l_len = sizeof(struct account);
    // lock.l_pid = getpid();
    //
    // fcntl(fd, F_SETLK, &lock);

    close(fd);
    close(sd);

  }
  else if(req.req_id == ADMIN_REQUEST){
    struct admin ad;
    read(sd, &ad, sizeof(ad));
    int ret = readAdmin(&ad);
    if(ret == ACCESS_SUCCESS) req.status = SUCCESS;
    else req.status = INVALID_CREDENTIALS;

    write(sd, &req, sizeof(req));

    if(req.status == SUCCESS){
      read(sd, &req, sizeof(req));

      if(req.req_id == ADMIN_SEARCH_REQUEST){
        struct account acc;
        read(sd, &acc, sizeof(acc));
        int offset = readAccount(&acc);

        if(offset<0){
          req.status = FAILURE;
          write(sd, &req, sizeof(req));
          close(sd);
          return 0;
        }

        int fd = open("singleAccounts.txt", O_RDONLY);
        // int cur_lock = get_lock(fd, offset, sizeof(struct account));
        //
        // if(cur_lock == READ_LOCK) req.status = FAILURE;
        // else req.status = SUCCESS;

        write(sd, &req, sizeof(req));
        if(req.status == SUCCESS) write(sd, &acc, sizeof(acc));
        close(fd);

      }
      else if(req.req_id == ADMIN_MODIFY_REQUEST){
        struct account acc;
        read(sd, &acc, sizeof(acc));
        int offset = readAccount(&acc);

        if(offset<0){
          req.status = FAILURE;
          write(sd, &req, sizeof(req));
          close(sd);
          return 0;
        }

        int fd = open("singleAccounts.txt", O_RDWR);
        // int cur_lock = get_lock(fd, offset, sizeof(struct account));
        //
        // if(cur_lock != NO_LOCK) req.status = FAILURE;
        // else req.status = SUCCESS;

        write(sd, &req, sizeof(req));
        if(req.status == SUCCESS){
        //  apply_read_lock(fd, offset, sizeof(struct account));
          write(sd, &acc, sizeof(acc));
          read(sd, &acc, sizeof(acc));
          lseek(fd, -(sizeof(acc)), SEEK_CUR);
          if(write(fd, &acc, sizeof(acc))) req.status = SUCCESS;
          else req.status = FAILURE;
          //remove_lock(fd, offset, sizeof(acc));
          close(fd);
        }
      }
      else if(req.req_id == ADMIN_DELETE_REQUEST){
        struct account acc;
        int offset = readAccount(&acc);

        if(offset<0){
          req.status = FAILURE;
          write(sd, &req, sizeof(req));
          close(sd);
          return 0;
        }

        int fd = open("singleAccounts.txt", O_RDWR);
        int cur_lock = get_lock(fd, offset, sizeof(struct account));

        // if(cur_lock != NO_LOCK) req.status = FAILURE;
        // else req.status = SUCCESS;

      //  apply_read_lock(fd, offset, sizeof(struct account));
        write(sd, &req, sizeof(req));
        acc.status = CLOSED;
        lseek(fd, offset, SEEK_SET);
        if(write(fd, &acc, sizeof(acc))) req.status = SUCCESS;
        else req.status = FAILURE;
        //remove_lock(fd, offset, sizeof(acc));
        close(fd);
      }
      else if(req.req_id == ADMIN_ADD_REQUEST){
        int fd = open("singleAccounts.txt", O_RDWR);
        int offset = lseek(fd, 0, SEEK_END);
        struct account acc;
        acc.accno = ad.cur_account_number + 1;
        acc.status = ACTIVE;
        write(sd, &acc, sizeof(acc));
        read(sd, &acc, sizeof(acc));
        write(fd, &acc, sizeof(acc));
        incrementAccAdmin(ad);
        close(fd);
      }
    }

    close(sd);
  }

}

int main(){

  struct sockaddr_in serv, cli;
  pthread_t threads;
  int sd = socket (AF_INET, SOCK_STREAM, 0);
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = INADDR_ANY;

  serv.sin_port = htons(5555);
  int servlen = sizeof(serv), clinlen = sizeof(cli);
  bind (sd, (struct sockaddr *) &serv, servlen);
  listen (sd, 6);
  write(1,"Waiting for access from someone\n",33);

  while (1) {
    int nsd = accept (sd, (struct sockaddr *) &cli, &clinlen);
    if (!fork()) {
    write(1,"Connected to the client\n",26);
      close(sd);
      requestHandler(nsd);
      exit(0);
    } else{ 
    
      close(nsd);}
  }

}
