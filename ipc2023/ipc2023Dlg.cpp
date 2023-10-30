﻿
// ipc2023Dlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "ipc2023.h"
#include "ipc2023Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// Cipc2023Dlg 대화 상자



Cipc2023Dlg::Cipc2023Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IPC2023_DIALOG, pParent)
	, CBaseLayer("ChatDlg") //
	, m_bSendReady(FALSE)
	, m_nAckReady(-1)

	, m_unSrcAddr(_T(""))
	, m_unDstAddr(_T(""))
	, m_stMessage(_T(""))
	, m_adapterName(_T(""))
{
	//대화상자 멤버 변수 초기화
	//  m_unDstAddr = 0;
	//  unSrcAddr = 0;
	//  m_stMessage = _T("");
	//대화 상자 멤버 초기화 완료

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//Protocol Layer Setting
	m_LayerMgr.AddLayer(new CChatAppLayer("ChatApp"));
	m_LayerMgr.AddLayer(new CEthernetLayer("Ethernet"));
	m_LayerMgr.AddLayer(new FileTransLayer("FileTrans"));
	m_LayerMgr.AddLayer(new CNILayer("Network"));
	m_LayerMgr.AddLayer(this);

	// 레이어를 연결한다. (레이어 생성)
	m_LayerMgr.ConnectLayers("Network ( *Ethernet ( *ChatApp ( *ChatDlg ) *FileTrans ( *ChatDlg ) ) )");

	m_ChatApp = (CChatAppLayer*)m_LayerMgr.GetLayer("ChatApp");
	m_Network = (CNILayer*)m_LayerMgr.GetLayer("Network");
	m_Ethernet = (CEthernetLayer*)m_LayerMgr.GetLayer("Ethernet");
	m_File = (FileTransLayer*)m_LayerMgr.GetLayer("FileTrans");

	//Protocol Layer Setting
}

void Cipc2023Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SRC, m_unSrcAddr);
	DDX_Text(pDX, IDC_EDIT_DST, m_unDstAddr);
	DDX_Text(pDX, IDC_EDIT_MSG, m_stMessage);
	DDX_Control(pDX, IDC_LIST_CHAT, m_ListChat);
	DDX_Control(pDX, IDC_COMBO_ADAPTER_LIST, m_adapterList);
	DDX_CBString(pDX, IDC_COMBO_ADAPTER_LIST, m_adapterName);
	DDX_Control(pDX, IDC_EDIT_SRC, m_editSrc);
	DDX_Control(pDX, IDC_PROGRESS_FILE_TRANSFER, m_progressFile);
}

// 레지스트리에 등록하기 위한 변수
UINT nRegSendMsg;
UINT nRegAckMsg;
// 레지스트리에 등록하기 위한 변수


BEGIN_MESSAGE_MAP(Cipc2023Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ADDR, &Cipc2023Dlg::OnBnClickedButtonAddr)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &Cipc2023Dlg::OnBnClickedButtonSend)
	ON_WM_TIMER()
	// Ack 레지스터 등록

	ON_REGISTERED_MESSAGE(nRegSendMsg, OnRegSendMsg)
	ON_REGISTERED_MESSAGE(nRegAckMsg, OnRegAckMsg)


	ON_BN_CLICKED(IDC_CHECK_TOALL, &Cipc2023Dlg::OnBnClickedCheckToall)
	ON_CBN_SELCHANGE(IDC_COMBO_ADAPTER_LIST, &Cipc2023Dlg::OnCbnSelchangeComboAdapterList)
	ON_BN_CLICKED(IDC_BUTTON_SEL_FILE, &Cipc2023Dlg::OnBnClickedButtonSelFile)
	ON_BN_CLICKED(IDC_BUTTON_SEND_FILE, &Cipc2023Dlg::OnBnClickedButtonSendFile)
	ON_BN_CLICKED(IDC_CHECK2, &Cipc2023Dlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK1, &Cipc2023Dlg::OnBnClickedCheck1)
END_MESSAGE_MAP()


// Cipc2023Dlg 메시지 처리기

BOOL Cipc2023Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	SetRegstryMessage();
	SetDlgState(IPC_INITIALIZING);
	SetAdapterList();
	m_File->SetProgressBar(&m_progressFile);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void Cipc2023Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void Cipc2023Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR Cipc2023Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void Cipc2023Dlg::OnBnClickedButtonSend()
{
	UpdateData(TRUE);

	if (!m_stMessage.IsEmpty())
	{
		//SetTimer(1, 2000, NULL);
		//m_nAckReady = 0;

		SendData();
		m_stMessage = "";

		(CEdit*)GetDlgItem(IDC_EDIT3)->SetFocus();

		//////////////////////// fill the blank ///////////////////////////////
				// Send 신호를 브로드캐스트로 알림
		//::SendMessage(HWND_BROADCAST,nRegSendMsg, 0, 0);
		///////////////////////////////////////////////////////////////////////
	}

	UpdateData(FALSE);
}

