#pragma once
#include <iostream>
#include <string>
#include <WinSock2.h>
#include "message.h"
#define BUFFER_SIZE 2048
using namespace std;

int showMenu()
{
	int check;
	cout << "********Menu********" << endl;
	cout << "1. Register" << endl;
	cout << "2. Log in" << endl;
	cout << "3. Add address" << endl;
	cout << "4. Remove address" << endl;
	cout << "5. Show address" << endl;
	cout << "6. Share address" << endl;
	cout << "7. Backup category" << endl;
	cout << "8. Restore category" << endl;
	cout << "9. Log out" << endl;
	cout << "*. Enter to Exit" << endl;
	cout << "Please input the number:";
	string s;
	getline(cin, s);
	check = atoi(s.c_str());
	if (check == 0)
		exit(0);
	if (check < 1 || check > 9)
	{
		cout << "The number is not correctly" << endl;
		cout << "Please press to continue:";
		getchar();
		cout << "\n\n";
		return showMenu();
	}
	return check;
}

int getOriginalResponse(SOCKET &client)
{
	printf("original\n");
	static char recvBuff[BUFFER_SIZE];
	int ret;
	int point = 0;
	while (true)
	{
		int i;
		// receive response from server
		for (i = 0; i < 5; i++)
		{
			ret = recv(client, recvBuff, 3, 0);
			int length = getLength(recvBuff + 1);
			if (length != 0)
				ret += recv(client, recvBuff + 3, length, 0);
			//ret = recv(client, recvBuff, BUFFER_SIZE, 0);
			if (ret == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAETIMEDOUT)
				{
					if (i == 4)
						printf("Time out!\n");
					continue;
				}
				printf("Error %d: Can't receive message.\n", WSAGetLastError());
				exit(0);
			}
			break;
		}
		printf("receive message\n");
		if (ret < 3)
		{
			printf("response length is at lease 3\n");
			return -1;
		}
		int opcode = recvBuff[0];
		int length = (recvBuff[1] << 8) + recvBuff[2];
		printf("receive response from server:\n");
		printf("length of reponse: %d\n", ret);
		printf("opcode: %d\n", opcode);
		printf("length param: %d\n", length);

		if (length + 3 > ret)
		{
			printf("lenth: %d > recv_len: %d\n", length, ret);
			return -1;
		}
		if (opcode == -1 && length == 1)
			printf("error code %d\n", recvBuff[3]);
		else
		{
			for (int i = 3; i < length + 3; i++)
			{
				printf("%d ", recvBuff[i]);
			}
		}

		if (opcode == -1 || opcode == 0 || length == 0)
		{
			printf("\ndone\n");
			return opcode;
		}
	}
}

int getRG_Mess(char buff[], int buff_size)
{
	int length;
	string username, password;
	cout << "Please input your username:";
	getline(cin, username);
	cout << "Please input your password:";
	getline(cin, password);
	length = 4 + 1 + username.size() + 1 + password.size();
	if (length > buff_size)
	{
		cout << "Error: input is longer than able\n";
		//cout << "Please press to continue:";
		//getchar();
		//cout << "\n\n";
		return 0;
	}
	if (pasteLength(length, buff, buff_size) == 0)
		return 0;
	buff[2] = '\0';
	strcat_s(buff + 2, buff_size - 2, "RG ");
	strcat_s(buff + 2, buff_size - 2, username.c_str());
	strcat_s(buff + 2, buff_size - 2, "\n");
	strcat_s(buff + 2, buff_size - 2, password.c_str());
	return length;
}

int getLI_Mess(char buff[], int buff_size)
{
	int length = getRG_Mess(buff, buff_size);
	buff[2] = 'L';
	buff[3] = 'I';
	buff[4] = ' ';
	return length;
}

int getAA_Mess(char buff[], int buff_size, int &seq, bool &press)
{
	static string category, address;
	if (seq == 0)
	{
		category.clear();
		address.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "SG");
		pasteLength(length, buff, BUFFER_SIZE);
		seq = 1;
		return length;
	}
	if (seq == 1)
	{
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff, BUFFER_SIZE);
		seq = 2;
		press = false;
		return length;
	}
	if (seq == 2)
	{
		int length;
		string category, address;
		printf("Please input your category:");
		getline(cin, category);
		printf("Note: input number to add friend address\n");
		printf("Please input your address:");
		getline(cin, address);
		length = category.size() + address.size() + 1 + 5;
		if (length > buff_size)
		{
			cout << "Error: input is longer than able\n";
			seq = 0;
			return 0;
		}
		if (pasteLength(length, buff, buff_size) == 0)
			exit(0);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "AA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		strcat_s(buff + 2, buff_size - 2, "\n");
		strcat_s(buff + 2, buff_size - 2, address.c_str());
		seq = 0;
		return length;
	}
	//default error
	seq = 0;
	return 0;
}

