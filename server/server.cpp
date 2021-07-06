// OEBServer.cpp : This sample illustrates how to develop a simple echo server Winsock
// application using the Overlapped I/O model with event notification.
//

#include "stdafx.h"
#include "winsock2.h"
#include "stdio.h"
#include "WS2tcpip.h"
#include "string.h"

#include "data_type.h"
#include "response.h"
#include "io.h"

#define SERVER_ADDR "127.0.0.1"
#define PORT 5500
#define USER_FILE "account.txt"
#pragma comment(lib, "Ws2_32.lib")

SocketInfo *socketInfos[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
int nEvents = 0;

users userList;

int main()
{
	WSADATA wsaData;
	if (WSAStartup((2, 2), &wsaData) != 0)
	{
		printf("WSAStartup() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; i++)
	{
		socketInfos[i] = 0;
		events[i] = 0;
	}

	// Start Winsock and set up a LISTEN socket
	SOCKET listenSocket;
	if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 1;
	}

	// LISTEN socket associate with an event object
	events[0] = WSACreateEvent();
	nEvents++;
	WSAEventSelect(listenSocket, events[0], FD_ACCEPT | FD_CLOSE);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	if (listen(listenSocket, 10))
	{
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	if (!prepareUserList(USER_FILE, userList))
	{
		printf("read user list error\n");
		exit(0);
	}
	printf("userlist %d\n", userList.size());

	int index;
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	while (1)
	{
		// Wait for network events on all socket
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED)
		{
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
			return 0;
		}

		index = index - WSA_WAIT_EVENT_0;
		DWORD flags, transferredBytes;

		// If the event triggered was zero then a connection attempt was made
		// on our listening socket.
		if (index == 0)
		{
			WSAResetEvent(events[0]);
			if ((connSock = accept(listenSocket, (sockaddr *)&clientAddr, &clientAddrLen)) == INVALID_SOCKET)
			{
				printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
				return 0;
			}

			int i;
			if (nEvents == WSA_MAXIMUM_WAIT_EVENTS)
			{
				printf("\nToo many clients.");
				closesocket(connSock);
			}
			else
			{
				i = nEvents;
				init(socketInfos, events, i, connSock);

				nEvents++;

				// Post an overlpped I/O request to begin receiving data on the socket
				flags = 0;
				if (WSARecv(socketInfos[i]->socket, &(socketInfos[i]->dataBuf), 1, &transferredBytes, &flags, &(socketInfos[i]->overlapped), NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("WSARecv() failed with error %d\n", WSAGetLastError());
						unInit(socketInfos, events, i);
						nEvents--;
					}
				}
			}
		}
		else
		{ // If the event triggered wasn't zero then an I/O request is completed.
			SocketInfo *socketInfo;
			socketInfo = socketInfos[index];
			WSAResetEvent(events[index]);
			BOOL result;
			result = WSAGetOverlappedResult(socketInfo->socket, &(socketInfo->overlapped), &transferredBytes, FALSE, &flags);
			printf("transfer %d %d\n", transferredBytes, socketInfo->operation);
			if (result == FALSE || transferredBytes == 0)
			{
				ClientInfo *clientInfo = &socketInfo->clientInfo;
				if (socketInfo->operation == SEND)
				{
					for (int i = clientInfo->i; i < clientInfo->n; i++)
						delete (clientInfo->responses[i]);
				}
				if (clientInfo->islogin)
				{
					userList[clientInfo->login_place].islogin = false;
				}
				unInit(socketInfos, events, index);
				socketInfos[index] = socketInfos[nEvents - 1];
				events[index] = events[nEvents - 1];
				nEvents--;
				continue;
			}

			// Check to see if the operation field equals RECEIVE. If this is so, then
			// this means a WSARecv call just completed
			if (socketInfo->operation == RECEIVE)
			{
				printf("receive\n");
				for (int i = 0; i < (int)transferredBytes; i++)
					printf("%c", socketInfo->recvBuffer[i]);
				printf("done\n");
				socketInfo->recvBytes = transferredBytes; //the number of bytes which is received from client
				//process recvbuff
				getResponses(*socketInfo, userList);
				//config parameter
				ClientInfo *clientInfo = &(socketInfo->clientInfo);
				char **responses = clientInfo->responses;
				unsigned int length = getLength(responses[0] + 1);
				printf("responses %d, option %d, length %d\n", clientInfo->n, responses[0][0], length);
				clientInfo->i = 0;
				//if (responses[0][0] > 0 && length != 0)
				//	length = BUFFSIZE - 3;
				socketInfo->sendBytes = length + 3;
				socketInfo->dataBuf.buf = responses[0];
				socketInfo->dataBuf.len = length + 3;
				socketInfo->sentBytes = 0;	  //the number of bytes which is sent to client
				socketInfo->operation = SEND; //set operation to send reply message
			}
			else
				socketInfo->sentBytes += transferredBytes;

			// Post another I/O operation
			// Since WSASend() is not guaranteed to send all of the bytes requested,
			// continue posting WSASend() calls until all received bytes are sent.
			if (socketInfo->sendBytes > socketInfo->sentBytes)
			{
				ClientInfo *clientInfo = &(socketInfo->clientInfo);
				socketInfo->dataBuf.buf = clientInfo->responses[clientInfo->i] + socketInfo->sentBytes;
				socketInfo->dataBuf.len = socketInfo->sendBytes - socketInfo->sentBytes;
				socketInfo->operation = SEND;
				if (WSASend(socketInfo->socket, &(socketInfo->dataBuf), 1, &transferredBytes, flags, &(socketInfo->overlapped), NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("WSASend() failed with error %d\n", WSAGetLastError());
						for (int i = clientInfo->i; i < clientInfo->n; i++)
							delete (clientInfo->responses[i]);
						if (clientInfo->islogin)
							userList[clientInfo->login_place].islogin = false;
						unInit(socketInfos, events, index);
						socketInfos[index] = socketInfos[nEvents - 1];
						events[index] = events[nEvents - 1];
						nEvents--;
						continue;
					}
				}
			}
			else
			{
				ClientInfo *clientInfo = &(socketInfo->clientInfo);
				delete (clientInfo->responses[clientInfo->i++]);
				if (clientInfo->i >= clientInfo->n)
				{
					socketInfo->operation = RECEIVE;
					socketInfo->dataBuf.buf = socketInfo->recvBuffer;
					socketInfo->dataBuf.len = BUFFSIZE;
					flags = 0;

					if (WSARecv(socketInfo->socket, &(socketInfo->dataBuf), 1, &transferredBytes, &flags, &(socketInfo->overlapped), NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSA_IO_PENDING)
						{
							printf("WSARecv() failed with error %d\n", WSAGetLastError());
							if (clientInfo->islogin)
								userList[clientInfo->login_place].islogin = false;
							unInit(socketInfos, events, index);
							socketInfos[index] = socketInfos[nEvents - 1];
							events[index] = events[nEvents - 1];
							nEvents--;
						}
					}
				}
				else
				{
					char **responses = clientInfo->responses;
					int length = getLength(responses[clientInfo->i] + 1);
					socketInfo->sendBytes = length + 3;
					socketInfo->dataBuf.buf = clientInfo->responses[clientInfo->i];
					socketInfo->dataBuf.len = length + 3;
					socketInfo->sentBytes = 0; //the number of bytes which is sent to client
					socketInfo->operation = SEND;
					// Post an overlpped I/O request to begin receiving data on the socket
					flags = 0;
					if (WSASend(socketInfo->socket, &(socketInfo->dataBuf), 1, &transferredBytes, flags, &(socketInfo->overlapped), NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSA_IO_PENDING)
						{
							printf("WSASend() failed with error %d\n", WSAGetLastError());
							for (int i = clientInfo->i; i < clientInfo->n; i++)
								delete (clientInfo->responses[i]);
							if (clientInfo->islogin)
								userList[clientInfo->login_place].islogin = false;
							unInit(socketInfos, events, index);
							socketInfos[index] = socketInfos[nEvents - 1];
							events[index] = events[nEvents - 1];
							nEvents--;
							continue;
						}
					}
				}
			}
		}
	}
}