void Cipc2023Dlg::SetRegstryMessage()
{
	nRegSendMsg = RegisterWindowMessage(_T("Send IPC Message"));
	nRegAckMsg = RegisterWindowMessage(_T("Ack IPC Message"));

}

void Cipc2023Dlg::SendData()
{
	CString MsgHeader;

	MsgHeader.Format(_T("[%s -> %s] "), m_unSrcAddr, m_unDstAddr);

	m_ListChat.AddString(MsgHeader + m_stMessage);

	int nlength = m_stMessage.GetLength();
	unsigned char* ppayload = new unsigned char[nlength + 1];
	memcpy(ppayload, (unsigned char*)(LPCTSTR)m_stMessage, nlength);
	ppayload[nlength] = '\0';

	// 보낼 data와 메시지 길이를 Send함수로 넘겨준다.
	m_ChatApp->Send(ppayload, nlength);
}

BOOL Cipc2023Dlg::Receive(unsigned char* ppayload)
{
	if (m_nAckReady == -1)
	{
	}

	CString msg;
	msg.Format(_T("[%s -> %s] "), m_unSrcAddr, m_unDstAddr);

	m_ListChat.AddString(msg + (LPCTSTR)ppayload);
	return TRUE;
}

BOOL Cipc2023Dlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		switch (pMsg->wParam)
		{
		case VK_RETURN:
			if (::GetDlgCtrlID(::GetFocus()) == IDC_EDIT3)
				OnBnClickedButtonSend();
			return FALSE;
		case VK_ESCAPE: return FALSE;
		}
		break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void Cipc2023Dlg::SetAdapterList()
{
	CStringArray aList;
	m_Network->GetMacAddressList(aList);
	for (int i = 0; i < aList.GetSize(); i++)
		m_adapterList.AddString(aList[i]);
}

void Cipc2023Dlg::SetDlgState(int state)
{
	UpdateData(TRUE);

	CButton* pChkButton = (CButton*)GetDlgItem(IDC_CHECK1);

	CButton* pSendButton = (CButton*)GetDlgItem(bt_send);
	CButton* pSetAddrButton = (CButton*)GetDlgItem(bt_setting);
	CEdit* pMsgEdit = (CEdit*)GetDlgItem(IDC_EDIT3);
	CEdit* pSrcEdit = (CEdit*)GetDlgItem(IDC_EDIT1);
	CEdit* pDstEdit = (CEdit*)GetDlgItem(IDC_EDIT2);

	switch (state)
	{
	case IPC_INITIALIZING:
		pSendButton->EnableWindow(FALSE);
		pMsgEdit->EnableWindow(FALSE);
		m_ListChat.EnableWindow(FALSE);
		break;
	case IPC_READYTOSEND:
		pSendButton->EnableWindow(TRUE);
		pMsgEdit->EnableWindow(TRUE);
		m_ListChat.EnableWindow(TRUE);
		break;
	case IPC_WAITFORACK:	break;
	case IPC_ERROR:		break;
	case IPC_UNICASTMODE:
		memset(m_ucDstAddr, 0x00, 6);
		pDstEdit->EnableWindow(TRUE);
		break;
	case IPC_BROADCASTMODE:
		memset(m_ucDstAddr, 0xff, 6);
		pDstEdit->EnableWindow(FALSE);
		break;
	case IPC_ADDR_SET:
		pSetAddrButton->SetWindowText(_T("재설정(&R)"));
		//pSrcEdit->EnableWindow(FALSE);
		pDstEdit->EnableWindow(FALSE);
		//pChkButton->EnableWindow(FALSE);
		break;
	case IPC_ADDR_RESET:
		pSetAddrButton->SetWindowText(_T("설정(&O)"));
		pDstEdit->EnableWindow(TRUE);
		//if (!pChkButton->GetCheck())
		//	pDstEdit->EnableWindow(TRUE);
		//pChkButton->EnableWindow(TRUE);
		break;
	}

	UpdateData(FALSE);
}


void Cipc2023Dlg::EndofProcess()
{
	m_LayerMgr.DeAllocLayer();
}

// Send메시지 레지스트리가 켜졌을 때
LRESULT Cipc2023Dlg::OnRegSendMsg(WPARAM wParam, LPARAM lParam)
{

	if (m_nAckReady) {
		// File 레이어에서 상대방이 전송한 메시지가 담긴 파일을 가져옴
		if (m_LayerMgr.GetLayer("File")->Receive())
		{
			// 메시지를 받았다면 Ack 신호를 브로드캐스트로 날린다.
			::SendMessage(HWND_BROADCAST, nRegAckMsg, 0, 0);
		}
	}
	return 0;
}

LRESULT Cipc2023Dlg::OnRegAckMsg(WPARAM wParam, LPARAM lParam)
{
	if (!m_nAckReady) { // Ack 신호를 받으면 타이머를 멈춘다.
		m_nAckReady = -1;
		KillTimer(1);
	}

	return 0;
}