int getRA_Mess(char buff[], int buff_size, int &seq, bool &press)
{
	static string category, num;
	if (seq == 0)
	{
		category.clear();
		num.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff, BUFFER_SIZE);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1)
	{
		int length;
		printf("Please input your category:");
		getline(cin, category);
		length = category.size() + 5;
		if (length > buff_size)
		{
			cout << "Error: input is longer than able\n";
			seq = 0;
			return 0;
		}
		if (pasteLength(length, buff, buff_size) == 0)
			exit(0);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "GA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 2;
		press = false;
		return length;
	}
	if (seq == 2)
	{
		int length;
		printf("Please input the number of address:");
		getline(cin, num);
		length = category.size() + num.size() + 1 + 5;
		if (length > buff_size)
		{
			cout << "Error: input is longer than able\n";
			cout << "Please press to continue:";
			getchar();
			cout << "\n\n";
			seq = 0;
			return 0;
		}
		if (pasteLength(length, buff, buff_size) == 0)
			exit(0);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "RA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		strcat_s(buff + 2, buff_size - 2, "\n");
		strcat_s(buff + 2, buff_size - 2, num.c_str());
		seq = 0;
		return length;
	}
	//default
	seq = 0;
	return 0;
}

int getGA_Mess(char buff[], int buff_size, int &seq, bool &press)
{
	static string category;
	if (seq == 0)
	{
		category.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff, BUFFER_SIZE);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1)
	{
		string category;
		cout << "Please input your category:";
		getline(cin, category);
		int length = 4 + 1 + category.size();
		if (length > buff_size)
		{
			seq = 0;
			return 0;
		}
		if (pasteLength(length, buff, buff_size) == 0)
		{
			seq = 0;
			return 0;
		}
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "GA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 0;
		return length;
	}
	seq = 0;
	return 0;
}

int getSF_Mess(char buff[], int buff_size, int &seq, bool &press)
{
	string username;
	static string category, num;
	if (seq == 0)
	{
		category.clear();
		num.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff, BUFFER_SIZE);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1)
	{
		int length;
		printf("Please input your category:");
		getline(cin, category);
		length = category.size() + 5;
		if (length > buff_size)
		{
			cout << "Error: input is longer than able\n";
			seq = 0;
			return 0;
		}
		if (pasteLength(length, buff, buff_size) == 0)
			exit(0);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "GA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 2;
		press = false;
		return length;
	}
	if (seq == 2)
	{
		int length;
		printf("Please input the number of address:");
		getline(cin, num);
		printf("Please input your friend's username:");
		getline(cin, username);
		length = 4 + 1 + username.size() + 1 + category.size() + 1 + num.size();
		if (length > buff_size)
		{
			cout << "Error: input is longer than able\n";
			cout << "Please press to continue:";
			getchar();
			cout << "\n\n";
			seq = 0;
			return 0;
		}
		if (pasteLength(length, buff, buff_size) == 0)
			exit(0);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "SF ");
		strcat_s(buff + 2, buff_size - 2, username.c_str());
		strcat_s(buff + 2, buff_size - 2, "\n");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		strcat_s(buff + 2, buff_size - 2, "\n");
		strcat_s(buff + 2, buff_size - 2, num.c_str());
		seq = 0;
		return length;
	}
	//default
	seq = 0;
	return 0;
}

int getBK_Mess(char buff[], int buff_size, int &seq, bool &press)
{
	static string category;
	if (seq == 0)
	{
		category.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff, BUFFER_SIZE);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1)
	{
		string category;
		cout << "Please input your category:";
		getline(cin, category);
		int length = 4 + 1 + category.size();
		if (length > buff_size)
		{
			seq = 0;
			return 0;
		}
		if (pasteLength(length, buff, buff_size) == 0)
		{
			seq = 0;
			return 0;
		}
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "BK ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 0;
		return length;
	}
	seq = 0;
	return 0;
}

int getRS_Mess(char buff[], int buff_size, int &seq, bool &press)
{
	static string category;
	if (seq == 0)
	{
		category.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GB");
		pasteLength(length, buff, BUFFER_SIZE);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1)
	{
		string category;
		cout << "Please input your category:";
		getline(cin, category);
		int length = 4 + 1 + category.size();
		if (length > buff_size)
		{
			seq = 0;
			return 0;
		}
		if (pasteLength(length, buff, buff_size) == 0)
		{
			seq = 0;
			return 0;
		}
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "RS ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 0;
		return length;
	}
	seq = 0;
	return 0;
}

int getLO_Mess(char buff[], int buff_size)
{
	int length = 4;
	strcpy_s(buff + 2, BUFFER_SIZE - 3, "LO");
	pasteLength(length, buff, BUFFER_SIZE);
	return length;
}

