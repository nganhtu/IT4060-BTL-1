#pragma once

#include <WinSock2.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

#define RECEIVE 0
#define SEND 1
#define BUFFSIZE 2048

typedef struct user {
	string username;
	string password;
	bool islogin;// init is false
} user;

typedef vector<user> users;

int getAccountLocation(users &userList, user &usr) {
	for (int i = 0; i < (int)userList.size(); i++) {
		if (usr.username == userList[i].username && usr.password == userList[i].password)
			return i;
	}
	return -1;
}

/*struct contains infomation of client's connect session*/
typedef struct ClientInfo {
	bool islogin; //init is false
	int login_place;
	char username[260];
	int fa_bytes; //the bytes of the fa.txt file is get at log in
	char **responses; //init with 100 pieces
	int n; //count of the responses
	int i;//the number of responses is sent
} ClientInfo;

/*Struct contains information of the socket communicating with client*/
typedef struct SocketInfo {
	SOCKET socket;
	OVERLAPPED overlapped;
	int operation;//init is receive
	WSABUF dataBuf;
	char recvBuffer[BUFFSIZE];
	int recvBytes;
	int sendBytes;
	int sentBytes;
	ClientInfo clientInfo;
}SocketInfo;

/**
* The init function constructs a socketinfo and event and put connSock on n in array
* @param	socks		An array of pointers of socket information struct
* @param	events		An WSAEVENT array
* @param	n			Index of the construct socket
* @param	connSock	An socket
*/
void init(SocketInfo *socketInfos[],WSAEVENT events[],int n,SOCKET connSock) {
	// Disassociate connected socket with any event object
	WSAEventSelect(connSock, NULL, 0);

	// Append connected socket to the array of SocketInfo
	events[n] = WSACreateEvent();
	socketInfos[n] = new SocketInfo;
	socketInfos[n]->socket = connSock;
	socketInfos[n]->overlapped.hEvent = events[n];
	socketInfos[n]->dataBuf.buf = socketInfos[n]->recvBuffer;
	socketInfos[n]->dataBuf.len = BUFFSIZE;
	socketInfos[n]->operation = RECEIVE;
	socketInfos[n]->clientInfo.islogin = false;
	socketInfos[n]->clientInfo.responses = new char *[100];
}

/**
* The unInit function remove a SocketInfo and WSAEVENT at n
* @param	socks		An array of pointers of socket information struct
* @param	events		An WSAEVENT array
* @param	n			Index of the construct socket
*/
void unInit(SocketInfo *socks[], WSAEVENT events[], int n) {
	WSACloseEvent(events[n]);
	closesocket(socks[n]->socket);
	delete(socks[n]->clientInfo.responses);
	delete(socks[n]);
	socks[n] = NULL;
}



void toWchar_t(const string &src, wchar_t *des, int des_s) {
	for (int i = 0; i < (int)src.size(); i++) {
		if (des_s > i) {
			des[i] = (wchar_t)src[i];
		}
		else break;
	}
	if ((int)src.size() > des_s) {
		des[des_s - 1] = '\0';
	}
	else {
		des[src.size()] = '\0';
	}
}


void toWchar_t(const char *src, wchar_t *des, int des_s) {
	int src_len = strlen(src);
	for (int i = 0; i < src_len; i++) {
		if (des_s > i) {
			des[i] = (wchar_t)src[i];
		}
		else break;
	}
	if (src_len > des_s) {
		des[des_s - 1] = '\0';
	}
	else {
		des[src_len] = '\0';
	}
}

bool checkNum(const char s[]) {
	int len = strlen(s);
	if (len == 0) return false;
	for (int i = 0; i < len; i++) {
		if (s[i]<'0' || s[i]>'9') return false;
	}
	return true;
}