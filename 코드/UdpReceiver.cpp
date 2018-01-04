#include <winsock2.h>

void main(void)
{
	WSADATA wsaData;
	SOCKET ReceivingSocket;

	SOCKADDR_IN ReceiverAddr;
	int port = 5150;

	char ReceiveBuf[1024];
	int BufLength = 1024;
	SOCKADDR_IN SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);

	//윈속 버전 2.2 초기화
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//받을 소켓 생성
	ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//받을 소켓 주소 바인드
	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(port);
	ReceiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(ReceivingSocket, (SOCKADDR *)&SenderAddr, sizeof(SenderAddr));

	//받는 작업 처리
	recvfrom(ReceivingSocket, ReceiveBuf, BufLength, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);

	//받는게 완료되면 소켓을 닫는다.
	closesocket(ReceivingSocket);

	WSACleanup();
}