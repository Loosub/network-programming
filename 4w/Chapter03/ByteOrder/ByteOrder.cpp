#include "..\..\Common.h" //Common.h ���� �ȿ� �Ʒ� ��� ���� �������

#include <WinSock2.h> //����2 ���� ���
#include <WS2tcpip.h> //����2 Ȯ�� ���

#include <tchar.h> // _T(),...
#include <stdio.h> //printf(),...
#include <stdlib.h> //exit(), ...
#include <string.h> //strncpy(), ...

#pragma comment(lib, "ws2_32") //dll���� �ε�, ws2_32.

int main(int argc, char *argv[])
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	u_short x1 = 0x1234;
	u_long  y1 = 0x12345678;
	u_short x2;
	u_long  y2;

	// ȣ��Ʈ ����Ʈ -> ��Ʈ��ũ ����Ʈ
	printf("[ȣ��Ʈ ����Ʈ -> ��Ʈ��ũ ����Ʈ]\n");
	printf("%#x -> %#x\n", x1, x2 = htons(x1));
	printf("%#x -> %#x\n", y1, y2 = htonl(y1));

	// ��Ʈ��ũ ����Ʈ -> ȣ��Ʈ ����Ʈ
	printf("\n[��Ʈ��ũ ����Ʈ -> ȣ��Ʈ ����Ʈ]\n");
	printf("%#x -> %#x\n", x2, ntohs(x2));
	printf("%#x -> %#x\n", y2, ntohl(y2));

	// �߸��� ��� ��
	printf("\n[�߸��� ��� ��]\n");
	printf("%#x -> %#x\n", x1, htonl(x1));

	// ���� ����
	WSACleanup();
	return 0;
}
