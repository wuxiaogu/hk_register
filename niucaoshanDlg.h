
// niucaoshanDlg.h: 头文件
//

#pragma once
#define WM_MY_MESSAGE (WM_USER+100)
#define WM_DELETE_MESSAGE (WM_USER+101)
#define WM_CLOSEZD_MESSAGE (WM_USER+102)

// CniucaoshanDlg 对话框
class CniucaoshanDlg : public CDialogEx
{
// 构造
public:
	CniucaoshanDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NIUCAOSHAN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeleteMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseZdMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CListCtrl m_ListCrl;
	CStatic IDC;
	CStatic IDC_CARNUM;
	CStatic IDC_REG;
	CStatic IDC_BOX9;
	CStatic IDC_BOX10;
	CStatic IDC_STATIC11;
	CStatic IDC_STA12;
	UINT_PTR timer;
};
