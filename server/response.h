#pragma once
//#include "data_type.h"
#include "unbox_mess.h"
#include "io.h"
#define USER_FILE "account.txt"

void packageResHeader(char opcode, unsigned short length, char mes[]) {
	mes[0] = opcode;
	unsigned short Net_num = htons(length);
	void *p = &Net_num;
	mes[1] = *((char*)p);
	mes[2] = *((char*)p + 1);
}

char* getNoticeMess(int error_code, int opcode = 0) {
	char *notice;
	if (error_code != -1) {
		notice = new char[4];
		packageResHeader(-1, 1, notice);
		notice[3] = error_code;
	}
	else {
		notice = new char[3];
		packageResHeader(opcode, 0, notice);
	}
	return notice;
}

//return -1 and 0
int getRG_response(char **res,int &n, char mess[], int mess_len, users &userList) {
	user usr;
	int rt = getAccountFromMess(mess, mess_len, usr);
	if (rt != -1) {
		res[n++] = getNoticeMess(rt);
		return -1;
	}
	string foulder_path;
	foulder_path.append("users/");
	foulder_path.append(usr.username.c_str());
	if (mark_directory(foulder_path.c_str())) {
		printf("mark directory error\n");
		printf("%s\n", foulder_path.c_str());
		res[n++] = getNoticeMess(1);
		return -1;
	}
	string fa_path = foulder_path + "/fa.txt";
	if (!createFile(fa_path.c_str())) {
		printf("create fa file error\n");
		exit(0);
	}
	foulder_path.append("/back_up");
	if (mark_directory(foulder_path.c_str())) {
		printf("create back_up foulder error\n");
		exit(0);
	}
	//save account
	string s;
	s.append(usr.username.c_str());
	s.append(" ");
	s.append(usr.password.c_str());
	s.append("\n");
	if (!appendFile(USER_FILE, s.c_str(), s.size())) {
		printf("append user file error\n");
		exit(0);
	}
	//add user to userlist
	userList.push_back(usr);
	printf("user count %d\n", userList.size());
	printf("username:%s\n", usr.username.c_str());
	printf("password:%s\n",usr.password.c_str());
	res[n++] = getNoticeMess(-1);
	return 0;
}

int getLI_response(char **res, int &n, char mess[], int mess_len, users &userList, ClientInfo &clientInfo) {
	if (clientInfo.islogin) {
		res[n++] = getNoticeMess(6);
		return -1;
	}
	user usr;
	int rt = getAccountFromMess(mess, mess_len, usr);
	if (rt != -1) {
		res[n++] = getNoticeMess(rt);
		return -1;
	}
	int index = getAccountLocation(userList, usr);
	if (index == -1) {
		res[n++] = getNoticeMess(3);
		return -1;
	}
	if (userList[index].islogin) {
		res[n++] = getNoticeMess(4);
		return -1;
	}
	userList[index].islogin = true;
	clientInfo.islogin = true;
	clientInfo.login_place = index;
	strcpy_s(clientInfo.username, usr.username.c_str());
	printf("username:%s\n", clientInfo.username);
	string fa_path;
	fa_path.append("users/");
	fa_path.append(usr.username.c_str());
	fa_path.append("/fa.txt");
	printf("fa path:%s\n", fa_path.c_str());
	int fa_bytes = getFileSize(fa_path.c_str());
	clientInfo.fa_bytes = fa_bytes;
	//read file fa.txt
	int beginPos = 0;
	int buff_size = 2;
	while (beginPos < fa_bytes) {
		char *buff = new char[BUFFSIZE];
		int length = readFileBlock(fa_path.c_str(), beginPos, buff + 3, buff_size);
		if (length <= 0) {
			delete(buff);
			break;
		}
		packageResHeader(2, length, buff);
		res[n++] = buff;
		beginPos += length;
	}
	res[n++] = getNoticeMess(-1,2);
	return 0;
}

