
// niucaoshanDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "niucaoshan.h"
#include "niucaoshanDlg.h"
#include "afxdialogex.h"


#include <stdio.h>
#include <iostream>
#include <fstream> 
#include <string>
#include <streambuf> 
#include "Windows.h"
#include "HCNetSDK.h"
#include <time.h>
#include <conio.h>//getch()函数用
#include<curl\curl.h>
#include <json\json.h>

#pragma comment(lib,"HCNetSDK.lib")
#pragma comment(lib,"PlayCtrl.lib")
#pragma comment(lib,"GdiPlus.lib")
#pragma comment(lib,"HCCore.lib")
#pragma comment(lib,"libcurl_imp.lib")
#pragma comment(lib,"jsoncpp.lib")

using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//参数声明
int iNum = 0; 	//图片名称序号
LONG IUserID;	//摄像机设备
LONG IHandle = -1;//报警布防/撤防;
NET_DVR_DEVICEINFO_V30 struDeviceInfo;	//设备信息

char sDVRIP[20] = "192.168.18.5";	//抓拍摄像机设备IP地址 牛草山IP

//char sDVRIP[20] = "192.168.1.2";
short wDVRPort = 8000;	//设备端口号
char sUserName[20] = "admin";	  //登录的用户名
char sPassword[20] = "ahyx1688";  //用户密码
string carNum;//车牌号							
string LineByLine;//逐行读取文件 

CFont cfont;
CFont cfont1;
CFont cfont2;
CFont cfont3;

CFont cfont4;

struct  _tf_data
{
	CString name;
	CString cardno;
	CString phone;
	CString akm;
}TF_DATA;


//---------------------------------------------------------------------------------
//函数声明
void Init();//初始化
void Demo_SDK_Version(); //获取sdk版本
void Connect();//设置连接事件与重连时间
void Htime();//获取海康威视设备时间
bool Login(char* sDVRIP, short wDVRPort, char* sUserName, char* sPassword);//注册摄像机设备
void CALLBACK MSesGCallback(LONG ICommand, NET_DVR_ALARMER* pAlarmer, char* pAlarmInfo, DWORD dwBufLen, void* pUser);//报警回调函数
void SetMessageCallBack();//设置报警回调函数
void SetupAlarm();//报警布防
void CloseAlarm();//报警撤防
void OnExit(void);//退出
string CStringToString(CString cstr);


LRESULT CniucaoshanDlg::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	_tf_data* tf = (_tf_data*)wParam;

	int n = m_ListCrl.InsertItem(0, tf->name);
	m_ListCrl.SetItemText(n, 1, tf->cardno);
	m_ListCrl.SetItemText(n, 2, tf->phone);
	m_ListCrl.SetItemText(n, 3, tf->akm);

	CTime tm;
	tm = CTime::GetCurrentTime();
	CString timestr = tm.Format("现在时间是%Y年%m月%d日%X");

	// 显示时间
	IDC_STA12.SetWindowText(timestr);

	return 1;
}

LRESULT CniucaoshanDlg::OnDeleteMessage(WPARAM wParam, LPARAM lParam)
{
	m_ListCrl.DeleteAllItems();
	return 1;
}


LRESULT CniucaoshanDlg::OnCloseZdMessage(WPARAM wParam, LPARAM lParam)
{
	timer = SetTimer(1, 8000, NULL);
	return 1;
}


