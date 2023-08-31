#include "..\..\Common.h"

#define SERVERPORT 9000
#define BUFSIZE    50

typedef struct {
	unsigned char flag; //������ ����
	char		  buf[1024];
} myBuffer;

char buf2[10][1024] = { {'\0',}, };

int main(int argc, char *argv[])
{
	int retval;
	bool is_stx = false; //stx�� ã�Ҵ��� ��ã�Ҵ���
	
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	// bind()
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	
	int addrlen; //������ ����
	char buf[BUFSIZE + 1]; //�ӽ����� ����
	char buff[BUFSIZE + 1]; //����ؾ��ϴ� �޼��� ����

	static char sr_idx = 0; //������ �ε����� �˷��ִ� ����
	myBuffer simple_ring[10] = { 0, };

	while (1) {

		// accept() ���� ��ٸ���
		addrlen = sizeof(clientaddr);
		printf("before accept()");
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) { 
			err_display("accept()");
			break;
		}
		printf(">> after accept()");

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while (1) {
			
			// ������ �ޱ� - �ӽù��ۿ� �ֱ�
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// �޼��� ���ۿ� ������ �ֱ�
			for (int i = 0,i2 = 0 ; i < retval ; i++) { //�޽��� ������ ���̸�ŭ
				
				if (strncmp(buf + i, "GET", 3) == 0) //(�����ּҰ�, �� ���ڿ�, ��� �� �Ұ���), Ʋ����:1, ������:0
				{
					is_stx = true; //������
					//continue; //Ʋ���� ���� �ö�
				}
				if (strncmp(buf + i, "PUT", 3) == 0)
				{
					is_stx = true;
					//continue;
				}
				if (strncmp(buf + i, "POST", 4) == 0)
				{
					is_stx = true;
					//continue;
				}

				if (is_stx == true) {
					// flag 0x00 nothing
					// flag 0x01 recving
					// flag 0x03 ending (completed msg)
					// flag 0x04 ending with err
					// selecting myBuffer
					if (simple_ring[sr_idx].flag == 0x00) {
						simple_ring[sr_idx].flag = 0x01;
						i2 = 0;
					}
					else if (simple_ring[sr_idx].flag == 0x01) { //���°� 1�̸�
						i2 = strlen(simple_ring[sr_idx].buf); //������ ���� �ֱ�
					}

					// Real recving and buffering in MSG(myBuffer) 
					// Find ETX condition           

					if (strncmp(buf+i, "\r\n\r\n", 4) == 0) { //EXT \r\n\r\n
					//if (buf[i] == 0x09) { //EXT ã��(0x09)
						//simple_ring[sr_idx].buf[i2] = '\n'; //������ ���� ������ �ֱ�
						simple_ring[sr_idx].buf[i2++] = buf[i++]; 
						simple_ring[sr_idx].buf[i2] = '\0';
						simple_ring[sr_idx].buf[i2++] = buf[i++]; 
						simple_ring[sr_idx].buf[i2] = '\0';
						simple_ring[sr_idx].buf[i2++] = buf[i++];
						simple_ring[sr_idx].buf[i2] = '\0';
						simple_ring[sr_idx].buf[i2++] = buf[i]; //�������� i
						simple_ring[sr_idx].buf[i2] = '\0';

						simple_ring[sr_idx].flag = 0x03; //������
						i2 = 0;

						// Control simple Ring index
						if (sr_idx < 8)sr_idx++; //���� ������ �迭�� �ѱ��
						else sr_idx = 0;

						is_stx = false;
						break; //EXT ã���� for�� ������
					}
					else {
						simple_ring[sr_idx].buf[i2++] = buf[i]; //���� �ƴϸ� ������ �ֱ�
						simple_ring[sr_idx].buf[i2] = '\0';
					}
				}
			}

			// ���� ������ ���
			for (int k = 0; k<10  ; k++) {
				if (simple_ring[k].flag == 0x03) { //ending
					strncpy(buff, simple_ring[k].buf, strlen(simple_ring[k].buf)+1);
					simple_ring[k].flag = 0x00;

					printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buff);					
				}
			}
			
			//// ������ ������
			//retval = send(client_sock, buf, retval, 0);
			//if (retval == SOCKET_ERROR) {
			//	err_display("send()");
			//	break;
			//}
		}

		// ���� �ݱ�
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}
