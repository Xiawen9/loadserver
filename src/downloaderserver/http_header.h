#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

struct HTTP_RES_HEADER//保持相应头信息
{
	int status_code;//HTTP/1.1 '200' OK
	char content_type[128];//Content-Type: application/gzip
	long content_length;//Content-Length: 11683079
};

#endif // HTTP_HEADER_H

