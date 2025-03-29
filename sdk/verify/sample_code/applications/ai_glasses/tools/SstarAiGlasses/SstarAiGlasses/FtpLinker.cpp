#include "pch.h"
#include "FtpLinker.h"

const CString LOCAL_ROOT_PATH = L"download";

FtpLinker::FtpLinker(const CString& ip, unsigned int port)
	:m_ip(ip), m_port(port), m_ftp_connection(nullptr)
{

}
FtpLinker::~FtpLinker()
{

}

FtpLinker::FtpFileItem::FtpFileItem(const CString& name, const CTime& time, ULONGLONG size, bool is_dir, bool is_local)
	: m_name(name), m_time(time), m_size(size), m_is_dir(is_dir), m_is_local(is_local)
{
}
FtpLinker::FtpFileItem::FtpFileItem(const FtpFileItem& item)
	: m_name(item.m_name), m_time(item.m_time), m_size(item.m_size), m_is_dir(item.m_is_dir), m_is_local(item.m_is_local)
{

}
void FtpLinker::FtpFileItem::GetItemTitleList(CArray<CString>& item_title_list)
{
	item_title_list.Add(L"名称");
	item_title_list.Add(L"创建日期");
	item_title_list.Add(L"大小");
	item_title_list.Add(L"状态");
}
void FtpLinker::FtpFileItem::GetItemFieldList(CArray<CString>& item_field_list)
{
	if (this->m_name == "..")
	{
		// 上一级目录
		item_field_list.Add(L"..(返回上一级)");
		return;
	}
	CString time_str;
	CString size_str;
	time_str.Format(L"%d-%d-%d %d:%d:%d",
		this->m_time.GetYear(), this->m_time.GetMonth(), this->m_time.GetDay(),
		this->m_time.GetHour(), this->m_time.GetMinute(), this->m_time.GetSecond());
	size_str.Format(L"%llu Byte", this->m_size);
	if (this->m_is_dir)
	{
		item_field_list.Add(L"> " + this->m_name);
	}
	else
	{
		item_field_list.Add(L"  " + this->m_name);
	}
	item_field_list.Add(time_str);
	item_field_list.Add(size_str);
	item_field_list.Add(this->m_is_local ? L"已下载" : L"未下载");
}
const CString& FtpLinker::FtpFileItem::GetName() const
{
	return this->m_name;
}
const CTime& FtpLinker::FtpFileItem::GetTime() const
{
	return this->m_time;
}
const ULONGLONG& FtpLinker::FtpFileItem::GetSize() const
{
	return this->m_size;
}
bool FtpLinker::FtpFileItem::IsDir() const
{
	return this->m_is_dir;
}
bool FtpLinker::FtpFileItem::IsLocal() const
{
	return this->m_is_local;
}

