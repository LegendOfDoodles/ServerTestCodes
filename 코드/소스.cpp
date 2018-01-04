#include "winsock2.h"

void main(void)
{
	WSADATA wsaData;

	SOCKET ListeningSocket;
	SOCKET NewConnection;
	SOCKADDR_IN ServerAddr;
	SOCKADDR_IN ClientAddr;
	int port = 5150;
	//윈속 버전 2.2 초기화
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//클라이언트 요청을 받을 소켓 생성
	ListeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//SOCKADDR_IN 구조체 설정, 포트 번호 5150을 쓰는 모든 인터페이스의 연결 요청에 바인드 시키기 위해서
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(port);
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//리스닝 소켓 바인드
	bind(ListeningSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr));

	//바인드 시킨 소켓을 리스닝 상태에 돌입 , 5명의 사용자까지 가능
	listen(ListeningSocket, 5);

	//요청이 도착하면 받음
	NewConnection = accept(ListeningSocket, (SOCKADDR *)&ClientAddr, &ClientAddrLen);

	//클로징
	closesocket(NewConnection);
	closesocket(ListeningSocket);

	//모든 과정이 끝나면 WSACleanup 호출
	WSACleanup();
}


	