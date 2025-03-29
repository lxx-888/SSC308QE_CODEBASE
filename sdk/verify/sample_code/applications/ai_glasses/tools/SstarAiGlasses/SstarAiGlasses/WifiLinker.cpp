#include "pch.h"
#include "WifiLinker.h"
#include "WinSock2.h"
#include <WS2tcpip.h>

#include <afxpriv.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

WifiLinker::WifiLinker(const CString& ip, unsigned int port, WifiLinker::Mode mode)
	: m_ip(ip), m_port(port), m_mode(mode)
{
}
WifiLinker::~WifiLinker()
{
}

void WifiLinker::GetModeList(CArray<WifiLinker::ModeDefine> &mode_list)
{
	mode_list.Add(ModeDefine(WIFI_LINKER_MODE_VIDEO, L"1取视频"));
	mode_list.Add(ModeDefine(WIFI_LINKER_MODE_IMAGE, L"2取图像"));
	mode_list.Add(ModeDefine(WIFI_LINKER_MODE_WAKEUP, L"3仅唤醒"));
	mode_list.Add(ModeDefine(WIFI_LINKER_MODE_SLEEP, L"4进入休眠"));
}

bool WifiLinker::WakeUp()
{
	CString msg;
	msg.Format(L"TODO: Wake up wifi %s:%u with mode %d", this->m_ip.GetString(), 333, this->m_mode);
	AfxMessageBox(msg, MB_ICONASTERISK);

	// ip convert
	int n = m_ip.GetLength();
	int len = WideCharToMultiByte(CP_ACP, 0, m_ip, n, NULL, 0, NULL, NULL);
	char* ip_srv = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, m_ip, n, ip_srv, len, NULL, NULL);
	ip_srv[len] = '\0';

	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		AfxMessageBox(L"WSAStartup fail", MB_ICONHAND);
		return false;
	}

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == this->m_sock)
	{
		AfxMessageBox(L"socket fail", MB_ICONHAND);
		WSACleanup();
		return false;
	}

	SOCKADDR_IN srv_addr;
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(333);
	srv_addr.sin_addr.S_un.S_addr = inet_addr(ip_srv);
	AfxMessageBox(msg, MB_ICONASTERISK);

	if (connect(this->m_sock, (sockaddr*)&srv_addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		msg.Format(L"connect fail %d", WSAGetLastError());
		AfxMessageBox(msg, MB_ICONHAND);
		closesocket(m_sock);
		WSACleanup();
		return false;
	}

	if (send(this->m_sock, (char*)(&m_mode), strlen((char*)(&m_mode)), 0) < 0)
	{
		AfxMessageBox(L"send fail", MB_ICONHAND);
		closesocket(m_sock);
		WSACleanup();
		return false;
	}

	closesocket(m_sock);

	WSACleanup();

	delete[] ip_srv;
	ip_srv = NULL;

	return true;
}

bool WifiLinker::Sleep()
{
	CString msg;
	msg.Format(L"TODO: Sleep wifi %s:%u", this->m_ip.GetString(), this->m_port);
	AfxMessageBox(msg, MB_ICONASTERISK);

	this->m_mode = WIFI_LINKER_MODE_SLEEP;

	// ip convert
	int n = m_ip.GetLength();
	int len = WideCharToMultiByte(CP_ACP, 0, m_ip, n, NULL, 0, NULL, NULL);
	char* ip_srv = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, m_ip, n, ip_srv, len, NULL, NULL);
	ip_srv[len] = '\0';

	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		AfxMessageBox(L"WSAStartup fail", MB_ICONHAND);
		return false;
	}

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == this->m_sock)
	{
		AfxMessageBox(L"socket fail", MB_ICONHAND);
		WSACleanup();
		return false;
	}

	SOCKADDR_IN srv_addr;
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(333);
	srv_addr.sin_addr.S_un.S_addr = inet_addr(ip_srv);
	AfxMessageBox(msg, MB_ICONASTERISK);

	if (connect(this->m_sock, (sockaddr*)&srv_addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		msg.Format(L"connect fail %d", WSAGetLastError());
		AfxMessageBox(msg, MB_ICONHAND);
		closesocket(m_sock);
		WSACleanup();
		return false;
	}

	if (send(this->m_sock, (char*)(&m_mode), strlen((char*)(&m_mode)), 0) < 0)
	{
		AfxMessageBox(L"send fail", MB_ICONHAND);
		closesocket(m_sock);
		WSACleanup();
		return false;
	}

	closesocket(m_sock);

	WSACleanup();

	delete[] ip_srv;
	ip_srv = NULL;
	closesocket(m_sock);
}