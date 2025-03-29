
// SstarAiGlassesDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SstarAiGlasses.h"
#include "SstarAiGlassesDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSstarAiGlassesDlg 对话框



CSstarAiGlassesDlg::CSstarAiGlassesDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SSTARAIGLASSES_DIALOG, pParent)
	, m_link_port(1024)
	, m_ftp_port(21)
	, m_ftp_linker(nullptr)
	, m_wifi_linker(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSstarAiGlassesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOARD_IP, m_ip_addr_ctrl);
	DDX_Control(pDX, IDC_BOARD_PORT, m_link_port_edit);
	DDX_Control(pDX, IDC_FTP_PORT, m_ftp_port_edit);
	DDX_Control(pDX, IDC_BUTTON_LINK, m_link_button);
	DDX_Control(pDX, IDC_BUTTON_UNLINK, m_unlink_button);
	DDX_Control(pDX, IDC_COMBO_LINK_MODE, m_link_mode_combo);
	DDX_Control(pDX, IDC_LIST_FTP, m_ftp_list);
	DDX_Text(pDX, IDC_BOARD_PORT, m_link_port);
	DDX_Text(pDX, IDC_FTP_PORT, m_ftp_port);
}

BEGIN_MESSAGE_MAP(CSstarAiGlassesDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LINK, &CSstarAiGlassesDlg::OnClickedButtonLink)
	ON_BN_CLICKED(IDC_BUTTON_UNLINK, &CSstarAiGlassesDlg::OnClickedButtonUnlink)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FTP, &CSstarAiGlassesDlg::OnDblclkListFtp)
END_MESSAGE_MAP()


// CSstarAiGlassesDlg 消息处理程序

