
#define WIN32_LEAN_AND_MEAN  
#define INITGUID
#pragma comment(lib, "ws2_32.lib")
  // include important windows stuff
#include <windows.h> 
#include <Winsock2.h>


//AWS에 이식할때 바꿔야할 부분
#include "..\..\..\..\Client\Frameworks\Frameworks\Code\07.Network\protocol.h"
//참조 헤더
#include <thread>
#include <vector>
#include <array>
#include <iostream>
#include <unordered_set>
#include <mutex>
using namespace std;

HANDLE gh_iocp;
struct EXOVER {
	WSAOVERLAPPED m_over;
	char m_iobuf[MAX_BUFF_SIZE];
	WSABUF m_wsabuf;
	bool is_recv;
};

class Client {
public:
	SOCKET m_s;
	bool m_isconnected;
	int m_x;
	int m_y;
	EXOVER m_rxover;
	int m_packet_size;  // 지금 조립하고 있는 패킷의 크기
	int	m_prev_packet_size; // 지난번 recv에서 완성되지 않아서 저장해 놓은 패킷의 앞부분의 크기
	char m_packet[MAX_PACKET_SIZE];
	unordered_set <int> m_viewlist;
	//set? vector? map? list?
	//view list has to insert, delete, search
	//set : logN, logN, logN  : more easier than map, map has body 
	//unordered_set : 1, 1, 1 : It's faster than set except look from first to end
	//vector : 1(push_back), N, N --> too big
	//map : logN, logN, logN
	//list: 1, 1, N : O(N) is too big to use....
	mutex m_mvl; //mutex for view list

	Client()
	{
		m_isconnected = false;
		m_x = 500;
		m_y = 2500;

		ZeroMemory(&m_rxover.m_over, sizeof(WSAOVERLAPPED));
		m_rxover.m_wsabuf.buf = m_rxover.m_iobuf;
		m_rxover.m_wsabuf.len = sizeof(m_rxover.m_wsabuf.buf);
		m_rxover.is_recv = true;
		m_prev_packet_size = 0;
	}
};

class Minion {
public:
	int m_hp;
	int m_id;
	Minion()
	{
		m_id = -1;
	}
};

array <Minion, NUM_OF_NPC> g_minions;
array <Client, MAX_USER> g_clients;
int g_MinionCounts = 0;
int g_ReuseMinion = -1;
void error_display(const char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
		std::cout << msg;
		std::wcout << L"  에러" << lpMsgBuf << std::endl;
		LocalFree(lpMsgBuf);
		while (true);
}

void ErrorDisplay(const char * location)
{
	error_display(location, WSAGetLastError());
}
//굳이 서버에서 안할듯 싶음
bool CanSee(int a, int b)
{
	int dist_x = g_clients[a].m_x - g_clients[b].m_x;
	int dist_y = g_clients[a].m_y - g_clients[b].m_y;
	int dist = dist_x*dist_x + dist_y*dist_y;
	//don't use sqrt() in SERVER! 
	return (VIEW_RADIUS * VIEW_RADIUS >= dist);
}

bool IsNPC(int id)
{
	return ((id >= NPC_START) && id < (NUM_OF_NPC));
}

void initialize()
{
	g_clients[0].m_x = 500;
	g_clients[0].m_y = 2500;

	g_clients[1].m_x = 600;
	g_clients[1].m_y = 2500;

	g_clients[2].m_x = 9700;
	g_clients[2].m_y = 2500;

	g_clients[3].m_x = 9800;
	g_clients[3].m_y = 2500;


	gh_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0); // 의미없는 파라메터, 마지막은 알아서 쓰레드를 만들어준다.
	std::wcout.imbue(std::locale("korean"));

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
}

void StartRecv(int id)
{
	unsigned long r_flag = 0;
	ZeroMemory(&g_clients[id].m_rxover.m_over, sizeof(WSAOVERLAPPED));
	int ret = WSARecv(g_clients[id].m_s, &g_clients[id].m_rxover.m_wsabuf, 1, 
		NULL, &r_flag, &g_clients[id].m_rxover.m_over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) error_display("Recv Error", err_no);
	}
}

