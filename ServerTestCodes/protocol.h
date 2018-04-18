#include <windows.h>

#pragma once
#pragma pack(push,1)
struct Packet {
	
	BYTE size;
	short type;
};
//규칙
//SC -> 서버에서 클라 CS-> 클라에서 서버
//Ch -> 캐릭터 Mo -> 중립몬스터, 미니언
struct SC_MsgConnect : public Packet
{
	BYTE client_id;
	BYTE Character_id;
};
struct SC_MsgChCreate : public Packet
{
	BYTE Character_id;
	short x;
	short y;
};
struct SC_MsgMoCreate : public Packet
{
	BYTE Monster_id;
	short x;
	short y;
};
struct SC_MsgChDelete : public Packet
{
	BYTE Character_id;
};
struct SC_MsgMoDelete : public Packet
{
	BYTE Monster_id;
};
struct SC_MsgChMove : public Packet
{
	BYTE Character_id;
	short x;
	short y;
};
struct SC_MsgMoMove : public Packet
{
	BYTE Monster_id;
	short x;
	short y;
};
struct SC_MsgChAttack : public Packet
{
	BYTE Character_id;
	//배열로 처리해야 할때도 있을듯?
	//다중스킬!
	BYTE Monster_id;
};
struct SC_MsgMoAttack : public Packet
{
	BYTE Monster_id;
	BYTE Character_id;
};
struct SC_MsgChHit : public Packet
{
	BYTE Character_id;
	short Ch_Health;
};
struct SC_MsgMoHit : public Packet
{
	BYTE Monster_id;
	short Mo_Health;
};

struct CS_MsgChMove : public Packet
{
	BYTE Character_id;
};
#pragma pack(pop)

#define MAX_BUFF_SIZE 4000
#define MAX_PACKET_SIZE 255

#define MAP_WIDTH 800
#define MAP_HEIGHT 800

#define MAX_USER 5
#define NPC_START 1000
#define NUM_OF_NPC 10000

#define MY_SERVER_PORT 4000

//enum class Type : BYTE { Connect, Create, Delete, Move, Attack, Hit };
//Server-> Client
#define SC_CONNECT_PLAYER		  1
#define SC_PUT_PLAYER			  2
#define SC_DISCONNECT_PLAYER	  3
#define SC_MOVE_PLAYER			  4
#define SC_ATTACK				  5
#define SC_HIT					  6
//Client->Server
#define CS_ACTION				  1
#define CS_MOVE_PLAYER			  2
//In Client Move Object
#define CS_UP	 1
#define CS_DOWN  2
#define CS_LEFT  3
#define CS_RIGHT 4




//enum 추가할 예정


