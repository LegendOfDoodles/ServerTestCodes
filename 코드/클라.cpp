
#include "winsock2.h"

void main(void)
{
	WSADATA wsaData;
	SOCKET s;
	SOCKADDR_IN ServerAddr;
	int port = 5150;
	
	//보내진 데이터 수 넣을 변수
	int ret = 0;

	//윈속 2.2 초기화
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//클라이언트 연결할 새로운 소켓 생성
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//서버 ip주소를 136.149.3.29 라고 가정
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(port);
	ServerAddr.sin_addr.s_addr = inet_addr("136.149.3.29");

	//소켓과 서버 연결
	connect(s, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr));

	//데이터 전송, 수신
	char sendbuff[2048];
	int nBytes = 2048, nLeft, idx;
	
	//sendbuff 채우기 

	//s가 연결된 스트림 소켓이라고 가정
	nLeft = nBytes;
	idx = 0;

	while (nLeft > 0) {
		ret = send(s, &sendbuff[idx], nLeft, 0);
		if (ret == SOCKET_ERROR) {
			//전송에 에러 발생
		}
		nLeft -= ret;
		idx += ret;
	}

	//다쓴 소켓 클로징
	closesocket(s);

	//연결이 모두 끝난 뒤 WSACleanup 호출
	WSACleanup();
}