void SendPacket(int id, void *ptr)
{
	char *packet = reinterpret_cast<char *>(ptr);
	EXOVER *s_over = new EXOVER;
	s_over->is_recv = false;
	memcpy(s_over->m_iobuf, packet, packet[0]);
	s_over->m_wsabuf.buf = s_over->m_iobuf;
	s_over->m_wsabuf.len = s_over->m_iobuf[0];
	ZeroMemory(&s_over->m_over, sizeof(WSAOVERLAPPED));
	int res = WSASend(g_clients[id].m_s, &s_over->m_wsabuf, 1, NULL, 0,
		&s_over->m_over, NULL);
	if (0 != res) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) error_display("Send Error! ", err_no);
	}
}
void SendPutObjectPacket(int client, int object)
{
	SC_Msg_Put_Character p;
	p.Character_id = object;
	p.size = sizeof(p);
	p.type = SC_PUT_PLAYER;
	p.x = g_clients[object].m_x;
	p.y = g_clients[object].m_y;
	SendPacket(client, &p);
}
void SendRemovePacket(int client, int object)
{
	SC_Msg_Remove_Character p;
	p.Character_id = object;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	SendPacket(client, &p);
}
void ProcessPacket(int id, char *packet)
{
	//클라로부터 오는 패킷 종류들
	CS_MsgChMove* MovePacket = reinterpret_cast<CS_MsgChMove*>(packet);
	CS_MsgChCollision* CollisionPacket = reinterpret_cast<CS_MsgChCollision*>(packet);
	CS_MsgMoDelete* DeleteMinionPacket = reinterpret_cast<CS_MsgMoDelete*>(packet);
	int x = 0;
	int y = 0;
	//서버에서 클라로 보내줘야할 패킷들
	SC_MsgMoCreate p;
	cout << packet[1] << endl;
	switch (MovePacket->type)
	{
	//이동하는 부분
	case CS_UP: if (y > 0) y-=10; break;
	case CS_DOWN: if (y < MAP_HEIGHT - 1) y+=10; break;
	case CS_LEFT: if (x > 0) x-=10; break;
	case CS_RIGHT: if (x < MAP_WIDTH - 1) x+=10; break;
	case CS_MOVE_PLAYER: 
	{
		
		g_clients[MovePacket->Character_id].m_x = MovePacket->x;
		g_clients[MovePacket->Character_id].m_y = MovePacket->y;
		x = MovePacket->x;
		y = MovePacket->y;
		cout << "Client[" << id << "] X is " << g_clients[MovePacket->Character_id].m_x << endl;
		cout << "Client[" << id << "] Y is " << g_clients[MovePacket->Character_id].m_y << endl;
		break;
	}
	/*case CS_COLLISION:
	{
		if (g_clients[CollisionPacket->Character_id].m_x != CollisionPacket->x || g_clients[CollisionPacket->Character_id].m_y != CollisionPacket->y) {
			g_clients[CollisionPacket->Character_id].m_x = CollisionPacket->x;
			g_clients[CollisionPacket->Character_id].m_y = CollisionPacket->y;
			cout << "Client[" << id << "] Collied at X = " << g_clients[CollisionPacket->Character_id].m_x << " Y = " << g_clients[CollisionPacket->Character_id].m_y << endl;

			break;
		}
		else {
			break;
		}
	}*/

	
	//재사용을 할 필요가 있을까? --> 적용했는데 잘 안될경우 그냥 필요한 수만큼 선언.
	case CS_PUT_MINION:
	{
		for (int i = 0; i < g_MinionCounts; ++i) {
			if (g_minions[i].m_id == 0) {
				g_ReuseMinion = i;
			}
		}
		//빈곳이 있어서 재사용해야한다.
		if (g_ReuseMinion != -1) {
			g_minions[g_ReuseMinion].m_id = NPC_START + g_MinionCounts;
			//Debugging
			cout << "Minion ID: [" << g_minions[g_ReuseMinion].m_id << "] Created\n";
			p.Monster_id = g_minions[g_ReuseMinion].m_id;
			g_MinionCounts++;
		}
		//빈곳이 없다.
		else {
			g_minions[g_MinionCounts].m_id = NPC_START + g_MinionCounts;
			//Debugging
			cout << "Minion ID: [" << g_minions[g_MinionCounts].m_id << "] Created\n";
			p.Monster_id = g_minions[g_MinionCounts].m_id;
			g_MinionCounts++;
		}
		p.size = sizeof(p);
		p.type = SC_PUT_MINION;
		for (int i = 0; i < MAX_USER; ++i) {
			if (g_clients[i].m_isconnected == true) SendPacket(i, &p);
		}
		break;
	}
	case CS_DELETE_MINION:
	{
		for (int i = 0; i < g_MinionCounts; ++i) {
			if (DeleteMinionPacket->Monster_id == g_minions[i].m_id) {
				g_minions[i].m_id = 0;
				//Debugging
				cout << "Minion ID: [" << g_minions[i].m_id << "] Deleted\n";
			}
		}
		break;
	}
	case CS_DAMAND_MAKE_ROOM:
	{
		cout << "fuck!\n" << endl;
		break;
	}
	default: 
		cout << "Unkown Packet Type from Client [" << id << "]\n";
		return;
	}



	SC_MsgChMove pos_packet;

	pos_packet.Character_id = id;
	pos_packet.size = sizeof(SC_MsgChMove);
	pos_packet.type = SC_MOVE_PLAYER;
	pos_packet.x = x;
	pos_packet.y = y;

	unordered_set<int> new_vl;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (i == id) continue;
		if (false == g_clients[i].m_isconnected) continue;
		if (true == CanSee(id, i)) new_vl.insert(i);
	}

	SendPacket(id, &pos_packet);


	//// new_vl에는 있는데 old_vl에 없는 경우
	//for (auto ob : new_vl) {
	//	g_clients[id].m_mvl.lock();
	//	if (0 == g_clients[id].m_viewlist.count(ob)) {
	//		g_clients[id].m_viewlist.insert(ob);
	//		g_clients[id].m_mvl.unlock();
	//		SendPutObjectPacket(id, ob);

	//		if (true == IsNPC(ob)) continue;
	//		g_clients[ob].m_mvl.lock();
	//		if (0 == g_clients[ob].m_viewlist.count(id)) {
	//			g_clients[ob].m_viewlist.insert(id);
	//			g_clients[ob].m_mvl.unlock();

	//			SendPutObjectPacket(ob, id);
	//		}
	//		else {
	//			g_clients[ob].m_mvl.unlock();
	//			SendPacket(ob, &pos_packet);
	//		}
	//	}
	//	else {
	//		// new_vl에도 있고 old_vl에도 있는 경우
	//		g_clients[id].m_mvl.unlock();
	//		if (true == IsNPC(ob)) continue;
	//		g_clients[ob].m_mvl.lock();
	//		if (0 != g_clients[ob].m_viewlist.count(id)) {
	//			g_clients[ob].m_mvl.unlock();
	//			SendPacket(ob, &pos_packet);
	//		}
	//		else {
	//			g_clients[ob].m_viewlist.insert(id);
	//			g_clients[ob].m_mvl.unlock();
	//			SendPutObjectPacket(ob, id);
	//		}
	//	}

	//}

	//// new_vl에는 없는데 old_vl에 있는 경우
	//vector <int> to_remove;
	//g_clients[id].m_mvl.lock();
	//unordered_set<int> vl_copy = g_clients[id].m_viewlist;
	//g_clients[id].m_mvl.unlock();
	//for (auto ob : vl_copy) {
	//	if (0 == new_vl.count(ob)) {
	//		to_remove.push_back(ob);

	//		if (true == IsNPC(ob)) continue;
	//		g_clients[ob].m_mvl.lock();
	//		if (0 != g_clients[ob].m_viewlist.count(id)) {
	//			g_clients[ob].m_viewlist.erase(id);
	//			g_clients[ob].m_mvl.unlock();
	//			SendRemovePacket(ob, id);
	//		}
	//		else {
	//			g_clients[ob].m_mvl.unlock();
	//		}
	//	}
	//}

	//g_clients[id].m_mvl.lock();
	//for (auto ob : to_remove) g_clients[id].m_viewlist.erase(ob);
	//g_clients[id].m_mvl.unlock();
	//for (auto ob : to_remove) {
	//	SendRemovePacket(id, ob);
	//}

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_isconnected) {
			if (id == i) continue;
				SendPacket(i, &pos_packet);
		}
	}
}

