// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#pragma warning(disable : 4996)

using namespace std;

int getFileSize(const char file_path[]) {
	FILE *fp;
	//check file is exists
	fp = fopen(file_path, "rb");
	if (fp == NULL) {
		//printf("File path not exists!\n");
		return -1;
	}
	//check size of file
	fseek(fp, 0, SEEK_END);
	int file_size = (int)ftell(fp);
	fclose(fp);
	return file_size;
}

int readFileBlock(const char file_path[], int front, char *buff, int buff_size) {
	FILE *fp;
	//check file is exists
	fp = fopen(file_path, "rb");
	if (fp == NULL) {
		//printf("File path not exists!\n");
		return -1;
	}
	fseek(fp, front, SEEK_SET);
	int readedBytes = fread(buff, sizeof(char), buff_size, fp);
	fclose(fp);
	if (readedBytes >= 0) return readedBytes;
	return -1;
}

/*return -2 file error, -1 file empty, 0 ok, 1 line not exists*/
int findFileLine(const char file_path[], unsigned int line_num, int &front, int &last, string *lpcontent) {
	if (line_num <= 0) return 1;
	int &beginOfLine = front, &endOfLine = last;
	int file_size = (int)getFileSize(file_path);
	if (file_size <= 0) return file_size - 1;
	char *buff = new char[file_size];
	int readedBytes = (int)readFileBlock(file_path, 0, buff, file_size);
	if (readedBytes <= 0) {
		delete(buff);
		return readedBytes - 1;
	}
	int i, cnt = 0;
	for (i = 0; i<readedBytes; i++) {
		if (cnt == line_num - 1) {
			beginOfLine = i;
			break;
		}
		if (buff[i] == '\n') cnt++;
	}
	for (; i < readedBytes; i++) {
		if (buff[i] == '\n') cnt++;
		if (cnt == line_num) {
			endOfLine = i;
			break;
		}
	}
	printf("i %d\n", i);
	if (i >= readedBytes) {
		delete(buff);
		return 1;
	}
	if (lpcontent != NULL) {
		(*lpcontent).empty();
		char tmp[2] = { buff[endOfLine],'\0' };
		buff[endOfLine] = '\0';
		(*lpcontent).append(buff + beginOfLine);
		(*lpcontent).append(tmp);
	}
	delete(buff);
	return 0;
}

int main()
{
	string path;
	cout << "nhap vào ten cua file:";
	getline(cin, path);
	int num;
	cout << "nhap num:";
	cin >> num;
	string content;
	int front, last;
	findFileLine(path.c_str(), num, front, last, &content);
	cout << content << endl;
	cout << front << " " << last << endl;
	getchar();
}
