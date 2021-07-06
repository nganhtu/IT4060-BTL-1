#pragma once
#include "data_type.h"

/*get length from place possition of message*/
unsigned short getLength(char place[])
{
	void *p = place;
	unsigned short Host_num = ntohs(*((unsigned short *)p));
	return Host_num;
}

//return -1 if okey somethingelse is error code
int getAccountFromMess(char mess[], int mess_len, user &usr)
{
	if (mess_len < 6 || mess[4] != ' ')
	{
		printf("mess[4] error\n");
		return 0;
	}
	int separation = mess_len;
	for (int i = 5; i < mess_len; i++)
	{
		if (mess[i] == '\n')
		{
			separation = i;
			break;
		}
		if (mess[i] < 33)
		{ //check syntax
			printf("username syntax error\n");
			return 1;
		}
	}
	if (separation == 5 || separation >= mess_len - 1)
	{
		printf("separation >= mess_len - 1\n");
		return 0;
	}
	//check password syntax
	for (int i = separation + 1; i < mess_len; i++)
	{
		if (mess[i] < 33)
		{ //check syntax
			printf("password syntax error\n");
			return 2;
		}
	}
	usr.islogin = false;
	usr.username.clear();
	usr.password.clear();
	char *username = mess + 5;
	mess[separation] = '\0';
	usr.username.append(username);
	char *password = mess + separation + 1;
	char tmp[2] = {mess[mess_len - 1], '\0'};
	mess[mess_len - 1] = '\0';
	usr.password.append(password);
	usr.password.append(tmp);
	return -1;
}

/*return -1 if okey something else is error code*/
int getCategoryAndAddress(char *mess, int mess_len, string &category, string &address)
{
	if (mess_len < 6 || mess[4] != ' ')
	{
		return 0;
	}
	int separation = mess_len;
	for (int i = 5; i < mess_len; i++)
	{
		if (mess[i] == '\n')
		{
			separation = i;
			break;
		}
	}
	if (separation < 6 || separation >= mess_len - 1)
	{
		printf("sparation %d mess_len %d\n", separation, mess_len);
		return 0;
	}
	mess[separation] = '\0';
	category.empty();
	category.append(mess + 5);
	address.empty();
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	address.append(mess + separation + 1);
	address.push_back(tmp);
	return -1;
}

int getUsernameAndCategoryAndAddress(char *mess, int mess_len, string &username, string &category, string &address)
{
	if (mess_len < 6 || mess[4] != ' ')
	{
		return 0;
	}
	int separation1 = mess_len, separation2 = mess_len;
	for (int i = 5; i < mess_len; i++)
	{
		if (mess[i] == '\n')
		{
			separation1 = i;
			break;
		}
	}
	for (int i = separation1 + 1; i < mess_len; i++)
	{
		if (mess[i] == '\n')
		{
			separation2 = i;
			break;
		}
	}
	mess[separation1] = '\0';
	username.empty();
	username.append(mess + 5);
	mess[separation2] = '\0';
	category.empty();
	category.append(mess + separation1 + 1);
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	address.append(mess + separation2 + 1);
	address.push_back(tmp);
	return -1;
}
