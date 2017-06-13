#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Winsock2.h>
#include <Ws2ipdef.h>

#ifndef TCP_FASTOPEN
#define TCP_FASTOPEN 15
#endif

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	WSADATA wsaData = { 0 };
	int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc != 0) {
		printf("WSAStartup failed with error: %d\n", rc);
		return EXIT_FAILURE;
	}

	SOCKET s;
	do {
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET)
			break;

		SOCKADDR_IN si = { 0 };
		si.sin_family = AF_INET;
		si.sin_addr.s_addr = inet_addr("127.0.0.1");
		si.sin_port = htons(23333);

		int rc = bind(s, (SOCKADDR *)&si, sizeof(si));
		if (rc != 0)
			break;

		BOOL option = TRUE;
		rc = setsockopt(s, IPPROTO_TCP, TCP_FASTOPEN, (const char *)&option, sizeof(option));
		if (rc != 0)
			break;

		rc = listen(s, SOMAXCONN);
		if (rc != 0)
			break;

		while (1) {
			SOCKET client = accept(s, NULL, NULL);
			if (client == INVALID_SOCKET)
				break;

			char buf[16];
			size_t len = recv(client, (char *)&buf, sizeof(buf), 0);
			if (len != 0) {
				printf("recv: %s\n", buf);
			}

			closesocket(client);
		}
	} while (0);
	if (s != INVALID_SOCKET)
		closesocket(s);
}