string UTF8ToGBK(const string& str_utf8) {
	int len = MultiByteToWideChar(CP_UTF8, 0, str_utf8.c_str(), -1, NULL, 0);
	wchar_t* wsz_gbk = new wchar_t[len + 1];
	memset(wsz_gbk, 0, (len + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_utf8.c_str(), -1, wsz_gbk, len);
	len = WideCharToMultiByte(CP_ACP, 0, wsz_gbk, -1, NULL, 0, NULL, NULL);
	char* sz_gbk = new char[len + 1];
	memset(sz_gbk, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wsz_gbk, -1, sz_gbk, len, NULL, NULL);
	string str_gbk(sz_gbk);
	delete[] sz_gbk;
	delete[] wsz_gbk;
	return str_gbk;
}


//---------------------------------------------------------------------------------------------------
//函数定义
//初始化

void Init()
{
	//获取系统时间
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	NET_DVR_Init();//初始化
	Demo_SDK_Version();//获取 SDK  的版本号和 build  信息	

	//CString str_v;
	//UINT uRet = GetPrivateProfileString(TEXT("System"), TEXT("version"), TEXT("fff"), str_v.GetBuffer(MAX_PATH), MAX_PATH, TEXT(".\\config.ini"));
	//string str_v1 = CStringToString(str_v);
	//printf("str_v:%s\r\n", str_v1.c_str());

}

//设置连接事件与重连时间
void Connect()
{
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
}
//获取海康威视设备时间
void Htime() {
	bool iRet;
	DWORD dwReturnLen;
	NET_DVR_TIME struParams = { 0 };

	iRet = NET_DVR_GetDVRConfig(IUserID, NET_DVR_GET_TIMECFG, 1, \
		& struParams, sizeof(NET_DVR_TIME), &dwReturnLen);
	if (!iRet)
	{
		AfxMessageBox(_T("NET_DVR_GetDVRConfig NET_DVR_GET_TIMECFG  error."));
		NET_DVR_Logout(IUserID);
		NET_DVR_Cleanup();
	}

}

//注册摄像机设备
bool Login(char* sDVRIP, short wDVRPort, char* sUserName, char* sPassword)
{
	IUserID = NET_DVR_Login_V30(sDVRIP, wDVRPort, sUserName, sPassword, &struDeviceInfo);

	if (IUserID < 0)
	{
		AfxMessageBox(_T("Login Failed!"));
		NET_DVR_Cleanup();
		return false;
	}
	else
	{	
		AfxMessageBox(_T("Login Successfully!"));
		return true;
	}

}

//Demo_SDK_Version()海康威视sdk版本获取函数
void Demo_SDK_Version()
{
	unsigned int uiVersion = NET_DVR_GetSDKBuildVersion();

	char strTemp[1024] = { 0 };
	sprintf_s(strTemp, "HCNetSDK V%d.%d.%d.%d\n", \
		(0xff000000 & uiVersion) >> 24, \
		(0x00ff0000 & uiVersion) >> 16, \
		(0x0000ff00 & uiVersion) >> 8, \
		(0x000000ff & uiVersion));
	printf(strTemp);
}

//定义异常消息回调函数
void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void* pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType)
	{
	case EXCEPTION_RECONNECT:    //预览时重连  
		printf("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}



/**
* 一旦curl接收到数据，就会调用此回调函数
* buffer:数据缓冲区指针
* size:调试阶段总是发现为1
* nmemb:(memory block)代表此次接受的内存块的长度
* userp:用户自定义的一个参数
*/
size_t ConfigData(char* ptr, size_t size, size_t nmemb, std::string& userdata)
{
	size_t rest = 0;
	LONG lsize = size * nmemb;
	std::string stemp(ptr, lsize);
	userdata += stemp;
	return lsize;
}


//只需一个一个赋值即可!!

string CStringToString(CString cstr)

{

	string result(cstr.GetLength(), 'e');

	for (int i = 0; i < cstr.GetLength(); i++)

	{

		result[i] = (char)cstr[i];

	}

	return result;

}

LPCWSTR string2LPCWSTR(std::string str)
{
	size_t size = str.length();
	int wLen = ::MultiByteToWideChar(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0);
	wchar_t* buffer = new wchar_t[wLen + 1];
	memset(buffer, 0, (wLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), size, (LPWSTR)buffer, wLen);
	return buffer;
}


// string到CString

CString StringToCString(string str)

{

	CString result;

	for (int i = 0; i < (int)str.length(); i++)

	{

		result += str[i];

	}

	return result;

}


//报警回调函数
void CALLBACK MSesGCallback(LONG lCommand, NET_DVR_ALARMER* pAlarmer, char* pAlarmInfo, DWORD dwBufLen, void* pUser)
{

	//获取系统时间
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	//cout << sys.wYear << "-" << sys.wMonth << "-" << sys.wDay << " " << sys.wHour << ":" << sys.wMinute << ":" << sys.wSecond << endl;

	int i = 0;


	switch (lCommand) {
		//交通抓拍结果(老报警消息)
	case COMM_UPLOAD_PLATE_RESULT: {
		break;
	}
								 //交通抓拍结果(新报警消息)
	case COMM_ITS_PLATE_RESULT: {
		NET_ITS_PLATE_RESULT struITSPlateResult = { 0 };
		memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));


		// 清空详情信息
	

		// 显示车牌号
		carNum = struITSPlateResult.struPlateInfo.sLicense;
		CWnd* carnum = AfxGetApp()->GetMainWnd()->GetDlgItem(IDC_STATIC8);
		carnum->SetWindowText(string2LPCWSTR(carNum));

		curl_global_init(CURL_GLOBAL_ALL); // 首先全局初始化CURL
		CURL* curl = curl_easy_init(); // 初始化CURL句柄

		std::string content;
		if (curl)
		{

			std::string strstl = carNum;
			printf("strstl:%s\r\n", strstl.c_str());

			char* escape_control = curl_escape(strstl.c_str(), strstl.size());
			strstl = escape_control;
			curl_free(escape_control);
			std::string site_url = "http://niucaoshan.yuehuio.com/manage/api.Booking/info?plate=" + strstl;
				

			struct curl_slist* head = NULL;
			head = curl_slist_append(head, "Content-Type:application/x-www-form-urlencoded;charset=UTF-8");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);

			curl_easy_setopt(curl, CURLOPT_URL, site_url);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ConfigData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);


			// 执行一次URL请求
			CURLcode res = curl_easy_perform(curl);

			curl_slist_free_all(head);//记得要释放
			curl_easy_cleanup(curl);

			std::string c = content.c_str();
				
			//CString cstr(c.c_str());
			//AfxMessageBox(cstr);

			printf("content:%s\r\n", c);

			Json::Reader reader;
			Json::Value root;

			if (reader.parse(c, root))
			{

				const Json::Value arrayObj = root["data"][0]["children"];
				int total = arrayObj.size();

				CString total1;
				total1.Format(_T("%d"), total);

				// 显示预约总人数
				CWnd* totalhwd = AfxGetApp()->GetMainWnd()->GetDlgItem(IDC_STATIC5);
				totalhwd->SetWindowText(total1);


				// string name;
				CWnd* wnd = AfxGetApp()->GetMainWnd();

				::SendMessage(*wnd, WM_DELETE_MESSAGE, 0, 0);

				for (unsigned int j = 0; j < arrayObj.size(); j++) {

					string name = UTF8ToGBK(arrayObj[j]["name"].asString());
					string cardno = UTF8ToGBK(arrayObj[j]["card_no"].asString());
					string phone = UTF8ToGBK(arrayObj[j]["phone"].asString());
					string akm = UTF8ToGBK(arrayObj[j]["akm_status"].asString());

					printf("name:%s\r\n", name.c_str());

					//int n = m_ListCrl.InsertItem(0, string2LPCWSTR(name));
					//m_ListCrl.SetItemText(n, 1, string2LPCWSTR(cardno));
					//m_ListCrl.SetItemText(n, 2, string2LPCWSTR(phone));
					_tf_data *tf = new _tf_data;
					tf->name = string2LPCWSTR(name);
					tf->cardno = string2LPCWSTR(cardno);
					tf->phone = string2LPCWSTR(phone);
					tf->akm = string2LPCWSTR(akm);

					::SendMessage(*wnd, WM_MY_MESSAGE, (WPARAM)tf, 0);

				}

				// 预约过了 自动开门
				if (total > 0) {
					// 开启道闸

					char szLan[128] = { 0 };
					NET_DVR_BARRIERGATE_CFG m_struGateCFG;
					m_struGateCFG.dwSize = sizeof(NET_DVR_BARRIERGATE_CFG);
					m_struGateCFG.dwChannel = 1;
					m_struGateCFG.byLaneNo = 1;
					m_struGateCFG.byBarrierGateCtrl = 1;
					if (!NET_DVR_RemoteControl(IUserID, NET_DVR_BARRIERGATE_CTRL, &m_struGateCFG, sizeof(NET_DVR_BARRIERGATE_CFG)))
					{
						printf("设置修改失败, Error code: %d", NET_DVR_GetLastError());
					}
					else {

						//::SendMessage(*wnd, WM_CLOSEZD_MESSAGE, 0, 0);
						
					}

				}

			}
			else {
				AfxMessageBox(_T("数据解析错误!"));
			}

		}


		break;
	}
	default: {
		//std::cout << lCommand << endl;
		break;
	}
	}

	return;
}