BOOL CSstarAiGlassesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	this->m_ip_addr_ctrl.SetAddress(192, 168, 30, 1);

	this->m_unlink_button.EnableWindow(FALSE);
	this->m_ftp_list.EnableWindow(FALSE);

	// 连接模式下拉框设定
	CArray<WifiLinker::ModeDefine> mode_list;
	WifiLinker::GetModeList(mode_list);
	for (unsigned int i = 0; i < mode_list.GetCount(); ++i)
	{
		this->m_link_mode_combo.AddString(mode_list[i].name);
	}
	this->m_link_mode_combo.SetCurSel(0);

	// 文件列表表头设定
	this->m_ftp_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	CArray<CString> title_list;
	FtpLinker::FtpFileItem::GetItemTitleList(title_list);
	for (unsigned int i = 0; i < title_list.GetCount(); ++i)
	{
		this->m_ftp_list.InsertColumn(i, title_list[i], LVCFMT_LEFT, 200);
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSstarAiGlassesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSstarAiGlassesDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSstarAiGlassesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSstarAiGlassesDlg::OnClickedButtonLink()
{
	ASSERT(!this->m_ftp_linker);
	ASSERT(!this->m_wifi_linker);
	CString ip_str;
	WifiLinker::Mode mode;
	if (!this->GetIpAddress(ip_str) || !this->GetLinkMode(mode))
	{
		return;
	}

	this->m_wifi_linker = new WifiLinker(ip_str, this->m_link_port, mode);
	if (!this->m_wifi_linker->WakeUp())
	{
		AfxMessageBox(L"Wifi 唤醒失败", MB_ICONHAND);
		goto ERR_WIFI_WAKE_UP;
	}

	this->m_ftp_linker = new FtpLinker(ip_str, this->m_ftp_port);
	if (!this->m_ftp_linker->Connect())
	{
		AfxMessageBox(L"Ftp 连接失败", MB_ICONHAND);
		goto ERR_FTP_CONNECT;
	}

	this->m_ip_addr_ctrl.EnableWindow(FALSE);
	this->m_ftp_port_edit.EnableWindow(FALSE);
	this->m_link_port_edit.EnableWindow(FALSE);
	this->m_link_button.EnableWindow(FALSE);
	this->m_link_mode_combo.EnableWindow(FALSE);
	this->m_ftp_list.EnableWindow(TRUE);
	this->m_unlink_button.EnableWindow(TRUE);
	this->RefreshFileList();
	return;

ERR_FTP_CONNECT:
	delete this->m_ftp_linker;
	this->m_ftp_linker = nullptr;
ERR_WIFI_WAKE_UP:
	delete this->m_wifi_linker;
	this->m_wifi_linker = nullptr;
}


void CSstarAiGlassesDlg::OnClickedButtonUnlink()
{
	ASSERT(this->m_ftp_linker);
	ASSERT(this->m_wifi_linker);

	this->m_ftp_linker->Disconnect();
	delete this->m_ftp_linker;
	this->m_ftp_linker = nullptr;

	this->m_wifi_linker->Sleep();
	delete this->m_wifi_linker;
	this->m_wifi_linker = nullptr;

	this->RefreshFileList();
	this->m_ip_addr_ctrl.EnableWindow(TRUE);
	this->m_ftp_port_edit.EnableWindow(TRUE);
	this->m_link_port_edit.EnableWindow(TRUE);
	this->m_link_button.EnableWindow(TRUE);
	this->m_link_mode_combo.EnableWindow(TRUE);
	this->m_ftp_list.EnableWindow(FALSE);
	this->m_unlink_button.EnableWindow(FALSE);
}

bool CSstarAiGlassesDlg::GetIpAddress(CString& ip_str)
{
	BYTE field0 = 0, field1 = 0, field2 = 0, field3 = 0;
	if (4 != this->m_ip_addr_ctrl.GetAddress(field0, field1, field2, field3))
	{
		AfxMessageBox(L"ip 地址格式不符合要求");
		return false;
	}
	ip_str.Format(L"%d.%d.%d.%d", field0, field1, field2, field3);
	return true;
}

bool CSstarAiGlassesDlg::GetLinkMode(WifiLinker::Mode& mode)
{
	int sel = this->m_link_mode_combo.GetCurSel();
	CArray<WifiLinker::ModeDefine> mode_list;
	WifiLinker::GetModeList(mode_list);
	if (sel >= mode_list.GetCount())
	{
		AfxMessageBox(L"连接模式选择错误");
		return false;
	}
	mode = mode_list[sel].mode;
	return true;
}

void CSstarAiGlassesDlg::RefreshFileList()
{
	this->m_ftp_list.DeleteAllItems();
	if (!this->m_ftp_linker)
	{
		return;
	}
	CArray<FtpLinker::FtpFileItem>& file_list = this->m_ftp_linker->List();
	for (unsigned int row = 0; row < file_list.GetCount(); ++row)
	{
		CArray<CString> field_list;
		file_list[row].GetItemFieldList(field_list);
		this->m_ftp_list.InsertItem(row, L"");
		for (unsigned int col = 0; col < field_list.GetCount(); ++col)
		{
			this->m_ftp_list.SetItemText(row, col, field_list[col].GetString());
		}
	}
}
void CSstarAiGlassesDlg::OpenFileWithFFplay(const CString& file_path)
{
	CString window_style_file_path = file_path;
	window_style_file_path.Replace(L"/", L"\\");

	RECT rect;
	this->GetWindowRect(&rect);

	WCHAR cmd_line[1024];
	ZeroMemory(cmd_line, sizeof(cmd_line));
	wsprintf(cmd_line, L".\\ffplay.exe -left %ld -top %ld -x %ld -y %ld %s",
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		window_style_file_path.GetString());

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	if (!CreateProcess(NULL, cmd_line, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		DWORD dwError = GetLastError();
		CString errorMessage;
		errorMessage.Format(L"无法打开 ffplay，请确保程序执行目录下有 ffplay.exe 文件. 错误代码： %lu", dwError);
		AfxMessageBox(errorMessage);
		return;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}
void CSstarAiGlassesDlg::OnDblclkListFtp(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	if (this->m_ftp_linker->IsDir(pNMItemActivate->iItem))
	{
		this->m_ftp_linker->Chdir(pNMItemActivate->iItem);
		this->RefreshFileList();
		return;
	}

	CString local_file_path;
	if (!this->m_ftp_linker->Get(pNMItemActivate->iItem, local_file_path))
	{
		AfxMessageBox(L"文件下载失败", MB_ICONHAND);
		this->RefreshFileList();
		return;
	}
	this->OpenFileWithFFplay(local_file_path);
	this->RefreshFileList();
}


void CSstarAiGlassesDlg::OnOK()
{
	if (!this->m_ftp_linker && !this->m_wifi_linker)
	{
		this->OnClickedButtonLink();
	}
}


void CSstarAiGlassesDlg::OnCancel()
{
	if (this->m_ftp_linker && this->m_wifi_linker)
	{
		this->OnClickedButtonUnlink();
	}
	CDialogEx::OnCancel();
}
