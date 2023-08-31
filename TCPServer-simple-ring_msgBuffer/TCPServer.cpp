#include "..\..\Common.h"

#define SERVERPORT 9000
#define BUFSIZE    50

typedef struct {
	unsigned char flag; //링버퍼 상태
	char		  buf[1024];
} myBuffer;

char buf2[10][1024] = { {'\0',}, };

int main(int argc, char *argv[])
{
	int retval;
	bool is_stx = false; //stx를 찾았는지 못찾았는지
	
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	
	int addrlen; //데이터 길이
	char buf[BUFSIZE + 1]; //임시저장 버퍼
	char buff[BUFSIZE + 1]; //출력해야하는 메세지 버퍼

	static char sr_idx = 0; //링버퍼 인덱스를 알려주는 변수
	myBuffer simple_ring[10] = { 0, };

	while (1) {

		// accept() 연결 기다리기
		addrlen = sizeof(clientaddr);
		printf("before accept()");
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) { 
			err_display("accept()");
			break;
		}
		printf(">> after accept()");

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1) {
			
			// 데이터 받기 - 임시버퍼에 넣기
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 메세지 버퍼에 데이터 넣기
			for (int i = 0,i2 = 0 ; i < retval ; i++) { //메시지 데이터 길이만큼
				
				if (strncmp(buf + i, "GET", 3) == 0) //(시작주소값, 비교 문자열, 몇개나 비교 할건지), 틀리면:1, 맞으면:0
				{
					is_stx = true; //맞으면
					//continue; //틀리면 위로 올라감
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
					else if (simple_ring[sr_idx].flag == 0x01) { //상태가 1이면
						i2 = strlen(simple_ring[sr_idx].buf); //데이터 길이 넣기
					}

					// Real recving and buffering in MSG(myBuffer) 
					// Find ETX condition           

					if (strncmp(buf+i, "\r\n\r\n", 4) == 0) { //EXT \r\n\r\n
					//if (buf[i] == 0x09) { //EXT 찾음(0x09)
						//simple_ring[sr_idx].buf[i2] = '\n'; //링버퍼 끝에 뉴라인 넣기
						simple_ring[sr_idx].buf[i2++] = buf[i++]; 
						simple_ring[sr_idx].buf[i2] = '\0';
						simple_ring[sr_idx].buf[i2++] = buf[i++]; 
						simple_ring[sr_idx].buf[i2] = '\0';
						simple_ring[sr_idx].buf[i2++] = buf[i++];
						simple_ring[sr_idx].buf[i2] = '\0';
						simple_ring[sr_idx].buf[i2++] = buf[i]; //마지막은 i
						simple_ring[sr_idx].buf[i2] = '\0';

						simple_ring[sr_idx].flag = 0x03; //끝내기
						i2 = 0;

						// Control simple Ring index
						if (sr_idx < 8)sr_idx++; //다음 링버퍼 배열로 넘기기
						else sr_idx = 0;

						is_stx = false;
						break; //EXT 찾으면 for문 끝내기
					}
					else {
						simple_ring[sr_idx].buf[i2++] = buf[i]; //끝이 아니면 데이터 넣기
						simple_ring[sr_idx].buf[i2] = '\0';
					}
				}
			}

			// 받은 데이터 출력
			for (int k = 0; k<10  ; k++) {
				if (simple_ring[k].flag == 0x03) { //ending
					strncpy(buff, simple_ring[k].buf, strlen(simple_ring[k].buf)+1);
					simple_ring[k].flag = 0x00;

					printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buff);					
				}
			}
			
			//// 데이터 보내기
			//retval = send(client_sock, buf, retval, 0);
			//if (retval == SOCKET_ERROR) {
			//	err_display("send()");
			//	break;
			//}
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
