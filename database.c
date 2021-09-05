#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "File.h"

int main(){
  struct admin ad;

  int fd = open("admin.txt", O_RDWR|O_TRUNC);
  ad.id = 123456;
  strcpy(ad.pin, "1111");
  ad.cur_account_number = 0;
  write(fd, &ad, sizeof(ad));
  printf("%d\n", ad.cur_account_number);
  close(fd);
}
