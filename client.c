#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Winsock2.h>
#include <Ws2ipdef.h>
#include <Mswsock.h>

#ifndef TCP_FASTOPEN
#define TCP_FASTOPEN 15
#endif

#pragma comment(lib, "Ws2_32.lib")

void client()
{
	SOCKET s;
	do {
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET)
			break;

		SOCKADDR addr = { AF_INET };
		int rc = bind(s, &addr, sizeof(addr));
		if (rc != 0)
			break;

		BOOL option = TRUE;
		rc = setsockopt(s, IPPROTO_TCP, TCP_FASTOPEN, (const char *)&option, sizeof(option));
		if (rc != 0)
			break;

		SOCKADDR_IN si = { 0 };
		si.sin_family = AF_INET;
		si.sin_addr.s_addr = inet_addr("127.0.0.1");
		si.sin_port = htons(23333);

		GUID guid = WSAID_CONNECTEX;
		LPFN_CONNECTEX ConnectEx = 0;
		DWORD retBytes = 0;
		rc = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &ConnectEx, sizeof(ConnectEx), &retBytes, NULL, NULL);
		if (rc != 0)
			break;

		char buf[] = "Test!";
		DWORD sentBytes = 0;
		OVERLAPPED ol = { 0 };
		BOOL ret = ConnectEx(s, (SOCKADDR *)&si, sizeof(si), buf, sizeof(buf), &sentBytes, &ol);
		if (ret) {
			printf("ConnectEx succeeded.\n");
		} else {
			if (WSAGetLastError() == ERROR_IO_PENDING) {
				printf("ConnectEx pending.\n");

				ret = GetOverlappedResult((HANDLE)s, &ol, &retBytes, TRUE);
				if (ret)
					printf("ConnectEx succeeded.\n");
				else
					printf("ConnectEx failed: %d\n", WSAGetLastError());
			} else {
				printf("ConnectEx failed: %d\n", WSAGetLastError());
			}
		}

		int optlen = sizeof(option);
		rc = getsockopt(s, IPPROTO_TCP, TCP_FASTOPEN, (char *)&option, &optlen);
		printf("TCP_FASTOPEN: %d\n", option);

	} while (0);

	if (s != INVALID_SOCKET)
		closesocket(s);
}

int main()
{
	WSADATA wsaData = { 0 };
	int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc != 0) {
		printf("WSAStartup failed with error: %d\n", rc);
		return EXIT_FAILURE;
	}

	client();
	Sleep(1000);
	client();

	return 0;
}
