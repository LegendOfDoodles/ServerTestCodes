// TestClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
using namespace std;


// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void ErrorFunction(int value, int type) {
	if (type == 0) {
		if (value == SOCKET_ERROR) { err_quit("recv()"); }
		else if (!value) return;
	}
	else if (type == 1) {
		if (value == SOCKET_ERROR) { err_quit("send()"); }
		else if (!value) return;
	}
}


int main()
{

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1; //if fail, return

#pragma region [Make Socket]
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
#pragma endregion
	cout << "--------------------------------------------------- " <<endl;
	cout << "LegendOfDoodles				 // TESTCLIENT		 "<<endl;
	cout << "                                ver 0.0		     " <<endl;
	cout << "--------------------------------------------------  " <<endl;

#pragma region [Connect Socket]
	static SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(MY_SERVER_PORT);
	int retVal = connect(sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (retVal == SOCKET_ERROR) err_quit("bind()");
#pragma endregion

#pragma region [Debuging Connect and bind]

	std::cout << "   테스트 클라이언트가 서버에 접속하였습니다.     " << std::endl;
	std::cout << "   PORT NUMBER : " << serverAddr.sin_port << std::endl;
	std::cout << "--------------------------------------------------  " << std::endl;

#pragma endregion

#pragma region [TestFunction]
	//recv(sock, )

	while (7) {
		int inputOption{};

		std::cout << "테스트를 시작합니다. 1은 Send, 2는 Recv, 3은 Break  --->  ";

		std::cin >> inputOption;
		if (inputOption == 3) break;

		if (inputOption == 1) {
			std::cout << "  Testing Client Send - Server Recv, Choice Protocol " << std::endl;
			int inputProtocol{};
			std::cout << "  원하시는 ProtocolNumber or Datasize를 입력해주세요 ---> ";
			std::cin >> inputProtocol;

			if (inputProtocol >= 200)
				send(sock, (char*)&inputProtocol, sizeof(inputProtocol), 0);
			else if (inputProtocol < 100) {
				std::vector<char> sendBuffer;
				for (int i = 0; i < inputProtocol; i++) {
					sendBuffer.emplace_back('a');
				}

				send(sock, (char*)&sendBuffer, sizeof(inputProtocol), 0);
			}

			std::cout << "  정상적으로 Protocol Number를 전송했습니다. " << std::endl;
		}
		else if (inputOption == 2) {
			int inputProtocolNumber{};
			std::cout << "  Testing Client Recv - Server Send, Choice Protocol ";
			std::cout << "  OnReceving \n";

			int recvProtocolNumber{};
			recv(sock, (char*)&recvProtocolNumber, sizeof(recvProtocolNumber), 0);

			

			if (recvProtocolNumber == SC_PERMIT_MAKE_ROOM) {
				CS_MsgDemandMakeRoom MakeRoom;
				recv(sock, (char*)&MakeRoom, sizeof(MakeRoom), 0);
				cout << "전송받은 정보는 Client[" << MakeRoom.Character_id << "] Permittied to Make Room\n";
			}
		
		}
	}

#pragma endregion

#pragma region [ GoodBye Server! ]
	std::cout << "---Master- 서버를 종료하시겠습니까? --> ";
	char buf{};
	std::cin >> buf;


	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();

	return 0;
#pragma endregion
}