//设置报警回调函数
void SetMessageCallBack()
{
	NET_DVR_SetDVRMessageCallBack_V30(MSesGCallback, NULL);
}
//报警布防
void SetupAlarm()
{
	//启动布防
	NET_DVR_SETUPALARM_PARAM struSetupParam = { 0 };
	struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);


	struSetupParam.byAlarmInfoType = 1;//上传报警信息类型：0-老报警信息(NET_DVR_PLATE_RESULT), 1-新报警信息(NET_ITS_PLATE_RESULT)
	struSetupParam.byLevel = 1;//布防优先级：0- 一等级（高），1- 二等级（中），2- 三等级（低）
							   //bySupport 按位表示，值：0 - 上传，1 - 不上传;  bit0 - 表示二级布防是否上传图片;


	IHandle = NET_DVR_SetupAlarmChan_V41(IUserID, &struSetupParam);//建立报警上传通道，获取报警等信息。
	if (IHandle < 0)
	{
		std::cout << "NET_DVR_SetupAlarmChan_V41 Failed! Error number：" << NET_DVR_GetLastError() << std::endl;
		NET_DVR_Logout(IUserID);
		NET_DVR_Cleanup();
		return;
	}
	std::cout << "\n" << endl;

}
//报警撤防
void CloseAlarm()
{
	//撤销布防上传通道
	if (!NET_DVR_CloseAlarmChan_V30(IHandle))//布防句柄IHandle
	{
		std::cout << "NET_DVR_CloseAlarmChan_V30 Failed! Error number：" << NET_DVR_GetLastError() << std::endl;
		NET_DVR_Logout(IUserID);
		NET_DVR_Cleanup();
		return;
	}
	IHandle = -1;//布防句柄;
}
//退出
void OnExit(void)
{
	std::cout << "Begin exit..." << std::endl;

	//报警撤防
	CloseAlarm();

	//释放相机
	NET_DVR_Logout(IUserID);//注销用户
	NET_DVR_Cleanup();//释放SDK资源	
}


