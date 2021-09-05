#include "File.h"
#define SINGLE_ACCOUNT_REQUEST 11
#define JOINT_ACCOUNT_REQUEST 12
#define ADMIN_REQUEST 13
#define WRONG_PIN 14
#define INVALID_CREDENTIALS 15
#define UPDATE_REQUEST 16
#define VIEW_REQUEST 17
#define SUCCESS 18
#define FAILURE 19

#define ADMIN_SEARCH_REQUEST 1
#define ADMIN_ADD_REQUEST 2
#define ADMIN_DELETE_REQUEST 3
#define ADMIN_MODIFY_REQUEST 4
struct request {

  int req_id;
  int status;

};



