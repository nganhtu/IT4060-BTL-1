// client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "package_message.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 2048
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main(int argc, char *argv[])
{
	//Step 01: check argv
	struct in_addr ipAddr;
	if (argc != 3 || atoi(argv[2]) < 1 || inet_pton(AF_INET, argv[1], &ipAddr) != 1) {
		cout << "Input not correcly!" << endl;
		return 0;
	}

	//Step 02: start up winsock
	WORD wVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(wVersion, &wsaData) == SOCKET_ERROR) {
		cout << "Version is not supported!" << endl;
		return 0;
	}

	//Step 03: construct socket 
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		cout << "Don't construct socket with error code: " << WSAGetLastError() << endl;
		return 0;
	}
	// Set time-out for receive
	//int tv = 800;
	//setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char *)(&tv), sizeof(int));

	//Step 04: Specify server address
	sockaddr_in serverAddr;
	int serverAddrLen = sizeof(serverAddr);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! Cannot connect server.\n");
		return 0;
	}

	//Step 6: Communicate with server
	// prepare message
	char buff[BUFFER_SIZE];
	int opt = -1, rt, seq = 0;
	bool press;
	while (1) {
		if(seq == 0) opt = showMenu();
		press = true;
		switch (opt)
		{
		case 1://register message
			rt = getRG_Mess(buff, sizeof buff);
			if (rt <= 0) continue;
			break;
		case 2://login message
			rt = getLI_Mess(buff, sizeof buff);
			if (rt <= 0) continue;
			break;
		case 3://add address
			rt = getAA_Mess(buff, sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 4://remove address
			rt = getRA_Mess(buff,sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 5://show address
			rt = getGA_Mess(buff,sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 6://share address
			rt = getSF_Mess(buff,sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 7://backup categary
			rt = getBK_Mess(buff,sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 8://restore categary
			rt = getRS_Mess(buff,sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 10:
			printf("WARNING: friend address can be lost(y/n):");
			if (getchar() != 'y') break;
			rt = getLO_Mess(buff, sizeof buff);
			if (rt <= 0) continue;
			break;
		default:
			printf("option is error\n");
			continue;
		}
		//send message to server
		//memcpy_s(buff + rt, 2048, buff, rt);

		/*****************/
		printf("Client send to server: \"%s\"\n", buff + 2);
		/*****************/

		int ret = send(client, buff, rt, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Cannot send message.\n", WSAGetLastError());
			exit(0);
		}
		//receive response
		if (ret > 0)
		rt = receive(client);
		if (rt == -1 && seq != 0) {
			seq = 0;
			press = true;
		}
		if (press) {
			printf("Press to continue:");
			_getch();
			printf("\n\n");
		}
	}
	// Step 00: Close socket
	closesocket(client);
	
	WSACleanup();
	return 0;
}