DWORD WINAPI ThreadProc1(
	LPVOID lpParameter   // thread data
)
{
	for (;;) {
		SetMessageCallBack();	//报警回调函数 				
		Sleep(500);
	}
	return 1;
}

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


// CniucaoshanDlg 对话框


CniucaoshanDlg::CniucaoshanDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NIUCAOSHAN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CniucaoshanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST3, m_ListCrl);
	DDX_Control(pDX, IDC_STATIC5, IDC);
	DDX_Control(pDX, IDC_STATIC8, IDC_CARNUM);
	DDX_Control(pDX, IDC_STATIC6, IDC_REG);
	DDX_Control(pDX, IDC_STATIC9, IDC_BOX9);
	DDX_Control(pDX, IDC_STATIC10, IDC_BOX10);
	DDX_Control(pDX, IDC_STATIC12, IDC_STA12);
}

BEGIN_MESSAGE_MAP(CniucaoshanDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CniucaoshanDlg::OnBnClickedOk)
	ON_MESSAGE(WM_MY_MESSAGE, OnMyMessage)
	ON_MESSAGE(WM_DELETE_MESSAGE, OnDeleteMessage)
	ON_MESSAGE(WM_CLOSEZD_MESSAGE, OnCloseZdMessage)
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CniucaoshanDlg 消息处理程序

