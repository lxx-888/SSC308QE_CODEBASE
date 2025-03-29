#pragma once
class WifiLinker
{
public:
	enum Mode
	{
		WIFI_LINKER_MODE_NORMAL,
		WIFI_LINKER_MODE_VIDEO,
		WIFI_LINKER_MODE_IMAGE,
		WIFI_LINKER_MODE_WAKEUP,
		WIFI_LINKER_MODE_SLEEP,
	};
	struct ModeDefine
	{
		Mode mode;
		CString name;
		ModeDefine(Mode mode = WIFI_LINKER_MODE_VIDEO, const CString &name = L"")
			: mode(mode), name(name) {}
		ModeDefine(const ModeDefine& define)
			: mode(define.mode), name(define.name) {}
	};
public:
	WifiLinker(const CString &ip, unsigned int port, Mode mode);
	~WifiLinker();

	static void GetModeList(CArray<WifiLinker::ModeDefine>& mode_list);
	bool WakeUp();
	bool Sleep();

private:
	SOCKET m_sock;
	CString m_ip;
	unsigned int m_port;
	Mode m_mode;
};