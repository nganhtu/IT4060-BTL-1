#pragma once
#include <WS2tcpip.h>

/*get length from place possition of message*/
unsigned short getLength(char place[])
{
	void *p = place;
	unsigned short Host_num = ntohs(*((unsigned short *)p));
	return Host_num;
}

/*pate opcode and length number into header of message*/
int packageHeader(char opcode, unsigned short length, char mes[], int mes_size)
{
	if (mes_size < length + 3)
		return 0;
	mes[0] = opcode;
	unsigned short Net_num = htons(length);
	void *p = &Net_num;
	mes[1] = *((char *)p);
	mes[2] = *((char *)p + 1);
	return 1;
}