int getSG_response(char **res, int &n, char mess[], int mess_len, bool islogin) {
	if (mess_len != 4) {
		res[n++] = getNoticeMess(0);
		return -1;
	}
	if (!islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string sg_path = "suggest.txt";
	int sg_bytes = getFileSize(sg_path.c_str());
	printf("sg %d\n", sg_bytes);
	//read file suggest.txt
	int beginPos = 0;
	int buff_size = 2;
	while (beginPos < sg_bytes) {
		char *buff = new char[BUFFSIZE];
		int length = readFileBlock(sg_path.c_str(), beginPos, buff + 3, buff_size);
		if (length <= 0) {
			delete(buff);
			break;
		}
		packageResHeader(1, length, buff);
		res[n++] = buff;
		beginPos += length;
	}
	res[n++] = getNoticeMess(-1,1);
	return 0;
}

int getGC_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (mess_len != 4) {
		res[n++] = getNoticeMess(0);
		return -1;
	}
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string foulder_path;
	foulder_path.append("users/");
	foulder_path.append(clientInfo.username);
	foulder_path.append("/");
	printf("foulder_path %s\n", foulder_path.c_str());
	string file_names;
	int len = getFileNames(foulder_path, file_names);
	printf("len %d\n", len);
	int begin = 0;
	int buff_size = 2;
	while (begin < len) {
		char *buff = new char[BUFFSIZE];
		int length_param = BUFFSIZE - 3 > (len - begin) ? (len - begin) : BUFFSIZE - 3;
		//int length_param = 2;
		memcpy_s(buff + 3, BUFFSIZE - 3, file_names.c_str() + begin, length_param);
		packageResHeader(3, length_param, buff);
		res[n++] = buff;
		begin += length_param;
	}
	res[n++] = getNoticeMess(-1,3);
	return 0;
}

int getGB_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (mess_len != 4) {
		res[n++] = getNoticeMess(0);
		return -1;
	}
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string foulder_path;
	foulder_path.append("users/");
	foulder_path.append(clientInfo.username);
	foulder_path.append("/back_up/");
	printf("foulder_path %s\n", foulder_path.c_str());
	string file_names;
	int len = getFileNames(foulder_path, file_names);
	printf("len %d\n", len);
	int begin = 0;
	int buff_size = 2;
	while (begin < len) {
		char *buff = new char[BUFFSIZE];
		int length_param = BUFFSIZE - 3 > (len - begin) ? (len - begin) : BUFFSIZE - 3;
		//int length_param = 2;
		memcpy_s(buff + 3, BUFFSIZE - 3, file_names.c_str() + begin, length_param);
		packageResHeader(3, length_param, buff);
		res[n++] = buff;
		begin += length_param;
	}
	res[n++] = getNoticeMess(-1,3);
	return 0;
}

int getAA_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string categary, address;
	int rt = getCagegaryAndAddress(mess, mess_len, categary, address);
	if (rt != -1) {
		res[n++] = getNoticeMess(rt);
		return -1;
	}
	if (!categary.compare("fa")) {
		res[n++] = getNoticeMess(11);
		return -1;
	}
	string file_path, fa_path;
	file_path.append("users/");
	file_path.append(clientInfo.username);
	file_path.append("/"); 
	fa_path.append(file_path.c_str());
	fa_path.append("fa.txt");
	file_path.append(categary.c_str());
	file_path.append(".txt");
	printf("file_path %s\n", file_path.c_str());
	//Check add friend
	if (checkNum(address.c_str())) {
		printf("have number\n");
		int front, last, separation, num = atoi(address.c_str());
		string line;
		int rt = findFileLine(fa_path.c_str(), num, front, last, &line);
		printf("%s\n", fa_path.c_str());
		printf("%s\n", line.c_str());
		if (rt < 0) {
			res[n++] = getNoticeMess(11);
			return -1;
		}
		if (rt > 0) {
			res[n++] = getNoticeMess(21);
			return -1;
		}
		separation = line.size() - 1;
		for (int i = 0; i < line.size(); i++)
			if(line[i]==5) separation = i+1;
		printf("%d separation\n", separation);
		address.clear();
		printf("%s\n", address.c_str());
		address.push_back('-');
		printf("%s %d\n", address.c_str(), address.size());
		for (; separation < line.size(); separation++) {
			address.push_back(line[separation]);
		}
		printf("%s\n", address.c_str());
		printf("%s address\n", address.c_str());
		if (!appendFile(file_path.c_str(), address.c_str(), address.size())) {
			res[n++] = getNoticeMess(11);
			return -1;
		}
	}
	else {
		appendFile(file_path.c_str(), "+", 1);
		address.push_back('\n');
		if (!appendFile(file_path.c_str(), address.c_str(), address.size())) {
			res[n++] = getNoticeMess(11);
			return -1;
		}
	}
	res[n++] = getNoticeMess(-1);
	return 0;
}

