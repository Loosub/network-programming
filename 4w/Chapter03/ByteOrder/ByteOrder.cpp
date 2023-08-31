#include "..\..\Common.h" //Common.h 파일 안에 아래 헤더 파일 들어있음

#include <WinSock2.h> //윈속2 메인 헤더
#include <WS2tcpip.h> //윈속2 확장 헤더

#include <tchar.h> // _T(),...
#include <stdio.h> //printf(),...
#include <stdlib.h> //exit(), ...
#include <string.h> //strncpy(), ...

#pragma comment(lib, "ws2_32") //dll파일 로딩, ws2_32.

int main(int argc, char *argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	u_short x1 = 0x1234;
	u_long  y1 = 0x12345678;
	u_short x2;
	u_long  y2;

	// 호스트 바이트 -> 네트워크 바이트
	printf("[호스트 바이트 -> 네트워크 바이트]\n");
	printf("%#x -> %#x\n", x1, x2 = htons(x1));
	printf("%#x -> %#x\n", y1, y2 = htonl(y1));

	// 네트워크 바이트 -> 호스트 바이트
	printf("\n[네트워크 바이트 -> 호스트 바이트]\n");
	printf("%#x -> %#x\n", x2, ntohs(x2));
	printf("%#x -> %#x\n", y2, ntohl(y2));

	// 잘못된 사용 예
	printf("\n[잘못된 사용 예]\n");
	printf("%#x -> %#x\n", x1, htonl(x1));

	// 윈속 종료
	WSACleanup();
	return 0;
}
