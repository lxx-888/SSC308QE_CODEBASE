
// SstarAiGlassesDlg.h: 头文件
//

#pragma once
#include "FtpLinker.h"
#include "WifiLinker.h"

// CSstarAiGlassesDlg 对话框
class CSstarAiGlassesDlg : public CDialogEx
{
// 构造
public:
	CSstarAiGlassesDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SSTARAIGLASSES_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	virtual void OnOK();
	virtual void OnCancel();
private:
	bool GetIpAddress(CString& ip_str);
	bool GetLinkMode(WifiLinker::Mode &mode);
	void RefreshFileList();
	void OpenFileWithFFplay(const CString& filePath);
public:
	afx_msg void OnClickedButtonLink();
	afx_msg void OnClickedButtonUnlink();
	afx_msg void OnDblclkListFtp(NMHDR* pNMHDR, LRESULT* pResult);
private:
	CIPAddressCtrl m_ip_addr_ctrl;
	CEdit m_link_port_edit;
	CEdit m_ftp_port_edit;
	CComboBox m_link_mode_combo;
	CButton m_link_button;
	CButton m_unlink_button;
	CListCtrl m_ftp_list;

	unsigned int m_link_port;
	unsigned int m_ftp_port;

	FtpLinker* m_ftp_linker;
	WifiLinker* m_wifi_linker;
};