bool FtpLinker::Connect()
{
	unsigned int retryLimit = 5;
	while (!this->m_ftp_connection && retryLimit)
	{
		try
		{
			this->m_ftp_connection = internetSession.GetFtpConnection(this->m_ip, NULL, NULL, this->m_port);
		}
		catch (CInternetException* pEx)
		{
			pEx->Delete();
		}
		--retryLimit;
		Sleep(1);
	};
	return this->m_ftp_connection;
}
void FtpLinker::Disconnect()
{
	if (!this->m_ftp_connection)
	{
		return;
	}
	delete this->m_ftp_connection;
	this->m_ftp_connection = nullptr;
}
CArray<FtpLinker::FtpFileItem>& FtpLinker::List()
{
	this->m_file_items.RemoveAll();
	if (!this->m_ftp_connection)
	{
		return this->m_file_items;
	}

	CString curr_dir;
	if (!this->m_ftp_connection->GetCurrentDirectory(curr_dir))
	{
		AfxMessageBox(L"获取当前目录失败，请检查网络连接");
		return this->m_file_items;
	}
	curr_dir.ReleaseBuffer();

	CList<FtpFileItem> dir_lst;
	CList<FtpFileItem> file_lst;

	CFtpFileFind finder(this->m_ftp_connection);
	BOOL has_any_file = finder.FindFile(L"*");
	while (has_any_file)
	{
		has_any_file = finder.FindNextFile();
		if (finder.IsDirectory())
		{
			this->AddItemByTime(finder, dir_lst, false);
		}
		else
		{
			this->AddItemByTime(finder, file_lst, false);
		}
	}

	CFileFind local_finder;
	has_any_file = local_finder.FindFile(LOCAL_ROOT_PATH + curr_dir + L"/*");
	while (has_any_file)
	{
		has_any_file = local_finder.FindNextFile();
		if (!local_finder.IsDirectory() && !local_finder.IsDots())
		{
			this->AddItemByTime(local_finder, file_lst, true);
		}
	}

	if (curr_dir != L"/")
	{
		this->m_file_items.Add(FtpFileItem(L"..", 0, 0, true));
	}
	for (POSITION pos = dir_lst.GetHeadPosition(); pos != NULL; dir_lst.GetNext(pos))
	{
		this->m_file_items.Add(dir_lst.GetAt(pos));
	}
	for (POSITION pos = file_lst.GetHeadPosition(); pos != NULL; file_lst.GetNext(pos))
	{
		this->m_file_items.Add(file_lst.GetAt(pos));
	}
	return this->m_file_items;
}
bool FtpLinker::IsDir(unsigned int index)
{
	if (!this->m_ftp_connection)
	{
		return false;
	}
	const FtpFileItem& item = this->m_file_items.GetAt(index);
	return item.IsDir();
}
bool FtpLinker::Chdir(unsigned int index)
{
	if (!this->m_ftp_connection)
	{
		return false;
	}
	const FtpFileItem& item = this->m_file_items.GetAt(index);
	CString curr_dir;
	if (!this->m_ftp_connection->GetCurrentDirectory(curr_dir))
	{
		AfxMessageBox(L"获取当前目录失败，请检查网络连接");
		return false;
	}
	curr_dir.ReleaseBuffer();
	if (item.GetName() == ".." && curr_dir != "/")
	{
		int index = curr_dir.ReverseFind(L'/');
		if (index == -1)
		{
			return false;
		}
		return this->m_ftp_connection->SetCurrentDirectory(curr_dir.Left(index + 1));
	}
	return this->m_ftp_connection->SetCurrentDirectory(item.GetName());
}
bool FtpLinker::Get(unsigned int index, CString &local_file_path)
{
	if (!this->m_ftp_connection)
	{
		return false;
	}
	const FtpFileItem& item = this->m_file_items.GetAt(index);
	CString curr_dir;
	if (!this->m_ftp_connection->GetCurrentDirectory(curr_dir))
	{
		AfxMessageBox(L"获取当前目录失败，请检查网络连接");
		return false;
	}
	curr_dir.ReleaseBuffer(); // MUST, or can't join with others by `+`
	if (!this->Mkdir(LOCAL_ROOT_PATH + curr_dir))
	{
		AfxMessageBox(L"创建本地文件夹失败 " + LOCAL_ROOT_PATH + curr_dir);
		return false;
	}
	local_file_path = LOCAL_ROOT_PATH + curr_dir + L"/" + item.GetName();
	if (PathFileExists(local_file_path))
	{
		return true;
	}
	BOOL ret = this->m_ftp_connection->GetFile(curr_dir + L"/" + item.GetName(),
		local_file_path);
	if (!ret)
	{
		return false;
	}
	this->m_ftp_connection->Remove(curr_dir + L"/" + item.GetName());
	return true;
}
void FtpLinker::AddItemByTime(CFileFind& finder, CList<FtpFileItem>& item_list, bool is_local)
{
	CString name = finder.GetFileName();
	CTime time;
	finder.GetCreationTime(time);
	ULONGLONG size = finder.GetLength();
	bool is_dir = finder.IsDirectory();

	for (POSITION pos = item_list.GetHeadPosition(); pos != NULL; item_list.GetNext(pos))
	{
		const FtpLinker::FtpFileItem& item = item_list.GetAt(pos);
		if (time > item.GetTime())
		{
			item_list.InsertBefore(pos, FtpFileItem(name, time, size, is_dir, is_local));
			return;
		}
	}
	item_list.AddTail(FtpFileItem(name, time, size, is_dir, is_local));
}
bool FtpLinker::Mkdir(const CString& path)
{
	int index = 0;
	do
	{
		CString sub_path = path;
		index = path.Find(L"/", index + 1);
		if (index != -1)
		{
			sub_path = path.Left(index + 1);
		}
		if (!PathFileExists(sub_path))
		{
			if (!CreateDirectory(sub_path, 0))
			{
				return false;
			}
		}
	} while(-1 != index);
	return true;
}