void DisconnectPlayer(int id)
{
	SC_Msg_Remove_Character p;
	p.Character_id = id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	for (int i = 0; i < MAX_USER; ++i) {
		if (false == g_clients[i].m_isconnected) continue;
		if (i == id) continue;
		if (true == IsNPC(i)) break;
		g_clients[i].m_mvl.lock();
		if (0 != g_clients[i].m_viewlist.count(id)) {
			g_clients[i].m_viewlist.erase(id);
			g_clients[i].m_mvl.unlock();
			SendPacket(i, &p);
		}
		else {
			g_clients[i].m_mvl.unlock();
		}
	}
	closesocket(g_clients[id].m_s);
	g_clients[id].m_mvl.lock();
	g_clients[id].m_viewlist.clear();
	g_clients[id].m_mvl.unlock();
	g_clients[id].m_isconnected = false;
}

void worker_thread()
{
	while (true)
	{
		unsigned long io_size;
		unsigned long long iocp_key; // 64 비트 integer , 우리가 64비트로 컴파일해서 64비트
		WSAOVERLAPPED *over;
		BOOL ret = GetQueuedCompletionStatus(gh_iocp, &io_size, &iocp_key, &over, INFINITE);
		int key = static_cast<int>(iocp_key);
		cout << "WT::Network I/O with Client [" << key << "]\n";
		if (FALSE == ret) {
			cout << "Error in GQCS\n";
			DisconnectPlayer(key);
			continue;
		}
		if (0 == io_size) {
			DisconnectPlayer(key);
			continue;
		}

		EXOVER *p_over = reinterpret_cast<EXOVER *>(over);
		if (true == p_over->is_recv ) {
			cout << "WT:Packet From Client [" << key << "]\n";
			int work_size = io_size;
			char *wptr = p_over->m_iobuf;
			while (0 < work_size) {
				int p_size;
				if (0 != g_clients[key].m_packet_size)
					p_size = g_clients[key].m_packet_size;
				else {
					p_size = wptr[0];
					g_clients[key].m_packet_size = p_size;
				}
				int need_size = p_size - g_clients[key].m_prev_packet_size;
				if (need_size <= work_size) {
					memcpy(g_clients[key].m_packet 
						+ g_clients[key].m_prev_packet_size, wptr, need_size);
					ProcessPacket(key, g_clients[key].m_packet);
					g_clients[key].m_prev_packet_size = 0;
					g_clients[key].m_packet_size = 0;
					work_size -= need_size;
					wptr += need_size;
				}
				else {
					memcpy(g_clients[key].m_packet + g_clients[key].m_prev_packet_size, wptr, work_size);
					g_clients[key].m_prev_packet_size += work_size;
					work_size = -work_size;
					wptr += work_size;
				}
			}
			StartRecv(key);
		}
		else {  // Send 후처리
			cout << "WT:A packet was sent to Client[" << key << "]\n";
			delete p_over;
		}
	}
}

