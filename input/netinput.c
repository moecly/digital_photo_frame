#include "netinet/in.h"
#include "sys/socket.h"
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <input_manger.h>

#define SERVER_PORT 8888
#define BACKLOG 10
#define BUFFER_SIZE 1024

static int socket_server;
/*
 * receive data.
 */
static int net_get_input_event(input_event *idev) {
  char buf[BUFFER_SIZE];
  struct sockaddr_in sock_addr;
  struct timeval tv;
  unsigned int addr_len = sizeof(struct sockaddr_in);
  int ret;

  ret = recvfrom(socket_server, buf, BUFFER_SIZE - 1, 0,
                 (struct sockaddr *)&sock_addr, &addr_len);
  if (ret <= 0) {
    printf("network recv err\n");
    return -1;
  }

  /*
   * get data
   */
  gettimeofday(&tv, NULL);
  buf[ret] = '\0';
  idev->time = tv;
  idev->type = INPUT_TYPE_NET;
  strncpy(idev->str, buf, BUFFER_SIZE);

  return 0;
}

/*
 * init net device to server.
 */
static int net_device_init(void) {
  struct sockaddr_in socket_server_addr;
  int ret;

  /*
   * udp mode.
   */
  socket_server = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_server == -1) {
    printf("socket err\n");
    return -1;
  }

  /*
   * socket server.
   */
  socket_server_addr.sin_family = AF_INET;
  socket_server_addr.sin_port = htons(SERVER_PORT);
  socket_server_addr.sin_addr.s_addr = INADDR_ANY;
  memset(socket_server_addr.sin_zero, 0, sizeof(unsigned char));

  /*
   * bind.
   */
  ret = bind(socket_server, (struct sockaddr *)&socket_server_addr,
             sizeof(struct sockaddr_in));
  if (ret == -1) {
    printf("bind err\n");
    return -1;
  }

  return 0;
}

/*
 * close network device fd.
 */
static void net_device_exit(void) { close(socket_server); }

/*
 * network input.
 */
static input_device net_dev = {
    .name = "network_transfer",
    .device_init = net_device_init,
    .device_exit = net_device_exit,
    .get_input_event = net_get_input_event,
};

/*
 * register device.
 */
void net_device_register(void) { register_input_device(&net_dev); }

#if 0

/*
 * test.
 */
int main(int argc, char **argv) {
  input_event ievt;
  int ret;

  net_dev.device_init();
  for (;;) {
    ret = net_dev.get_input_event(&ievt);
    if (ret) {
      printf("get input event err\n");
      return -1;
    }

    printf("type: %d\n", ievt.type);
    printf("str: %s\n", ievt.str);
    printf("time: %d\n", (int)ievt.time.tv_sec);
  }

  return 0;
}

#endif