#pragma once
#include<WS2tcpip.h>

/*get length from place possition of message*/
unsigned short getLength(char place[]) {
	void *p = place;
	unsigned short Host_num = ntohs(*((unsigned short*)p));
	return Host_num;
}

/*paste length number into header of message
length is the length of all message*/
int pasteLength(unsigned short length, char mes[], int mes_size) {
	if (mes_size < length) return 0;
	unsigned short Net_num = htons(length);
	void *p = &Net_num;
	mes[0] = *((char*)p);
	mes[1] = *((char*)p + 1);
	return 1;
}