void accept_thread()	//새로 접속해 오는 클라이언트를 IOCP로 넘기는 역할
{
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN bind_addr;
	ZeroMemory(&bind_addr, sizeof(SOCKADDR_IN));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(MY_SERVER_PORT);
	bind_addr.sin_addr.s_addr = INADDR_ANY;	// 0.0.0.0  아무대서나 오는 것을 다 받겠다.

	::bind(s, reinterpret_cast<sockaddr *>(&bind_addr), sizeof(bind_addr));
	listen(s, 1000);

	while (true)
	{
		SOCKADDR_IN c_addr;
		ZeroMemory(&c_addr, sizeof(SOCKADDR_IN));
		c_addr.sin_family = AF_INET;
		c_addr.sin_port = htons(MY_SERVER_PORT);
		c_addr.sin_addr.s_addr = INADDR_ANY;	// 0.0.0.0  아무대서나 오는 것을 다 받겠다.
		int addr_size = sizeof(sockaddr);

		SOCKET cs = WSAAccept(s, reinterpret_cast<sockaddr *>(&c_addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == cs) {
			ErrorDisplay("In Accept Thread:WSAAccept()");
			continue;
		}
		cout << "New Client Connected!\n";
		int id = -1;
		for (int i = 0; i < MAX_USER; ++i) 
			if (false == g_clients[i].m_isconnected) {
				id = i;
				break;
			}
		if (-1 == id) {
			cout << "MAX USER Exceeded\n";
			continue;
		}
		cout << "ID of new Client is [" << id << "]\n";
		g_clients[id].m_s = cs;
		//clear for reuse
		g_clients[id].m_packet_size = 0;
		g_clients[id].m_prev_packet_size = 0;
		g_clients[id].m_viewlist.clear();
		


		CreateIoCompletionPort(reinterpret_cast<HANDLE>(cs), gh_iocp, id, 0);
		g_clients[id].m_isconnected = true;
		StartRecv(id);
		
		SC_Msg_Put_Character p;
		p.Character_id = id;
		p.size = sizeof(p);
		p.type = SC_PUT_PLAYER;
		p.x = g_clients[id].m_x;
		p.y = g_clients[id].m_y;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (g_clients[i].m_isconnected) {
				//check for in range of seeing
				if (false == CanSee(i, id)) continue;
				g_clients[id].m_mvl.lock();
				g_clients[id].m_viewlist.insert(id);
				g_clients[id].m_mvl.unlock();
				SendPacket(i, &p);
			}
			
		}

		for (int i = 0; i < MAX_USER; ++i)
		{
			//if (!g_clients[i].m_isconnected)
				//continue;
			if (i == id)
				continue;
			//check for in range of seeing
			//if (false == CanSee(i, id)) continue;
			p.Character_id = i;
			p.x = g_clients[i].m_x;
			p.y = g_clients[i].m_y;
			
			g_clients[id].m_mvl.lock();
			g_clients[id].m_viewlist.insert(i);
			g_clients[id].m_mvl.unlock();
			SendPacket(id, &p);

		}
	}
}

int main()
{
	vector <thread> w_threads;
	initialize();
	//CreateWorkerThreads();	// 쓰레드 조인까지 이 안에서 해주어야 한다. 전역변수 해서 관리를 해야 함. 전역변수 만드는 것은
							// 좋은 방법이 아님.
	for (int i = 0; i < 4; ++i) w_threads.push_back(thread{ worker_thread }); // 4인 이유는 쿼드코어 CPU 라서
	//CreateAcceptThreads();
	thread a_thread{ accept_thread };
	for (auto& th : w_threads) th.join();
	a_thread.join();
}