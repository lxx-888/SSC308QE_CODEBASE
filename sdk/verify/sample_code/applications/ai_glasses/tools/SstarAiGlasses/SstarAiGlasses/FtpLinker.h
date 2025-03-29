#pragma once
class FtpLinker
{
public:
	class FtpFileItem
	{
	public:
		FtpFileItem(const CString& name = L"", const CTime& time = 0, ULONGLONG size = 0, bool is_dir = false, bool is_local = false);
		FtpFileItem(const FtpFileItem& item);
		static void GetItemTitleList(CArray<CString>& item_title_list);
		void GetItemFieldList(CArray<CString>& item_field_list);
		const CString& GetName() const;
		const CTime& GetTime() const;
		const ULONGLONG& GetSize() const;
		bool IsDir() const;
		bool IsLocal() const;
	private:
		CString m_name;
		CTime m_time;
		ULONGLONG m_size;
		bool m_is_dir;
		bool m_is_local;
	};
public:
	FtpLinker(const CString& ip, unsigned int port);
	~FtpLinker();

	bool Connect();
	void Disconnect();

	CArray<FtpFileItem>& List();
	bool IsDir(unsigned int index);
	bool Chdir(unsigned int index);
	bool Get(unsigned int index, CString& local_file_path);
private:
	void AddItemByTime(CFileFind& finder, CList<FtpFileItem>& item_list, bool is_local);
	bool Mkdir(const CString& dir);
private:
	CString m_ip;
	unsigned int m_port;
private:
	CInternetSession internetSession;
	CFtpConnection* m_ftp_connection;
	CArray<FtpFileItem> m_file_items;
};