int getRA_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string categary, address;
	int rt = getCagegaryAndAddress(mess, mess_len, categary, address);
	if (rt != -1) {
		res[n++] = getNoticeMess(rt);
		return -1;
	}
	if (!categary.compare("fa")) {
		res[n++] = getNoticeMess(11);
		return -1;
	}
	int line_num = atoi(address.c_str());
	if (line_num <= 0) {
		res[n++] = getNoticeMess(21);
		return -1;
	}
	string file_path;
	file_path.append("users/");
	file_path.append(clientInfo.username);
	file_path.append("/"); file_path.append(categary.c_str());
	file_path.append(".txt");
	printf("%s\n", address.c_str());
	printf("file_path %s line %d\n", file_path.c_str(), line_num);
	rt = deleteFileLine(file_path.c_str(), line_num);
	printf("111\n");
	if (rt == -1) {
		res[n++] = getNoticeMess(11);
		return -1;
	}
	if (rt == 0) {
		res[n++] = getNoticeMess(21);
		return -1;
	}
	res[n++] = getNoticeMess(-1);
	return 0;
}

int getSF_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string username, categary, address;
	int rt = getUsernameAndCagegaryAndAddress(mess, mess_len, username, categary, address);
	if (rt != -1) {
		res[n++] = getNoticeMess(rt);
		return -1;
	}
	if (!categary.compare("fa")) {
		res[n++] = getNoticeMess(11);
		return -1;
	}
	int line_num = atoi(address.c_str());
	if (line_num <= 0) {
		res[n++] = getNoticeMess(21);
		return -1;
	}
	string file_path, addressContent;
	file_path.append("users/");
	file_path.append(clientInfo.username);
	file_path.append("/"); file_path.append(categary.c_str());
	file_path.append(".txt");
	rt = getFileLineContent(file_path.c_str(), line_num, addressContent);
	if (rt == 0) {
		res[n++] = getNoticeMess(21);
		return -1;
	}

	string fa_file_path;
	fa_file_path.append("users/");
	fa_file_path.append(username);
	fa_file_path.append("/fa.txt");
	printf("%s\n", address.c_str());

	string writeToFile;
	writeToFile.append(clientInfo.username);
	writeToFile.append(" ");
	writeToFile.append(categary);
	writeToFile.append("\5");
	writeToFile.append(addressContent);
	rt = appendFile(fa_file_path.c_str(), writeToFile.c_str(), strlen(writeToFile.c_str()));
	printf("111\n");
	if (rt == -1) {
		res[n++] = getNoticeMess(1);
		return -1;
	}
	if (rt == 0) {
		res[n++] = getNoticeMess(11);
		return -1;
	}
	res[n++] = getNoticeMess(-1);
	return 0;
}

