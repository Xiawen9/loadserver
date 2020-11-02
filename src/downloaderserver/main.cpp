#include <string>
#include <winsock2.h>
#include "common.h"
#include "parse.h"
#include "server_def.h"

#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"ws2_32.lib")

void get_ip_addr(char *host_name, char *ip_addr)
{
	/*通过域名得到相应的ip地址*/
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;

	//WinSock初始化
	wVersionRequested = MAKEWORD(2, 2); //希望使用的WinSock DLL的版本
	ret = WSAStartup(wVersionRequested, &wsaData);

	struct hostent *host = gethostbyname(host_name);//此函数将会访问DNS服务器
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

void progress_bar(long cur_size, long total_size, double speed)
{
	/*用于显示下载进度条*/
	float percent = (float)cur_size / total_size;
	const int numTotal = 50;
	int numShow = (int)(numTotal * percent);

	if (numShow == 0)
		numShow = 1;

	if (numShow > numTotal)
		numShow = numTotal;

	char sign[51] = { 0 };
	memset(sign, '=', numTotal);

	printf("\r%.2f%%[%-*.*s] %.2f/%.2fMB %4.0fkb/s", percent * 100, numTotal, numShow, sign, cur_size / 1024.0 / 1024.0, total_size / 1024.0 / 1024.0, speed);
	fflush(stdout);

	if (numShow == numTotal)
		printf("\n");
}

unsigned long get_file_size(const char *filename)
{
	//通过系统调用直接得到文件的大小
	struct stat buf;
	if (stat(filename, &buf) < 0)
		return 0;
	return (unsigned long)buf.st_size;
}

void download(int client_socket, char *file_name, long content_length)
{
	/*下载文件函数*/
	long hasrecieve = 0;//记录已经下载的长度
	SYSTEMTIME t_start, t_end;//记录一次读取的时间起点和终点, 计算速度
	int mem_size = 8192;//缓冲区大小8K
	int buf_len = mem_size;//理想状态每次读取8K大小的字节流
	int len;

	//创建文件描述符
	//int fd = open(file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
	HANDLE  hfile = CreateFile((PCTSTR)file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile < 0)
	{
		printf("文件创建失败!\n");
		exit(0);
	}

	char *buf = (char *)malloc(mem_size * sizeof(char));

	//从套接字流中读取文件流
	long diff = 0;
	int prelen = 0;
	double speed;

	while (hasrecieve < content_length)
	{
		//gettimeofday(&t_start, NULL); //获取开始时间
		GetLocalTime(&t_start);
		//len = read(client_socket, buf, buf_len);
		len = recv(client_socket, buf, buf_len, 0);
		//write(fd, buf, len);
		SetFilePointer(hfile, 0, 0, FILE_BEGIN);  //将句柄指到文件首
		DWORD dwWritten;     //保存写了多少字节到文件中
		WriteFile(hfile, buf, len, &dwWritten, 0);
		//gettimeofday(&t_end, NULL); //获取结束时间
		GetLocalTime(&t_end);

		hasrecieve += len;//更新已经下载的长度

		//计算速度
		if (t_end.wMilliseconds - t_start.wMilliseconds >= 0 && t_end.wSecond - t_start.wSecond >= 0)
			diff += 1000000 * (t_end.wSecond - t_start.wSecond) + (t_end.wMilliseconds - t_start.wMilliseconds);//us

		if (diff >= 1000000)//当一个时间段大于1s=1000000us时, 计算一次速度
		{
			speed = (double)(hasrecieve - prelen) / (double)diff * (1000000.0 / 1024.0);
			prelen = hasrecieve;//清零下载量
			diff = 0;//清零时间段长度
		}

		progress_bar(hasrecieve, content_length, speed);

		if (hasrecieve == content_length)
			break;
	}
}

int main(int argc, char const *argv[])
{
	/* 命令行参数: 接收两个参数, 第一个是下载地址, 第二个是文件的保存位置和名字, 下载地址是必须的, 默认下载到当前目录
	 * 示例: ./download http://www.baidu.com baidu.html
	 */
	char url[URL_LEN] = "127.0.0.1";//设置默认地址为本机,
	char remote_host_name[REMOTE_HOST_LEN] = { 0 };//远程主机地址
	char remote_ip_addr[REMOTE_IP_LEN] = { 0 };//远程主机IP地址
	int port = 80;//远程主机端口, http默认80端口
	char file_name[FILE_NAME_LEN] = { 0 };//下载文件名

	if (argc == 1)
	{
		printf("您必须给定一个http地址才能开始工作\n");
		exit(0);
	}
	else
		strcpy(url, argv[1]);

	puts("1: 正在解析下载地址...");
	parse_url(url, remote_host_name, &port, file_name);//从url中分析出主机名, 端口号, 文件名

	if (argc == 3)
	{
		printf("\t您已经将下载文件名指定为: %s\n", argv[2]);
		strcpy(file_name, argv[2]);
	}

	puts("2: 正在获取远程服务器IP地址...");
	get_ip_addr(remote_host_name, remote_ip_addr);//调用函数同访问DNS服务器获取远程主机的IP
	if (strlen(remote_ip_addr) == 0)
	{
		printf("错误: 无法获取到远程服务器的IP地址, 请检查下载地址的有效性\n");
		return 0;
	}

	puts("\n>>>>下载地址解析成功<<<<");
	printf("\t下载地址: %s\n", url);
	printf("\t远程主机: %s\n", remote_host_name);
	printf("\tIP 地 址: %s\n", remote_ip_addr);
	printf("\t主机PORT: %d\n", port);
	printf("\t 文件名 : %s\n\n", file_name);

	//设置http请求头信息
	char header[2048] = { 0 };
	sprintf(header, \
		"GET %s HTTP/1.1\r\n"\
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"\
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n"\
		"Host: %s\r\n"\
		"Connection: keep-alive\r\n"\
		"\r\n"\
		, url, remote_host_name);

	puts("3: 创建网络套接字...");
	//int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int client_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (client_socket < 0)
	{
		printf("套接字创建失败: %d\n", client_socket);
		exit(-1);
	}

	//创建IP地址结构体
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(remote_ip);
	addr.sin_port = htons(port);

	//连接远程主机
	puts("4: 正在连接远程主机...");
	int res = connect(client_socket, (struct sockaddr *) &addr, sizeof(addr));
	if (res == -1)
	{
		printf("连接远程主机失败, error: %d\n", res);
		exit(-1);
	}

	puts("5: 正在发送http下载请求...");
	//write(client_socket, header, strlen(header));//write系统调用, 将请求header发送给服务器
	send(client_socket, header, strlen(header), 0);

	int mem_size = 4096*64;
	int length = 0;
	int len;
	char *buf = (char *)malloc((mem_size+1) * sizeof(char));
	char *response = (char *)malloc(mem_size * sizeof(char));

	//每次单个字符读取响应头信息
	puts("6: 正在解析http响应头...");
	//while ((len = read(client_socket, buf, 1)) != 0)
	while ((len = recv(client_socket, buf, mem_size, 0)) != 0)
	{
		if (length + len > mem_size)
		{
			//动态内存申请, 因为无法确定响应头内容长度
			//mem_size *= 2;
			char * temp = (char *)realloc(response, sizeof(char) * mem_size);
			if (temp == NULL)
			{
				printf("动态内存申请失败\n");
				exit(-1);
			}
			response = temp;
		}

		buf[len] = '\0';
		std::strcat(response, buf);

		//找到响应头的头部信息
		int flag = 0;
		for (int i = strlen(response) - 1; response[i] == '\n' || response[i] == '\r'; i--, flag++);
		if (flag == 4)//连续两个换行和回车表示已经到达响应头的头尾, 即将出现的就是需要下载的内容
			break;

		length += len;
	}

	struct HTTP_RES_HEADER resp = parse_header(response);

	printf("\n>>>>http响应头解析成功:<<<<\n");

	printf("\tHTTP响应代码: %d\n", resp.status_code);
	if (resp.status_code != 200)
	{
		printf("文件无法下载, 远程主机返回: %d\n", resp.status_code);
		return 0;
	}
	printf("\tHTTP文档类型: %s\n", resp.content_type);
	printf("\tHTTP主体长度: %ld字节\n\n", resp.content_length);


	printf("7: 开始文件下载...\n");
	download(client_socket, file_name, resp.content_length);
	printf("8: 关闭套接字\n");

	if (resp.content_length == get_file_size(file_name))
		printf("\n文件%s下载成功! ^_^\n\n", file_name);
	else
	{
		remove(file_name);
		printf("\n文件下载中有字节缺失, 下载失败, 请重试!\n\n");
	}
	shutdown(client_socket, 2);//关闭套接字的接收和发送
	return 0;
}
