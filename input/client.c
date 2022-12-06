#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>

/* socket
 * connect
 * send/recv
 */

#define SERVER_PORT 8888

int main(int argc, char **argv) {
  int iSocketClient;
  struct sockaddr_in tSocketServerAddr;

  int iRet;
  unsigned char ucSendBuf[1000];
  int iSendLen;
  int iAddrLen;

  if (argc != 3) {
    printf("Usage:\n");
    printf("%s <server_ip> <str>\n", argv[0]);
    return -1;
  }

  iSocketClient = socket(AF_INET, SOCK_DGRAM, 0);
  tSocketServerAddr.sin_family = AF_INET;
  tSocketServerAddr.sin_port = htons(SERVER_PORT); /* host to net, short */

  if (0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr)) {
    printf("invalid server_ip\n");
    return -1;
  }
  memset(tSocketServerAddr.sin_zero, 0, 8);

#if 0
	iRet = connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));	
	if (-1 == iRet)
	{
		printf("connect error!\n");
		return -1;
	}
#endif

  iAddrLen = sizeof(struct sockaddr);
  iSendLen = sendto(iSocketClient, argv[2], strlen(argv[2]), 0,
                    (const struct sockaddr *)&tSocketServerAddr, iAddrLen);

  close(iSocketClient);

  return 0;
}