int getGA_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo) {
	//length|GA_categary.
	if (mess_len < 6 || mess[4] != ' ') {
		res[n++] = getNoticeMess(0);
		return -1;
	}
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string categary;
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	categary.append(mess + 5);
	categary.push_back(tmp);
	string categary_path = "users/";
	categary_path.append(clientInfo.username);
	categary_path.push_back('/');
	categary_path.append(categary.c_str());
	categary_path.append(".txt");
	printf("%s file path\n", categary_path.c_str());
	//read file categary_name.txt
	int cgBytes = getFileSize(categary_path.c_str());
	if (cgBytes < 0) {
		res[n++] = getNoticeMess(11);
		return -1;
	}
	int beginPos = 0;
	int buff_size = 2;
	while (beginPos < cgBytes) {
		char *buff = new char[BUFFSIZE];
		int length = readFileBlock(categary_path.c_str(), beginPos, buff + 3, buff_size);
		if (length <= 0) {
			delete(buff);
			break;
		}
		packageResHeader(4, length, buff);
		res[n++] = buff;
		beginPos += length;
	}
	res[n++] = getNoticeMess(-1, 4);
	return 0;
}

int getBK_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo) {
	//length|BK_categary.
	if (mess_len < 6 || mess[4] != ' ') {
		res[n++] = getNoticeMess(0);
		return -1;
	}
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string categary;
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	categary.append(mess + 5);
	categary.push_back(tmp);

	string categary_path = "users/";
	categary_path.append(clientInfo.username);
	categary_path.push_back('/');
	categary_path.append(categary.c_str());
	categary_path.append(".txt");
	printf("%s file path\n", categary_path.c_str());
	//read file categary_name.txt
	int cgBytes = getFileSize(categary_path.c_str());
	if (cgBytes < 0) {
		res[n++] = getNoticeMess(11);
		return -1;
	}

	string backupFilePath = "users/";
	backupFilePath.append(clientInfo.username);
	backupFilePath.append("/back_up/");
	backupFilePath.append(categary.c_str());
	backupFilePath.append(".txt");
	printf("%s is backup file path\n", backupFilePath.c_str());

	int rt = copyFile(categary_path.c_str(), backupFilePath.c_str());
	if (rt == -1) {
		res[n++] = getNoticeMess(11);
		return -1;
	}

	res[n++] = getNoticeMess(-1, 0);
	return 0;
}

int getRS_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo) {
	//length|RS_categary.
	if (mess_len < 6 || mess[4] != ' ') {
		res[n++] = getNoticeMess(0);
		return -1;
	}
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	string categary;
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	categary.append(mess + 5);
	categary.push_back(tmp);

	string categary_path = "users/";
	categary_path.append(clientInfo.username);
	categary_path.push_back('/');
	categary_path.append(categary.c_str());
	categary_path.append(".txt");
	printf("%s file path\n", categary_path.c_str());
	//read file categary_name.txt
	int cgBytes = getFileSize(categary_path.c_str());
	if (cgBytes < 0) {
		res[n++] = getNoticeMess(11);
		return -1;
	}

	string backupFilePath = "users/";
	backupFilePath.append(clientInfo.username);
	backupFilePath.append("/back_up/");
	backupFilePath.append(categary.c_str());
	backupFilePath.append(".txt");
	printf("%s is backup file path\n", backupFilePath.c_str());
	cgBytes = getFileSize(categary_path.c_str());
	if (cgBytes < 0) {
		res[n++] = getNoticeMess(12);
		return -1;
	}

	int rt = copyFile(backupFilePath.c_str(), categary_path.c_str());
	if (rt == -1) {
		res[n++] = getNoticeMess(11);
		return -1;
	}

	res[n++] = getNoticeMess(-1, 0);
	return 0;
}

int getLO_response(char **res, int &n, char mess[], int mess_len, ClientInfo &clientInfo,users &userList) {
	if (mess_len != 4) {
		res[n++] = getNoticeMess(0);
		return -1;
	}
	if (!clientInfo.islogin) {
		res[n++] = getNoticeMess(5);
		return -1;
	}
	clientInfo.islogin = false;
	userList[clientInfo.login_place].islogin = false;
	int fa_bytes = clientInfo.fa_bytes;
	string fa_path;
	fa_path.append("users/"); fa_path.append(clientInfo.username);
	fa_path.append("/fa.txt");
	int file_size = getFileSize(fa_path.c_str());
	if (file_size <= 0 || fa_bytes == 0) {
		res[n++] = getNoticeMess(-1);
		return 0;
	}
	char *buff = new char[file_size-fa_bytes];
	int block_size = readFileBlock(fa_path.c_str(), fa_bytes, buff, file_size - fa_bytes);
	deleteFile(fa_path.c_str());
	appendFile(fa_path.c_str(), buff, file_size - fa_bytes);
	res[n++] = getNoticeMess(-1);
	return 0;
}

