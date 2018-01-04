#include <winsock2.h>

void main(void)
{
	WSADATA wsaData;
	SOCKET SendingSocket;
	SOCKADDR_IN ReceiveAddr;
	int port = 5150;
	char SendBuf[1024];
	int BufLength = 1024;

	//윈속 버전 2.2 초기화
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//받을 소켓 생성
	SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//클라이언트라 가정하고, 소켓의 주소값 설정, ip주소는 136.149.3.29 라고 가정
	ReceiveAddr.sin_family = AF_INET;
	ReceiveAddr.sin_port = htons(port);
	ReceiveAddr.sin_addr.s_addr = inet_addr("136.149.3.29");

	//리시버 소켓에 자료 보냄
	sendto(SendingSocket, SendBuf, BufLength, 0, (SOCKADDR *)&ReceiveAddr, sizeof(ReceiveAddr));

	closesocket(SendingSocket);

	WSACleanup();
}