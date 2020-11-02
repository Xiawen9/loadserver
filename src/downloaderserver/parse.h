#ifndef _PARSE_H_
#define _PARSE_H_

#include <string>
#include "http_header.h"

struct HTTP_RES_HEADER parse_header(const char *response)
{
	/*获取响应头的信息*/
	struct HTTP_RES_HEADER resp;

	char *pos = strStr(response, "HTTP/");
	if (pos)//获取返回代码
		sscanf(pos, "%*s %d", &resp.status_code);

	pos = strStr(response, "Content-Type:");
	if (pos)//获取返回文档类型
		sscanf(pos, "%*s %s", resp.content_type);

	pos = strStr(response, "Content-Length:");
	if (pos)//获取返回文档长度
		sscanf(pos, "%*s %ld", &resp.content_length);

	return resp;
}

void parse_url(const char *url, char *host, int *port, char *file_name)
{
	/*通过url解析出域名, 端口, 以及文件名*/
	int j = 0;
	int start = 0;
	//*port = 80;
	char *patterns[] = { "http://", "https://", NULL };

	for (int i = 0; patterns[i]; i++)//分离下载地址中的http协议
		if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
			start = strlen(patterns[i]);

	//解析域名, 这里处理时域名后面的端口号会保留
	for (int i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
		host[j] = url[i];
	host[j] = '\0';

	//解析端口号, 如果没有, 那么设置端口为80
	char *pos = strstr(host, ":");
	if (pos)
		sscanf(pos, ":%d", port);

	//删除域名端口号
	for (int i = 0; i < (int)strlen(host); i++)
	{
		if (host[i] == ':')
		{
			host[i] = '\0';
			break;
		}
	}

	//获取下载文件名
	j = 0;
	for (int i = start; url[i] != '\0'; i++)
	{
		if (url[i] == '/')
		{
			if (i != strlen(url) - 1)
				j = 0;
			continue;
		}
		else
			file_name[j++] = url[i];
	}
	file_name[j] = '\0';
}


void get_ip_addr(char *host_name, char *ip_addr)
{
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int ret = WSAStartup(wVersionRequested, &wsaData);

	struct hostent *host = gethostbyname(host_name);
	if (!host)
	{
		ip_addr = NULL;
		return;
	}

	for (int i = 0; host->h_addr_list[i]; i++)
	{
		strcpy(ip_addr, inet_ntoa(*(struct in_addr*) host->h_addr_list[i]));
		break;
	}
}

#endif // !_PARSE_H_