BOOL CniucaoshanDlg::OnInitDialog()
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

	// TODO: 在此添加额外的初始化代码
	cfont.CreateFont(108, // nHeight 
		0, // nWidth 
		0, // nEscapement 
		0, // nOrientation 
		FW_NORMAL, // nWeight 
		FALSE, // bItalic 
		FALSE, // bUnderline 
		0, // cStrikeOut 
		ANSI_CHARSET, // nCharSet 
		OUT_DEFAULT_PRECIS, // nOutPrecision 
		CLIP_DEFAULT_PRECIS, // nClipPrecision 
		DEFAULT_QUALITY, // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("Microsoft Sans Serif")); // lpszFac);
	
	IDC_CARNUM.SetFont(&cfont);

	cfont1.CreateFont(72, // nHeight 
		0, // nWidth 
		0, // nEscapement 
		0, // nOrientation 
		FW_NORMAL, // nWeight 
		FALSE, // bItalic 
		FALSE, // bUnderline 
		0, // cStrikeOut 
		ANSI_CHARSET, // nCharSet 
		OUT_DEFAULT_PRECIS, // nOutPrecision 
		CLIP_DEFAULT_PRECIS, // nClipPrecision 
		DEFAULT_QUALITY, // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("Microsoft Sans Serif")); // lpszFac);

	IDC.SetFont(&cfont1);
	

	cfont2.CreateFont(28, // nHeight 
		0, // nWidth 
		0, // nEscapement 
		0, // nOrientation 
		FW_NORMAL, // nWeight 
		FALSE, // bItalic 
		FALSE, // bUnderline 
		0, // cStrikeOut 
		ANSI_CHARSET, // nCharSet 
		OUT_DEFAULT_PRECIS, // nOutPrecision 
		CLIP_DEFAULT_PRECIS, // nClipPrecision 
		DEFAULT_QUALITY, // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("Microsoft Sans Serif")); // lpszFac);

	IDC_REG.SetFont(&cfont2);

	cfont3.CreateFont(28, // nHeight 
		0, // nWidth 
		0, // nEscapement 
		0, // nOrientation 
		FW_NORMAL, // nWeight 
		FALSE, // bItalic 
		FALSE, // bUnderline 
		0, // cStrikeOut 
		ANSI_CHARSET, // nCharSet 
		OUT_DEFAULT_PRECIS, // nOutPrecision 
		CLIP_DEFAULT_PRECIS, // nClipPrecision 
		DEFAULT_QUALITY, // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("Microsoft Sans Serif")); // lpszFac);
	m_ListCrl.SetFont(&cfont3);



	cfont4.CreateFont(20, // nHeight 
		0, // nWidth 
		0, // nEscapement 
		0, // nOrientation 
		FW_NORMAL, // nWeight 
		FALSE, // bItalic 
		FALSE, // bUnderline 
		0, // cStrikeOut 
		ANSI_CHARSET, // nCharSet 
		OUT_DEFAULT_PRECIS, // nOutPrecision 
		CLIP_DEFAULT_PRECIS, // nClipPrecision 
		DEFAULT_QUALITY, // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("Microsoft Sans Serif")); // lpszFac);
	IDC_BOX9.SetFont(&cfont4);
	IDC_BOX10.SetFont(&cfont4);


	m_ListCrl.InsertColumn(0, L"姓名", LVCFMT_LEFT, 200);
	m_ListCrl.InsertColumn(1, L"身份证号", LVCFMT_LEFT, 300);
	m_ListCrl.InsertColumn(2, L"手机号", LVCFMT_LEFT, 260);
	m_ListCrl.InsertColumn(3, L"安康码", LVCFMT_LEFT, 200);
	
	Init();//初始化sdk
	Connect();//设置连接事件与重连时间	
	
	if (!Login(sDVRIP, wDVRPort, sUserName, sPassword)) {	//注册设备
		//exit(0);
	}

	Htime(); //获取海康威视设备时间



	//char szLan[128] = { 0 };
	//NET_DVR_BARRIERGATE_CFG m_struGateCFG;
	//m_struGateCFG.dwSize = sizeof(NET_DVR_BARRIERGATE_CFG);
	//m_struGateCFG.dwChannel = 1;
	//m_struGateCFG.byLaneNo = 1;
	//m_struGateCFG.byBarrierGateCtrl = 1;
	//if (!NET_DVR_RemoteControl(IUserID, NET_DVR_BARRIERGATE_CTRL, &m_struGateCFG, sizeof(NET_DVR_BARRIERGATE_CFG)))
	//{
	//	printf("设置修改失败, Error code: %d", NET_DVR_GetLastError());
	//}
	//else {
	//	CWnd* wnd = AfxGetApp()->GetMainWnd();
	//	::SendMessage(*wnd, WM_CLOSEZD_MESSAGE, 0, 0);
	//}



	// 下面4行临时注释
	 SetupAlarm();//布防
	 SetMessageCallBack();	//注册报警回调函数 
	 DWORD tid;
	 CreateThread(NULL, 0, ThreadProc1, 0, CREATE_SUSPENDED, &tid);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CniucaoshanDlg::OnSysCommand(UINT nID, LPARAM lParam)
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


void CniucaoshanDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)//定时器1达到条件
	{
		printf("定时器");

		// 关闭道闸
		char szLan[128] = { 0 };
		NET_DVR_BARRIERGATE_CFG m_struGateCFG;
		m_struGateCFG.dwSize = sizeof(NET_DVR_BARRIERGATE_CFG);
		m_struGateCFG.dwChannel = 1;
		m_struGateCFG.byLaneNo = 1;
		m_struGateCFG.byBarrierGateCtrl = 0;
		if (!NET_DVR_RemoteControl(IUserID, NET_DVR_BARRIERGATE_CTRL, &m_struGateCFG, sizeof(NET_DVR_BARRIERGATE_CFG)))
		{
			char csError[1024]={0};
			printf("设置修改失败, Error code: %d", NET_DVR_GetLastError());

		} else{
			KillTimer(timer);//当定时器1产生后立即关闭
		}

	}

	CDialogEx::OnTimer(nIDEvent);
}


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CniucaoshanDlg::OnPaint()
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
HCURSOR CniucaoshanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CniucaoshanDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


void CniucaoshanDlg::OnClose() {
	//报警撤防
	CloseAlarm();
	//释放相机
	NET_DVR_Logout(IUserID);//注销用户
	NET_DVR_Cleanup();//释放SDK资源	
	CDialog::OnClose();
}