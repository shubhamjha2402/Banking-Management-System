#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#define ACCESS_SUCCESS 0
#define ACCESS_FAILURE -1
#define RECORD_NOT_FOUND -2
#define LOCK_SUCCESS -3
#define LOCK_FAILURE -4
#define READ_LOCK -5
#define WRITE_LOCK -6
#define NO_LOCK -7
#define CLOSED -11
#define ACTIVE -12
struct account{

  long long int bal;
  int accno;
  char pin[4];
  int status;

};

struct admin{

  int id;
  char pin[4];
  int num_of_records;
  int cur_account_number;

};

int readAccount(struct account *acc);
int readSingleAccount(struct account *acc);
int readAdmin(struct admin *ad);
int incrementAccAdmin(struct admin ad);
int get_lock(int fd, int off_set, int size);
int apply_read_lock(int fd, int off_set, int size);
int apply_write_lock(int fd, int off_set, int size);
int remove_lock(int fd, int off_set, int size);

int readAccount(struct account *acc){
  struct account *cur = (struct account*) malloc(sizeof(struct account));
  int fd = open("singleAccounts.txt", O_RDWR);

  while(read(fd, cur, sizeof(struct account))){
    if(cur->accno == acc->accno){
      if(strncmp(acc->pin, cur->pin, 4) != 0) {
        close(fd);
        return ACCESS_FAILURE;
      }
      *acc = *cur;
      int offset = lseek(fd, -(sizeof(struct account)), SEEK_CUR);
      close(fd);
      return offset;
    }
  }

  close(fd);
  return RECORD_NOT_FOUND;
}

int readSingleAccount(struct account *acc){
  struct account *cur = (struct account*) malloc(sizeof(struct account));
  int fd = open("singleAccounts.txt", O_RDONLY);

  while(read(fd, cur, sizeof(struct account))){
    if(cur->accno == acc->accno){
      if(strncmp(acc->pin, cur->pin, 4) != 0) {
        close(fd);
        return ACCESS_FAILURE;
      }
      *acc = *cur;
      int offset = lseek(fd, -(sizeof(struct account)), SEEK_CUR);
      if(apply_write_lock(fd, offset, sizeof(struct account)) != LOCK_SUCCESS){
        printf("hiii\n");
        close(fd); return LOCK_FAILURE;
      }
      close(fd);
      return offset;
    }
  }

  close(fd);
  return RECORD_NOT_FOUND;
}

int admin_request(struct account *acc, int type){
  struct account *cur = (struct account*) malloc(sizeof(struct account));
  int fd = open("singleAccounts.txt", O_RDONLY);

  while(read(fd, cur, sizeof(struct account))){
    if(cur->accno == acc->accno){
      *acc = *cur;
      close(fd);
      return ACCESS_SUCCESS;
    }
  }

  close(fd);
  return RECORD_NOT_FOUND;
}

int readAdmin(struct admin *ad){
  struct admin *cur = (struct admin*) malloc(sizeof(struct admin));
  int fd = open("admin.txt", O_RDONLY);
    read(fd, cur, sizeof(struct admin));
    if(cur->id == ad->id) {
      //*ad = *cur;
      close(fd);
      if(strncmp(ad->pin, cur->pin, 4) == 0) {
        *ad = *cur; return ACCESS_SUCCESS;
      }
      else return ACCESS_FAILURE;
    }

  return RECORD_NOT_FOUND;
}

int get_lock(int fd, int off_set, int size){
  struct flock lock;

  lock.l_whence = SEEK_SET;
  lock.l_start = off_set;
  lock.l_len = size;

  fcntl(fd, F_GETLK, &lock);
  if(lock.l_type == F_WRLCK) return WRITE_LOCK;
  else if(lock.l_type == F_RDLCK) return READ_LOCK;
  else return NO_LOCK;
}

int incrementAccAdmin(struct admin ad){
  struct admin *cur = (struct admin*) malloc(sizeof(struct admin));
  int fd = open("admin.txt", O_RDWR);

  ad.cur_account_number = ad.cur_account_number + 1;
  lseek(fd, 0, SEEK_SET);
  write(fd, &ad, sizeof(struct admin));
  return ACCESS_SUCCESS;
  return RECORD_NOT_FOUND;
}

int apply_read_lock(int fd, int off_set, int size){
  struct flock lock;
  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = off_set;
  lock.l_len = size;
  lock.l_pid = getpid();
  fcntl(fd, F_SETLKW, &lock);
  
  return LOCK_SUCCESS;
}
int apply_write_lock(int fd, int off_set, int size){
  struct flock lock;
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = off_set;
  lock.l_len = size;
  lock.l_pid = getpid();

  fcntl(fd, F_SETLKW, &lock);
  //check
  // if(apply_write_lock(fd, off_set, size) == LOCK_FAILURE) printf("check success %d\n", getpid());
  // else printf("check fail\n");
  //if(apply_read_lock(fd, off_set, size) == LOCK_FAILURE) printf("check success2 %d\n", getpid());
  return LOCK_SUCCESS;
}

int remove_lock(int fd, int off_set, int size){
  struct flock lock;
  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = off_set;
  lock.l_len = size;
  lock.l_pid = getpid();
  fcntl(fd, F_SETLK, &lock);
  return LOCK_SUCCESS;
}