void Cipc2023Dlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	m_ListChat.AddString(_T(">> The last message was time-out.."));
	m_nAckReady = -1;
	KillTimer(1);

	CDialog::OnTimer(nIDEvent);
}

void Cipc2023Dlg::Stouc(CString& src, UCHAR* dst)
{
	sscanf_s(src, "%02x:%02x:%02x:%02x:%02x:%02x",
		&dst[0], &dst[1], &dst[2],
		&dst[3], &dst[4], &dst[5]);
}

void Cipc2023Dlg::UctoS(UCHAR* src, CString& dst)
{
	dst.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"),
		src[0], src[1], src[2],
		src[3], src[4], src[5]);
}

void Cipc2023Dlg::OnBnClickedButtonAddr()
{
	UpdateData(TRUE);



	if (m_unDstAddr == "" ||
		m_unSrcAddr == "" ||
		m_unDstAddr.GetLength() < 16)
	{
		AfxMessageBox(_T("주소를 설정 오류발생",
			"경고"),
			MB_OK | MB_ICONSTOP);

		return;
	}
	m_Network->flip();
	if (m_bSendReady) {
		SetDlgState(IPC_ADDR_RESET);
		SetDlgState(IPC_INITIALIZING);
	}
	else {
		Stouc(m_unDstAddr, m_ucDstAddr);
		m_Ethernet->SetSourceAddress(m_ucSrcAddr);
		m_Ethernet->SetDestinAddress(m_ucDstAddr);

		AfxBeginThread(m_Network->ThreadFunction_RECEIVE, m_Network);
		SetDlgState(IPC_ADDR_SET);
		SetDlgState(IPC_READYTOSEND);
	}

	m_bSendReady = !m_bSendReady;
}



void Cipc2023Dlg::OnBnClickedCheckToall()
{
	CButton* pChkButton = (CButton*)GetDlgItem(IDC_CHECK_TOALL);

	if (pChkButton->GetCheck()) {
		SetDlgState(IPC_BROADCASTMODE);
	}
	else {
		SetDlgState(IPC_UNICASTMODE);
	}
}

void Cipc2023Dlg::OnCbnSelchangeComboAdapterList()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	memcpy(m_ucSrcAddr, m_Network->SetAdapter(m_adapterList.GetCurSel()), 6);
	UctoS(m_ucSrcAddr, m_unSrcAddr);
	m_editSrc.SetWindowTextA(m_unSrcAddr);
}


void Cipc2023Dlg::OnBnClickedButtonSelFile()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str = _T("All files(*.*)|*.*|"); // 모든 파일 표시

	CFileDialog dlg(TRUE, _T("*.dat"), NULL,
		OFN_HIDEREADONLY |
		OFN_OVERWRITEPROMPT
		, _T("All Files (*.*)|*.*|Text Files (*.txt)|*.txt|JPEG Files (*.jpg)|*.jpg|"), this);	//파일 확장자 필터링하는 코드

	if (dlg.DoModal() == IDOK)
	{
		strPathName = dlg.GetPathName();

		CString strFileExt = dlg.GetFileExt();

		if (((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck() && strFileExt.CompareNoCase(_T("jpg")) == 0) {
			m_File->SetFilePath(strPathName);
			AfxBeginThread(m_File->FILE_SEND, m_File);
		}

		else if (((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck() && strFileExt.CompareNoCase(_T("txt")) == 0) {
			m_File->SetFilePath(strPathName);
			AfxBeginThread(m_File->FILE_SEND, m_File);
		}
		else {
			AfxMessageBox(_T("파일 형식이랑 버튼 선택이 일치하지 않아요."), MB_OK | MB_ICONWARNING);
		}

		/*
		// 파일 경로를 가져와 사용할 경우, Edit Control에 값 저장
		SetDlgItemText(IDC_EDIT_FILE_PATH, strPathName);
		m_File->SetFilePath(strPathName);
		*/
	}

}


void Cipc2023Dlg::OnBnClickedButtonSendFile()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//CFile file, output;
	//unsigned char buffer[1488];
	//CString a;
	//file.Open(strPathName, CFile::modeRead);
	//output.Open(_T(file.GetFileName()), CFile::modeCreate | CFile::modeWrite);
	//CArchive ar(&file, CArchive::load);
	//a.Format(_T("[%s] : %d"), file.GetFileName(), file.GetLength());
	//AfxMessageBox(a);
	//m_progressFile.SetRange(0, file.GetLength()/1488);
	//int i = 0;
	//while (ar.Read(buffer, 1488)) {
	//	m_progressFile.SetPos(i);
	//	i++;
	//	output.Write(buffer, 1488);
	//}
	//ar.Close();
	//file.Close();
	//output.Close();
	AfxBeginThread(m_File->FILE_SEND, m_File);
}

void Cipc2023Dlg::OnBnClickedCheck1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck()) {
		((CButton*)GetDlgItem(IDC_CHECK2))->SetCheck(0);
	}
}


void Cipc2023Dlg::OnBnClickedCheck2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck()) {
		((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(0);
	}
}