void getResponses(SocketInfo &socketInfo, users &userList) {
	printf("getResponse\n");
	int buff_len = socketInfo.recvBytes;
	char *recvBuff = socketInfo.recvBuffer;
	char **responses = socketInfo.clientInfo.responses;
	int &n = socketInfo.clientInfo.n;

	n = 0;

	int mes_beginPos = 0, mes_endPos = 0;
	while (mes_endPos<buff_len - 1) {
		printf("mes_beginPos %d mes_endPos %d\n", mes_beginPos, mes_endPos);
		if (buff_len - mes_beginPos < 4) {	//min of mess length
			//receive error message
			printf("done1\n");
			char *notice_mess = new char[4];
			packageResHeader(-1, 1, notice_mess);
			notice_mess[3] = 0;
			responses[n++] = notice_mess;
			break;
		}
		int length = getLength(recvBuff + mes_beginPos);
		printf("length %d", length);
		mes_endPos = mes_beginPos + (length - 1);
		//ckeck mes_endPos
		if (mes_endPos<mes_beginPos + 3 || mes_endPos>buff_len - 1) {
			//receive error message
			printf("done2\n");
			char *notice_mess = new char[4];
			packageResHeader(-1, 1, notice_mess);
			notice_mess[3] = 0;
			responses[n++] = notice_mess;
			break;
		}
		int mess_type = ((unsigned char)*(recvBuff + mes_beginPos + 2) << 8) + (unsigned char)*(recvBuff + mes_beginPos + 3);//get mess type
		printf("mess_type = %d\n", mess_type);
		switch (mess_type) {
		case (('R' << 8) + 'G'):
		{
			printf("register\n");
			getRG_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, userList);
			break;
		}
		case (('L' << 8) + 'I'): 
		{
			printf("login\n");
			getLI_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, userList, socketInfo.clientInfo);
			break;
		}
		case (('S' << 8) + 'G'):
		{
			printf("suggest\n");
			getSG_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo.islogin);
			break;
		}
		case (('G' << 8) + 'C'):
		{
			printf("getcategary\n");
			getGC_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo);
			break;
		}
		case (('G' << 8) + 'B'):
		{
			printf("get backup categary\n");
			getGB_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo);
			break;
		}
		case (('A' << 8) + 'A'): 
		{
			printf("add address\n");
			getAA_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo);
			break;
		}
		case (('R' << 8) + 'A'): 
		{
			printf("remove address\n");
			getRA_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo);
			break;
		}
		case (('S' << 8) + 'F'): 
		{
			printf("share address\n");
			getSF_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo);
			break;
		}
		case (('G' << 8) + 'A'): 
		{
			printf("get address\n");
			getGA_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo);
			break;
		}
		case (('B' << 8) + 'K'): 
		{
			printf("get address\n");
			getBK_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo);
			break;
		}
		case (('R' << 8) + 'S'): 
		{
			printf("get address\n");
			getRS_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo);
			break;
		}
		case (('L' << 8) + 'O'): 
		{
			printf("logout\n");
			getLO_response(responses, n, recvBuff + mes_beginPos, mes_endPos - mes_beginPos + 1, socketInfo.clientInfo,userList);
			break;
		}
		default:
		{
			printf("default\n");
			responses[n++] = getNoticeMess(0);
			break;
		}
		}
		mes_beginPos = ++mes_endPos;
	}
	printf("mes_endPos %d\n", mes_endPos);
}