void getErrorNotice(int error_code)
{
	switch (error_code)
	{
	case 0:
		printf("Error: unspecified message\n");
		break;
	case 1:
		printf("Error: username is not valid\n");
		break;
	case 2:
		printf("Error: password is not valid\n");
		break;
	case 3:
		printf("Error: username or password is wrong\n");
		break;
	case 4:
		printf("Error: account haved login at another\n");
		break;
	case 5:
		printf("Error: didn't log in yet\n");
		break;
	case 6:
		printf("Error: haven't log out\n");
		break;
	case 11:
		printf("Error: category name is not valid\n");
		break;
	case 12:
		printf("Error: category name is not exists\n");
		break;
	case 21:
		printf("Error: address num is not exists\n");
		break;
	default:
		printf("common error\n");
		break;
	}
};

int receive(SOCKET client)
{
	static char recvBuff[BUFFER_SIZE];
	bool first = true;
	bool enter = false;
	int num = 1;
	while (true)
	{
		int i = 0, ret;
		// receive response from server
		for (i; i < 5; i++)
		{
			ret = recv(client, recvBuff, 3, 0);
			int length = getLength(recvBuff + 1);
			if (length != 0)
			{
				ret += recv(client, recvBuff + 3, length, 0);
			}
			//ret = recv(client, recvBuff, BUFFER_SIZE, 0);
			if (ret == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAETIMEDOUT)
				{
					if (i == 4)
						printf("Time out!\n");
					return -1;
				}
				printf("Error %d: Can't receive message.\n", WSAGetLastError());
				exit(0);
			}
			break;
		}

		if (ret < 3)
		{
			printf("Response length is at lease 3\n");
			return -1;
		}
		int opcode = recvBuff[0];
		int length = getLength(recvBuff + 1);
		if (length + 3 > ret)
		{
			printf("Length: %d > recv_len: %d\n", length, ret);
			return -1;
		}
		switch (opcode)
		{
		case -1:
			if (length != 1)
			{
				printf("Erorr opcode=1 length != 1\n");
				return -1;
			}
			getErrorNotice(recvBuff[3]);
			return -1;

		case 0:
			if (length != 0)
			{
				printf("Erorr opcode=0 length != 0\n");
				return -1;
			}
			printf("+++The command is done+++\n");
			return 0;

		case 1:
			if (first)
			{
				first = false;
				printf("===Show suggested category===\n");
				if (length != 0)
					printf("\t");
			}
			for (int i = 3; i < length + 3; i++)
			{
				if (recvBuff[i] == '\n')
				{
					enter = true;
					printf("\n");
				}
				else if (enter)
				{
					enter = false;
					printf("\t%c", recvBuff[i]);
				}
				else
				{
					printf("%c", recvBuff[i]);
				}
			}
			break;

		case 2:
			if (first)
			{
				first = false;
				printf("+++Log in is okey+++\n");
				if (length != 0)
				{
					printf("===Show friend address===\n");
					printf("format:num friend category |^| address\n");
					printf("%d\t", num++);
				}
			}
			for (int i = 3; i < length + 3; i++)
			{
				switch (recvBuff[i])
				{
				case 5:
					printf(" |^| ");
					enter = false;
					break;
				case '\n':
					printf("\n");
					enter = true;
					break;
				default:
					if (enter)
						printf("%d\t", num++);
					printf("%c", recvBuff[i]);
					enter = false;
					break;
				}
			}
			break;

		case 3:
			if (first)
			{
				first = false;
				printf("===Show category===\n");
				if (length != 0)
				{
					printf("\t");
				}
			}
			for (int i = 3; i < length + 3; i++)
			{
				if (recvBuff[i] == '\n')
				{
					enter = true;
					printf("\n");
				}
				else if (enter)
				{
					enter = false;
					printf("\t%c", recvBuff[i]);
				}
				else
				{
					printf("%c", recvBuff[i]);
				}
			}
			break;

		case 4:
			if (first)
			{
				first = false;
				printf("===Show address===\n");
				if (length != 0)
				{
					printf("prefix - is myself\n");
					printf("%d\t", num++);
				}
			}
			for (int i = 3; i < length + 3; i++)
			{
				switch (recvBuff[i])
				{
				case '\n':
					printf("\n");
					enter = true;
					break;
				case '+':
				case '-':
					if (enter)
					{
						enter = false;
						printf("%d\t%c", num++, recvBuff[i]);
					}
					else
					{
						printf("%c", recvBuff[i]);
					}
					break;
				default:
					printf("%c", recvBuff[i]);
					break;
				}
			}
			break;

		case 5:
			if (first)
			{
				first = false;
				printf("===Show back up===\n");
				if (length != 0)
					printf("\t");
			}
			for (int i = 3; i < length + 3; i++)
			{
				if (recvBuff[i] == '\n')
				{
					enter = true;
					printf("\n");
				}
				else if (enter)
				{
					printf("\t%c", recvBuff[i]);
				}
				else
				{
					printf("%c", recvBuff[i]);
				}
			}
			break;
		}
		if (length == 0)
			return opcode;
	}
}
