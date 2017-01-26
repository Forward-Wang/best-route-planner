// ������������·���滮ģ��ϵͳDlg.cpp : implementation file
//

/***************************************************************************************************************************************/
/*ͷ�ļ�*/
/***************************************************************************************************************************************/
#include "stdafx.h"
#include "������������·���滮ģ��ϵͳ.h"
#include "������������·���滮ģ��ϵͳDlg.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <mmsystem.h>
#include "HelpDlg.h"
#include "CarDlg.h"
#include "LawDlg.h"
#include "QueryDlg.h"

#include <winsock.h>
#pragma  comment(lib,"winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/***************************************************************************************************************************************/
/*�궨��*/
/***************************************************************************************************************************************/
#define UNKNOWN    0x0000 //δ֪

#define OUTDOOR    0x1000 //δѡ��
#define ONBLUE     0x2000 //ѡ������
#define ONROAD     0x3000 //ѡ�е�·

#define STOREFLAG  0x4000 //��ǲֿ�
#define CLIENTFLAG 0x5000 //��ǿͻ�
#define HIDEFLAG   0x6000 //��ʱ���ر�ǵĿͻ��������ظ�����

#define SMOOTH     0x7000 //��·����
#define CROWD      0x8000 //��·ӵ��
/***************************************************************************************************************************************/
/*�ඨ��*/
/***************************************************************************************************************************************/
//�����
typedef struct
{
	int  x; //��������
	int  y; //���������
	int  aroundid[9]; //����������ID�����8�����+������־0��
	int  roadid[6]; //���������·ID�����5��·+������־0��
	int  id; //���ID
	int  choose; //�û���ѡ��STOREFLAG/CLIENTFLAG��
	int  father; //�����ID������goback()����·�ߣ�
	BOOL running; //������ڵ�·�Ƿ������н�
	BOOL been; //�Ƿ������͵�
}NODE;

//��·��
typedef struct
{
	int  id; //��·ID
	int  traffic; //��··��
	BOOL running; //�Ƿ������н���
	BOOL then; //·���Ƿ�̬���
}ROAD;

//ASTAR�����ࣨ����A���㷨��
typedef struct astar
{
	int     id; //���ID
	int     center; //���Ľ��ID
	double  g; //�Ӳֿ������Ľ����չ�����ӽ���·�̸�g(n)
	double  price; //��·��f(n)=g(n)+h(n) ������h(n)������������
	struct  astar *next;
}ASTAR;

//ROUTE�����ࣨ���ڴ�����ID��
typedef struct route
{
	int     id; //���ID
	struct  route *next;
}ROUTE;
/***************************************************************************************************************************************/
/*ȫ�ֱ���*/
/***************************************************************************************************************************************/
//����·����
int* finalArray;
ROUTE  *Head=NULL; //ͷ���
ROUTE  *Start; //������
ROUTE  *Link,*link,*lin; //������

CPoint Point1,Point2; //������ꡢ����ͼ��λ��
CDC    memDC1,*pDC1;CBitmap bmp1; CRect rt1; //��ͼ��
BOOL   timer=FALSE,ing=FALSE,end=FALSE,crowd=TRUE,adjust=FALSE,test=FALSE,oncrowd=TRUE; //��ʱ���Ƿ������Ƿ����н��С��Ƿ񵽴��յ㡢����Ƿ�ӵ�����Ƿ�̬�滮���Ƿ�������
BOOL   fok=TRUE,fstop=TRUE,freplay=TRUE; //���ε����ť
int    lastnode; //����ʱ����һ���������
int    touch=OUTDOOR,only=0,plan=-1,speed=15,mode; //ѡ��״̬���ֿ�Ψһ���������н��ٶ�
double xi,yi,sonx,sony; //��ǰ������ꡢ��һ���������
double D,O,P,M,V,S,T; //�����(km)��ÿ�������(����/��)�������(kw)��������(t)���ٶ�(m/s)���ɱ�(Ԫ)��ʱ��(h)
/***************************************************************************************************************************************/
/*��������*/
/***************************************************************************************************************************************/
int  * Array(); //�ɱ�ע��Ϣ��̬��������
int  * Product(int *order,int n); //������˳�����������˳��
ROUTE* FindRoute(int *store,int *client); //������*store��*clientΪ��ʼ������·������
ROUTE* FindCircle(int *order); //��order�������ŵ�˳�����Ѳ��·������
ROUTE* Cool(int *order,int num); //�˻��㷨������Ѳ��·������
ROUTE* Recool(ROUTE *old,int times); //����Ƿ���ļ��еľ�·�߻�Ҫ��
BOOL   CheckBlank(CString str); //���������Ϣ�Ƿ�Ϊ��
BOOL   CheckNum(CString str); //��������Ƿ�Ƿ�
BOOL   CheckSurvive(int one); //����Ƿ���·����
BOOL   CheckTraffic(int one,int two); //���������㹲�е�·�Ƿ�ӵ��
BOOL   CheckNode(int first,int second); //���first�����Χ�Ƿ����second���
BOOL   CheckCool(ROUTE *left); //����Ƿ���Ҫ��ʣ�����������˻�
BOOL   CheckClient(int first,int second); //����first���second��֮���Ƿ����δ���͵��Ŀͻ����
BOOL   CheckOncrowd(int r); //���ͻ�������ڵ�·�Ƿ�ӵ��
void   ChangeCircle(); //��̬�滮��·��
void   Goback(int son); //��sonΪ���տͻ���ʼ����·�� 
void   FreeAstar(ASTAR *h); //�ͷ�ASTAR����
void   FreeRoute(ROUTE *h); //�ͷ�ROUTE����
void   AddRunning(int first,int second); //��first��second���ȷ����·�������޸�Ϊ�н���
void   ChooseA(double need);//ȷ������
void   FprintfA(double G); //����д���ļ�
void   CalculateTime(); //����ʱ��
void   CalculateHead();//����Head������·��
double CalculateDis(int first,int second); //���������ľ���
double CalculateSpeed(int first,int second); //�����ٶ�
/***************************************************************************************************************************************/
/*¼�����н����Ϣ*/
/***************************************************************************************************************************************/
//����75����·
ROAD road[76];

//����71�����
NODE node[72]=		
//��ͨ�����Ϣ¼��
{{0,0},{222,271,{2,5,51,52,0},{1,2,5,0}},{222,333,{1,3,7,52,53,0},{2,3,7,0}},{222,416,{2,4,8,53,54,0},{3,4,8,0}},{222,494,{3,9,54,0},{4,9,0}},{308,271,{1,6,0},{5,6,0}},
{340,280,{5,7,10,55,0},{6,10,11,14,0}},{340,333,{2,6,8,11,0},{7,11,12,15,0}},{340,414,{3,7,9,13,56,0},{8,12,13,16,0}},{340,494,{4,8,14,56,0},{9,13,17,0}},{384,280,{6,11,16,0},{14,18,21,0}},
{384,333,{7,10,12,0},{15,18,22,0}},{426,332,{11,13,17,0},{19,22,23,0}},{426,413,{8,12,14,18,0},{16,19,20,24,0}},{426,493,{9,13,19,0},{17,20,25,0}},{475,218,{16,22,0},{26,30,0}},
{475,277,{10,15,17,24,0},{21,26,27,31,0}},{475,330,{12,16,18,24,25,57,0},{23,27,28,33,34,0}},{475,413,{13,17,19,26,57,62,0},{24,28,29,35,0}},{475,492,{14,18,33,62,0},{25,29,36,0}},
{505,68,{21,27,58,59,0},{37,38,43,0}},{505,123,{20,22,28,59,0},{38,39,44,0}},{505,218,{15,21,23,49,60,0},{30,39,40,45,0}},{505,265,{22,24,25,49,61,0},{32,40,41,75,0}},
{493,285,{16,17,23,0},{31,32,33,0}},{505,330,{17,23,26,31,61,0},{34,41,42,46,0}},{505,413,{18,25,32,0},{35,42,47,0}},{538,68,{20,28,0},{43,48,0}},{538,123,{21,27,29,63,0},{44,48,49,0}},
{538,218,{28,30,49,63,0},{49,50,74,0}},{538,286,{29,31,0},{50,51,0}},{538,330,{25,30,32,36,64,65,0},{46,51,52,55,0}},{538,413,{26,31,33,34,64,65,66,67,0},{47,52,53,56,0}},
{538,491,{19,32,35,66,67,68,0},{36,53,54,58,0}},{584,412,{32,39,0},{56,57,0}},{595,506,{33,42,70,0},{58,59,0}},{612,328,{31,37,0},{55,60,0}},{631,363,{36,38,0},{60,61,0}},
{651,385,{37,39,43,0},{61,62,66,0}},{649,424,{34,38,40,69,0},{57,62,63,67,0}},{648,456,{39,41,0},{63,64,0}},{645,469,{40,42,0},{64,65,0}},{642,508,{35,41,48,70,0},{59,65,68}},
{710,408,{38,44,0},{66,71,0}},{723,432,{43,45,71,0},{71,72,0}},{721,478,{44,46,71,0},{72,73,0}},{713,530,{45,47,0},{70,73,0}},{701,515,{46,48,0},{69,70,0}},{678,509,{42,47,0},{68,69,0}},{529,218,{22,23,29,60,0},{45,74,75,0}},{0,0},
//�ͻ������Ϣ¼��
{222,231,{1,0},{1,0}},{222,302,{1,2,0},{2,0}},{222,382,{2,3,0},{3,0}},{222,463,{3,4,0},{4,0}},{340,228,{6,0},{10,0}},
{340,465,{8,9,0},{13,0}},{475,378,{17,18,0},{28,0}},{505,40,{20,0},{37,0}},{505,100,{20,21,0},{38,0}},{518,218,{22,49,0},{45,0}},
{505,295,{23,25,0},{41,0}},{475,459,{18,19,0},{29,0}},{538,165,{28,29,0},{49,0}},{538,364,{31,32,65,0},{52,0}},{538,395,{31,32,64,0},{52,0}},
{538,437,{32,33,67,0},{53,0}},{538,475,{32,33,66,0},{53,0}},{538,596,{33,0},{54,0}},{665,427,{39,0},{67,0}},{631,508,{35,42,0},{59,0}},{723,453,{44,45,0},{72,0}}};

//�ͻ��ص������
char* nodename[22]=
{"","�����ʹ�","�ܹ��ʹ�","������������","��ɽ�й���ҽԺ","�ӱ���ɽһ��","��ɽ����ҽ�ǿ�ҽԺ","��԰��������","����ٻ��г�","��ɽѧԺ","����԰","��ɽ�еھ�ҽԺ","ͬ��ҽԺ","��ɽ����","��ɽ������","��ɽ����","���ȹ�԰","�Ϻ���Ƶ�","�Ϻ���԰","��ɽ��·��������","��ɽƽ��ҽԺ","��ɽ�س�ҽԺ"};
/***************************************************************************************************************************************/
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyDlg dialog

CMyDlg::CMyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMyDlg)
	m_DistanceStr = _T("");
	m_MinRadio = -1;
	m_PrimeStr = _T("");
	m_SpeedStr = _T("");
	m_TrafficStr = _T("");
	m_WholeStr = _T("");
	m_FuelEdit = _T("");
	m_NeedEdit = _T("");
	m_OtherEdit = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMyDlg)
	DDX_Text(pDX, IDC_DistanceStr, m_DistanceStr);
	DDX_Radio(pDX, IDC_MindisRadio, m_MinRadio);
	DDX_Text(pDX, IDC_PrimeStr, m_PrimeStr);
	DDX_Text(pDX, IDC_SpeedStr, m_SpeedStr);
	DDX_Text(pDX, IDC_TrafficStr, m_TrafficStr);
	DDX_Text(pDX, IDC_WholeStr, m_WholeStr);
	DDX_Text(pDX, IDC_FuelEdit, m_FuelEdit);
	DDX_Text(pDX, IDC_NeedEdit, m_NeedEdit);
	DDX_Text(pDX, IDC_OtherEdit, m_OtherEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMyDlg, CDialog)
	//{{AFX_MSG_MAP(CMyDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_OkButton, OnOkButton)
	ON_BN_CLICKED(IDC_StopButton, OnStopButton)
	ON_BN_CLICKED(IDC_ReplayButton, OnReplayButton)
	ON_BN_CLICKED(IDC_ResetButton, OnResetButton)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_ClientMenu, OnClientMenu)
	ON_COMMAND(ID_StoreMenu, OnStoreMenu)
	ON_COMMAND(ID_SmoothMenu, OnSmoothMenu)
	ON_COMMAND(ID_CrowdMenu, OnCrowdMenu)
	ON_BN_CLICKED(IDC_TestButton, OnTestButton)
	ON_COMMAND(ID_HelpMenu, OnHelpMenu)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_CarMenu, OnCarMenu)
	ON_COMMAND(ID_LawMenu, OnLawMenu)
	ON_COMMAND(ID_QueryMenu, OnQueryMenu)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyDlg message handlers

BOOL CMyDlg::OnInitDialog() //��ʼ������
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

/*************************************************/
/*��ʼ������*/
/*************************************************/	

	//ɾ����¼�ļ�
	remove("mindistance.txt");
	remove("CarA.txt");
	remove("adjust.txt");

	//ѯ��ѡ�ú���������ʽ
	if(MessageBox("�������Ƿ�ѡ�á���������ģʽ����\n��������ģʽ�����������õ���·��һ������һ�εĶ̣����ױƽ����·�������ʺ�ʵ���ã�\n����������ģʽ������������·���Ǿ���һ������㷨�õ��Ĳ��ȶ�ֵ�����ڲ����㷨��׼ȷ�ԡ����ʺϲ����ã�",
				  "ѯ��",MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
		mode=0;
		GetDlgItem(IDC_ModeStr)->SetWindowText("��������ģʽ");
	}
	else
	{
		mode=1;
		GetDlgItem(IDC_ModeStr)->SetWindowText("����������ģʽ");
	}

	//����ʽ�˵�����
	m_Menu.LoadMenu(IDR_MarkMenu);
	pen1.CreatePen(PS_SOLID,4,RGB(230,0,0)); //�������ʶ���1������ɫ�ߣ�
	pen2.CreatePen(PS_SOLID,10,RGB(0,64,152)); //�������ʶ���2���������㣩
	pen3.CreatePen(PS_SOLID,10,RGB(34,174,230)); //�������ʶ���3����ǳ���㣩

	GetDlgItem(IDC_StopButton)->EnableWindow(FALSE);
	GetDlgItem(IDC_ReplayButton)->EnableWindow(FALSE);

	//��ͼ׼��
	pDC1=GetWindowDC(); 
	memDC1.CreateCompatibleDC(pDC1)  ; 
	GetWindowRect(&rt1);
	bmp1.CreateCompatibleBitmap(pDC1,rt1.Width(),rt1.Height());  
	memDC1.SelectObject(&bmp1); 

	//��ʼ������������Ϣ
	for(int i=1;i<=71;i++)
	{
		node[i].id=i; //�ý��ID
		node[i].choose=UNKNOWN; //δѡ���ַ
		node[i].running=FALSE; //���δռ��
		node[i].father=UNKNOWN; //ָ��δ֪�ĸ����
		node[i].been=FALSE; //δ���͵�
	}	
	//��ʼ����·��������Ϣ
	for(i=1;i<=75;i++)
	{
		road[i].id=i; //�õ�·ID
		road[i].traffic=SMOOTH; //�õ�··��
		road[i].running=FALSE; //��·����
		road[i].then=FALSE; //·���Ƕ�̬���
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}
/***************************************************************************************************************************************/
void CMyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMyDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMyDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/***************************************************************************************************************************************/
/*��Ϣ��Ӧ��������*/
/***************************************************************************************************************************************/
void CMyDlg::OnOkButton() //ȷ����ť
{

/************************************/
/*���������Ϣ*/
/************************************/
	UpdateData(TRUE); //��ȡ����
	//������
	if(CheckBlank(m_NeedEdit) == FALSE) //�Ƿ�Ϊ��
	{
		MessageBox("����δ����������!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(CheckNum(m_NeedEdit) == FALSE) //�����Ƿ�Ƿ�
	{
		MessageBox("��������Ч������!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(atof(m_NeedEdit) > 12) //�����Ƿ񳬹�
	{
		MessageBox("������ز�����12��!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	//ȼ�ͷ�
	if(CheckBlank(m_FuelEdit) == FALSE)
	{
		MessageBox("����δ����ȼ�ͷ�!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(CheckNum(m_FuelEdit) == FALSE)
	{
		MessageBox("��������Чȼ�ͷ�!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(atof(m_FuelEdit) > 10) //�ͷ��Ƿ񳬹�
	{
		MessageBox("ȼ�ͷѲ�����10Ԫ/��!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	//�����
	if(CheckBlank(m_OtherEdit) == FALSE)
	{
		MessageBox("����δ���������!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(CheckNum(m_OtherEdit) == FALSE)
	{
		MessageBox("��������Ч�����!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	//���ͷ���
	if(m_MinRadio == -1)//�ж�ѡ�еĵ�ѡ���ǣ�δѡ��-1,ѡ�е�һ��0,ѡ�еڶ���1��
	{
		MessageBox("����δѡ�����ͷ���!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	plan=m_MinRadio;//�ѷ�������ȫ�ֱ���plan�������������������ж����ĸ�����	
	//����û���û����ͼ�ϱ��
	int havestore=UNKNOWN,haveclient=UNKNOWN;
	for(int n=51;n<=71;n++)
	{
		switch(node[n].choose)
		{
			case STOREFLAG: havestore=STOREFLAG  ;break;
			case CLIENTFLAG:haveclient=CLIENTFLAG;break;
		}
		if(havestore == STOREFLAG && haveclient == CLIENTFLAG)
			break;
	}
/************************************/
/*���ͨ��*/
/************************************/
	if(havestore == STOREFLAG && haveclient == CLIENTFLAG)
	{
		ing=TRUE; //����Ѿ��ڽ�����Ϸ����

		//��ť��ʾ
		if(fok == TRUE)
		{
			MessageBox("ϵͳ�Ѿ�Ϊ���ҵ��˳������·����\n�����������⣬�����Զ�ε�����Ұ�ť�����ҵ����·����","��ʾ",MB_OK|MB_ICONINFORMATION);
			fok=FALSE;
		}
		GetDlgItem(IDC_OkButton)->EnableWindow(FALSE); //�ؼ���ѡת��
		CutScreen();//��ͼ
	
		int *order=Array();

//��ʱorder[]���ŵľ��ǳ���˳��

/************************************/
/*��������·��������������·��*/
/************************************/
		
		//��ʼ����㼰���̽��
		Head=(ROUTE*)malloc(sizeof(ROUTE));
		Start=(ROUTE*)malloc(sizeof(ROUTE));
		Head->id=0;Head->next=Start;
		Start->id=*(order+1);Start->next=NULL;
		Link=Start;

		//�˻��㷨������Ѳ��·��
		for(int num=4;*(order+num)!=0;num++); //�����鳤��
		Head=Cool(order,++num);
		free(order);order=NULL; //�ͷ������ڴ�

//��ʱHeadָ������·���������������δ����Ⱥ���ID��Head->id��Ÿ���·���ܳ�
		
/************************************/
/*����ת����ͼ����ʾ*/
/************************************/	
		
		int  first,second;//��ѯ�н��е�ǰ���ν��
		
		//��ǰ���ν��
		Link=Head->next;
		first=Link->id;node[first].running=TRUE;//ǰ���ռ��
		Link=Link->next;
		second=Link->id;node[second].running=TRUE;//�ν��ռ��
		AddRunning(first,second);//��ǰ�ν��ȷ����·�Ƿ����н���

		//����ǰ���ν�������
		xi=node[first].x;
		yi=node[first].y;
		sonx=node[second].x;
		sony=node[second].y;

/************************************/
/*�ı���Ϣ��������ʾ*/
/************************************/
		
		//����ؼ�״̬
		GetDlgItem(IDC_NeedEdit)->EnableWindow(FALSE);
		GetDlgItem(IDC_FuelEdit)->EnableWindow(FALSE);
		GetDlgItem(IDC_OtherEdit)->EnableWindow(FALSE);
		GetDlgItem(IDC_MindisRadio)->EnableWindow(FALSE);
		GetDlgItem(IDC_MintimRadio)->EnableWindow(FALSE);
		GetDlgItem(IDC_StopButton)->EnableWindow(TRUE);
		GetDlgItem(IDC_ReplayButton)->EnableWindow(TRUE);
		GetDlgItem(IDC_ResetButton)->EnableWindow(TRUE);	
		
		//״̬����ʾ
		ChooseA(atof(m_NeedEdit)); //ȷ��P��O��M������
		V=CalculateSpeed(first,second); //�����ٶ�
		Statistic(); //��������������ʾ

		timer=TRUE;SetTimer(1,speed,NULL); //��Ƕ�ʱ��������


		//��������¼д�����ݿ�
		WriteData();
	}
	else
	{
		MessageBox("����δ��ǲֿ⼰�ͻ���ַ!","����",MB_OK|MB_ICONSTOP);
		return;
	}
}
void CMyDlg::WriteData()
{

}
/***************************************************************************************************************************************/
void CMyDlg::OnStopButton() //��ͣ��ť
{
	CString str;
	GetDlgItem(IDC_StopButton)->GetWindowText(str);
	if(str == "����")
	{
		timer=TRUE;
		SetTimer(1,speed,NULL);
		GetDlgItem(IDC_StopButton)->SetWindowText("��ͣ");
	}
	else
	{
		timer=FALSE;
		KillTimer(1);

		//��ť��ʾ
		if(fstop == TRUE)
		{
			MessageBox("�����ͣ��ť����ֹͣС���н����ڴ��ڼ������Ա�Ƕ³�·�Ρ�\n�������Ϻ󣬿��Ե��������ť�����н���","��ʾ",MB_OK|MB_ICONINFORMATION);
			fstop=FALSE;
		}
		GetDlgItem(IDC_StopButton)->SetWindowText("����");
	}
}
/***************************************************************************************************************************************/
void CMyDlg::OnReplayButton() //���Ұ�ť
{
	//�жϼ�ʱ��
	if(timer == TRUE)
	{ 
		timer=FALSE;
		KillTimer(1);
	}
/************************************/
/*ͼ����*/
/************************************/
	if(freplay == TRUE) //��һ�ε����Ұ�ť
	{
		MessageBox("��ε�����Ұ�ť���԰����������ҵ����·����\n���⣬�����������øð�ť������ʾ�ҵ������·����","��ʾ",MB_OK|MB_ICONINFORMATION);
		freplay=FALSE; //�Ѿ����ǵ�һ�ε����Ұ�ť��
	}
	PutScreen(); //�ػ�
	GetDlgItem(IDC_StopButton)->SetWindowText("��ͣ");
	GetDlgItem(IDC_StopButton)->EnableWindow(TRUE);
/************************************/
/*���ݴ���*/
/************************************/	
	FreeRoute(Head); //�ͷ�����
	speed=15;adjust=FALSE;end=FALSE;oncrowd=TRUE; //������
	for(int i=1;i<=71;i++) //��յ�·�ͽ��ռ��
	{
		if(node[i].choose == HIDEFLAG)
			node[i].choose=CLIENTFLAG;
		node[i].father=UNKNOWN;
		node[i].running=FALSE;
		node[i].been=FALSE;
	}
	for(i=1;i<=75;i++)
	{
		road[i].running=FALSE;
		if(road[i].then == TRUE && road[i].traffic == CROWD)
		{	
			road[i].traffic=SMOOTH;
			road[i].then=FALSE;
		}
	}

	//���¸���������ͷָ��
	int first,second,*order=Array(); //��ʱ�����������û�ѡ��Ŀͻ�
	for(int num=4;*(order+num)!=0;num++); //�����鳤��
	Head=Cool(Product(order,num),++num); //�˻��㷨��Ѳ�����
	free(order);order=NULL; //�ͷ������ڴ�

	Link=Head->next;first=Link->id;
	xi=node[Link->id].x;
	yi=node[Link->id].y;

	Link=Link->next;second=Link->id;
	sonx=node[Link->id].x;
	sony=node[Link->id].y;

	//�ı���ʾ
	AddRunning(first,second);
	V=CalculateSpeed(first,second);
	Statistic();

	timer=TRUE;
	SetTimer(1,speed,NULL);
}
/***************************************************************************************************************************************/
void CMyDlg::OnResetButton() //���ð�ť
{
	//�жϼ�ʱ��
	if(timer == TRUE)
	{ 
		timer=FALSE;
		KillTimer(1);
	}
/************************************/
/*���ݴ���*/
/************************************/
	if(plan != -1 && ing == TRUE)
		FreeRoute(Head); //�������
	speed=15;plan=-1;adjust=FALSE;ing=FALSE;end=FALSE;test=FALSE;only=0;fok=TRUE;oncrowd=TRUE; //������

	//ɾ����¼�ļ�
	remove("mindistance.txt");
	remove("CarA.txt");

	for(int i=1;i<=71;i++) //��ղ�������
	{
		node[i].choose=UNKNOWN;
		node[i].running=FALSE;
		node[i].father=UNKNOWN;
		node[i].been=FALSE;
	}
	for(i=1;i<=75;i++)
	{
		road[i].traffic=SMOOTH;
		road[i].running=FALSE;
		road[i].then=FALSE;
	}
/************************************/
/*ͼ����*/
/************************************/
	MessageBox("��������գ������ڿ������±�ע��ַ��","��ʾ",MB_OK|MB_ICONINFORMATION);

	//״̬�����
	m_NeedEdit="";
	m_FuelEdit="";
	m_OtherEdit="";
	m_MinRadio=-1;
	m_DistanceStr="";
	m_PrimeStr="";
	m_WholeStr="";
	m_SpeedStr="";
	m_TrafficStr="";
	UpdateData(FALSE); //ˢ����Ļ
	//��ͼ�ػ�
	Map();
	//ģʽ����
	if(mode == 0)	GetDlgItem(IDC_ModeStr)->SetWindowText("��������ģʽ");
	else			GetDlgItem(IDC_ModeStr)->SetWindowText("����������ģʽ");
	//����ؼ�״̬
	GetDlgItem(IDC_NeedEdit)->EnableWindow(TRUE);
	GetDlgItem(IDC_FuelEdit)->EnableWindow(TRUE);
	GetDlgItem(IDC_OtherEdit)->EnableWindow(TRUE);
	GetDlgItem(IDC_MindisRadio)->EnableWindow(TRUE);
	GetDlgItem(IDC_MintimRadio)->EnableWindow(TRUE);

	GetDlgItem(IDC_OkButton)->EnableWindow(TRUE);
	GetDlgItem(IDC_StopButton)->SetWindowText("��ͣ");
	GetDlgItem(IDC_StopButton)->EnableWindow(FALSE);
	GetDlgItem(IDC_ReplayButton)->EnableWindow(FALSE);
	GetDlgItem(IDC_TestButton)->SetWindowText("����");
}
/***************************************************************************************************************************************/
void CMyDlg::OnTimer(UINT nIDEvent) //��ʱ����Ӧ����
{
	static int first,second;
	
	//�����ж�֮ǰ�ĺ��������Ƿ��Ѿ�������һ���
	if(xi == node[Link->id].x && yi == node[Link->id].y) //�������½��
	{
		first=Link->id; //�ֻ�ǰ���ν��

		//������ǿͻ�
		if(node[Link->id].choose == CLIENTFLAG)
		{
			//�Ż�����
			PlaySound("Been.wav",NULL,SND_FILENAME|SND_ASYNC); //������Ч
			if(node[Link->id].been == FALSE) PutScreen(); //����֮ǰ�ߵĺ��ߣ�Ԥʾ�Ѿ����͵�һ���ͻ�
			node[Link->id].been=TRUE; //���Ϊ�����ͱ�־
			ShowBeen(); //ɸѡ�����͵Ŀͻ���������ʾ��

			//�������ռ��
			for(int i=1;i<=71;i++)
				node[i].running=FALSE;
			for(i=1;i<=75;i++)
			{
				road[i].running=FALSE;
				TrafficFlag(i); //�ػ�ӵ����·
			}
		}

		//��һ������ȡ��
		if(Link->next != NULL)//δ����β
		{
			Link=Link->next; //ȡ��һ�����
			second=Link->id; //�ν��ı�

			sonx=node[second].x; //ȡ��һ�������
			sony=node[second].y;

			//�Ż�����
			node[second].running=TRUE; //�½��ռ��
			AddRunning(first,second); //��·ռ�����
			ChangeAnytime(first,second); //��ʱ�����ٶȺ�·��
		}
		else //�����յ�
		{
			end=TRUE; //��־����
			timer=FALSE; //��־��ʱ���ر�
			KillTimer(1); //ɾ����ʱ��

			if(test != TRUE)	MessageBox("����ɱ���Ѱ·��","��ʾ",MB_OK|MB_ICONINFORMATION);
			GetDlgItem(IDC_StopButton)->EnableWindow(FALSE); //��ͣ��ť����ѡ
		}
	}

	//���н�·��
	DrawLine();

	CDialog::OnTimer(nIDEvent);
}
/***************************************************************************************************************************************/
void CMyDlg::OnRButtonDown(UINT nFlags, CPoint point) //�Ҽ���Ϣ
{	
	//�����ͼ��
	if((point.x>=180&&point.x<=780)&&(point.y>=11&&point.y<=611))
	{
		//��ʼ���˵���Ч
		CMenu *pMenu=m_Menu.GetSubMenu(0);
		if(only == 0)	pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_ENABLED);
		else       	    pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);
		pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_ENABLED);
		pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);

		//�����ַ�͵�·ʱ������λ�ô���
		Point1.x=point.x;Point1.y=point.y; //���û�������괫��ȫ�����꣨�Ա���������Ӧ�������ã�
		if	   ((point.x>=227&&point.x<=237)&&(point.y>=226&&point.y<=236))	{Point2.x=232;Point2.y=231;if(node[51].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=227&&point.x<=237)&&(point.y>=297&&point.y<=307))	{Point2.x=232;Point2.y=302;if(node[52].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=227&&point.x<=237)&&(point.y>=377&&point.y<=387)) {Point2.x=232;Point2.y=382;if(node[53].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=233&&point.x<=243)&&(point.y>=458&&point.y<=468)) {Point2.x=238;Point2.y=463;if(node[54].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=315&&point.x<=325)&&(point.y>=223&&point.y<=233)) {Point2.x=320;Point2.y=228;if(node[55].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=317&&point.x<=327)&&(point.y>=460&&point.y<=470)) {Point2.x=320;Point2.y=465;if(node[56].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=450&&point.x<=460)&&(point.y>=373&&point.y<=383)) {Point2.x=455;Point2.y=378;if(node[57].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=510&&point.x<=520)&&(point.y>=35 &&point.y<=45 )) {Point2.x=515;Point2.y=40 ;if(node[58].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=510&&point.x<=520)&&(point.y>=99 &&point.y<=109)) {Point2.x=515;Point2.y=104;if(node[59].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=513&&point.x<=523)&&(point.y>=203&&point.y<=213)) {Point2.x=518;Point2.y=208;if(node[60].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=510&&point.x<=520)&&(point.y>=290&&point.y<=300)) {Point2.x=515;Point2.y=295;if(node[61].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}		
		else if((point.x>=480&&point.x<=490)&&(point.y>=454&&point.y<=464)) {Point2.x=485;Point2.y=459;if(node[62].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=541&&point.x<=551)&&(point.y>=160&&point.y<=170)) {Point2.x=546;Point2.y=165;if(node[63].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=548&&point.x<=558)&&(point.y>=359&&point.y<=369)) {Point2.x=553;Point2.y=364;if(node[64].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=541&&point.x<=551)&&(point.y>=390&&point.y<=400)) {Point2.x=546;Point2.y=395;if(node[65].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=543&&point.x<=553)&&(point.y>=432&&point.y<=442)) {Point2.x=558;Point2.y=447;if(node[66].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=541&&point.x<=551)&&(point.y>=470&&point.y<=480)) {Point2.x=546;Point2.y=475;if(node[67].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=510&&point.x<=520)&&(point.y>=591&&point.y<=601)) {Point2.x=515;Point2.y=596;if(node[68].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=660&&point.x<=670)&&(point.y>=412&&point.y<=422)) {Point2.x=680;Point2.y=432;if(node[69].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=625&&point.x<=635)&&(point.y>=510&&point.y<=520)) {Point2.x=607;Point2.y=538;if(node[70].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}
		else if((point.x>=728&&point.x<=738)&&(point.y>=448&&point.y<=458)) {Point2.x=733;Point2.y=453;if(node[71].choose!=UNKNOWN||ing==TRUE||end==TRUE) {pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);}}	
		else
		{
			//�ֿ⡢�ͻ���ǲ���ѡ��·����ѡ
			pMenu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);
			pMenu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);
			pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_ENABLED);

			//����·���ɲ���ѡ
			if     ((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=231&&Point1.y<=271)) {if(road[1 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=271&&Point1.y<=333)) {if(road[2 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=219&&Point1.x<=224)&&(Point1.y>=333&&Point1.y<=416)) {if(road[3 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=416&&Point1.y<=494)) {if(road[4 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=224&&Point1.x<=306)&&(Point1.y>=268&&Point1.y<=274)) {if(road[5 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=306&&Point1.x<=338)&&(Point1.y>=268&&Point1.y<=283)) {if(road[6 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=222&&Point1.x<=338)&&(Point1.y>=330&&Point1.y<=336)) {if(road[7 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=222&&Point1.x<=338)&&(Point1.y>=413&&Point1.y<=419)) {if(road[8 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=222&&Point1.x<=340)&&(Point1.y>=491&&Point1.y<=497)) {if(road[9 ].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=228&&Point1.y<=280)) {if(road[10].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=277&&Point1.y<=333)) {if(road[11].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=337&&Point1.x<=342)&&(Point1.y>=333&&Point1.y<=414)) {if(road[12].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=414&&Point1.y<=494)) {if(road[13].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=340&&Point1.x<=384)&&(Point1.y>=277&&Point1.y<=282)) {if(road[14].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=340&&Point1.x<=384)&&(Point1.y>=332&&Point1.y<=336)) {if(road[15].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=338&&Point1.x<=426)&&(Point1.y>=413&&Point1.y<=418)) {if(road[16].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=340&&Point1.x<=426)&&(Point1.y>=491&&Point1.y<=497)) {if(road[17].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=381&&Point1.x<=387)&&(Point1.y>=280&&Point1.y<=333)) {if(road[18].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=423&&Point1.x<=429)&&(Point1.y>=332&&Point1.y<=413)) {if(road[19].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=423&&Point1.x<=429)&&(Point1.y>=413&&Point1.y<=493)) {if(road[20].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=384&&Point1.x<=476)&&(Point1.y>=277&&Point1.y<=281)) {if(road[21].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=384&&Point1.x<=426)&&(Point1.y>=330&&Point1.y<=336)) {if(road[22].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=330&&Point1.y<=335)) {if(road[23].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=412&&Point1.y<=416)) {if(road[24].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=490&&Point1.y<=496)) {if(road[25].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=218&&Point1.y<=277)) {if(road[26].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=277&&Point1.y<=330)) {if(road[27].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=330&&Point1.y<=413)) {if(road[28].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=413&&Point1.y<=492)) {if(road[29].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=476&&Point1.x<=506)&&(Point1.y>=215&&Point1.y<=222)) {if(road[30].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=477&&Point1.x<=493)&&(Point1.y>=274&&Point1.y<=288)) {if(road[31].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=493&&Point1.x<=503)&&(Point1.y>=265&&Point1.y<=285)) {if(road[32].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=477&&Point1.x<=493)&&(Point1.y>=285&&Point1.y<=328)) {if(road[33].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
		    else if((Point1.x>=476&&Point1.x<=505)&&(Point1.y>=327&&Point1.y<=333)) {if(road[34].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=476&&Point1.x<=505)&&(Point1.y>=410&&Point1.y<=416)) {if(road[35].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=476&&Point1.x<=538)&&(Point1.y>=490&&Point1.y<=495)) {if(road[36].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=502&&Point1.x<=508)&&(Point1.y>=40 &&Point1.y<=68 )) {if(road[37].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=502&&Point1.x<=508)&&(Point1.y>=68 &&Point1.y<=123)) {if(road[38].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=504&&Point1.x<=508)&&(Point1.y>=123&&Point1.y<=218)) {if(road[39].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=218&&Point1.y<=265)) {if(road[40].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=265&&Point1.y<=330)) {if(road[41].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=330&&Point1.y<=413)) {if(road[42].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=65 &&Point1.y<=71 )) {if(road[43].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=120&&Point1.y<=126)) {if(road[44].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=505&&Point1.x<=529)&&(Point1.y>=215&&Point1.y<=221)) {if(road[45].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=327&&Point1.y<=333)) {if(road[46].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=410&&Point1.y<=416)) {if(road[47].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=68 &&Point1.y<=123)) {if(road[48].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=123&&Point1.y<=218)) {if(road[49].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=218&&Point1.y<=286)) {if(road[50].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=286&&Point1.y<=330)) {if(road[51].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}		
			else if((Point1.x>=535&&Point1.x<=540)&&(Point1.y>=330&&Point1.y<=413)) {if(road[52].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=413&&Point1.y<=491)) {if(road[53].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=491&&Point1.y<=596)) {if(road[54].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=538&&Point1.x<=612)&&(Point1.y>=327&&Point1.y<=331)) {if(road[55].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=538&&Point1.x<=584)&&(Point1.y>=410&&Point1.y<=416)) {if(road[56].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=584&&Point1.x<=648)&&(Point1.y>=412&&Point1.y<=424)) {if(road[57].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=541&&Point1.x<=595)&&(Point1.y>=491&&Point1.y<=506)) {if(road[58].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=595&&Point1.x<=642)&&(Point1.y>=503&&Point1.y<=511)) {if(road[59].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=610&&Point1.x<=631)&&(Point1.y>=330&&Point1.y<=363)) {if(road[60].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=631&&Point1.x<=651)&&(Point1.y>=363&&Point1.y<=386)) {if(road[61].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=647&&Point1.x<=653)&&(Point1.y>=385&&Point1.y<=424)) {if(road[62].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=646&&Point1.x<=651)&&(Point1.y>=424&&Point1.y<=456)) {if(road[63].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=645&&Point1.x<=648)&&(Point1.y>=456&&Point1.y<=469)) {if(road[64].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=640&&Point1.x<=647)&&(Point1.y>=469&&Point1.y<=508)) {if(road[65].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=653&&Point1.x<=710)&&(Point1.y>=385&&Point1.y<=408)) {if(road[66].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=650&&Point1.x<=665)&&(Point1.y>=422&&Point1.y<=427)) {if(road[67].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=642&&Point1.x<=678)&&(Point1.y>=506&&Point1.y<=515)) {if(road[68].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=678&&Point1.x<=701)&&(Point1.y>=509&&Point1.y<=515)) {if(road[69].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=701&&Point1.x<=712)&&(Point1.y>=515&&Point1.y<=529)) {if(road[70].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=710&&Point1.x<=723)&&(Point1.y>=408&&Point1.y<=432)) {if(road[71].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=720&&Point1.x<=725)&&(Point1.y>=433&&Point1.y<=478)) {if(road[72].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=711&&Point1.x<=722)&&(Point1.y>=478&&Point1.y<=530)) {if(road[73].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=529&&Point1.x<=538)&&(Point1.y>=215&&Point1.y<=221)) {if(road[74].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else if((Point1.x>=508&&Point1.x<=535)&&(Point1.y>=223&&Point1.y<=265)) {if(road[75].running==TRUE||end==TRUE||test==TRUE) {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);}}
			else   {pMenu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);} //������
		}
		//�������ʾ����ʽ�˵�
		ClientToScreen(&point);
		CRect rect;
		rect.top=point.x;
		rect.left=point.y;
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,rect.top,rect.left,this,&rect);
	}

	CDialog::OnRButtonDown(nFlags, point);
}
/***************************************************************************************************************************************/
void CMyDlg::OnMouseMove(UINT nFlags, CPoint point) //����ƶ�
{
	//���������ʾ
	if((point.x>=179&&point.x<=779)&&(point.y>=11&&point.y<=611))
	{
		CString str;
		str.Format("%d",point.x);
		GetDlgItem(IDC_x)->SetWindowText(str);
		str.Format("%d",point.y);
		GetDlgItem(IDC_y)->SetWindowText(str);
	}
	else
	{
		CString str="";
		GetDlgItem(IDC_x)->SetWindowText(str);
		GetDlgItem(IDC_y)->SetWindowText(str);
	}
	//��ַѡ��Ч��
	if		((point.x>=227&&point.x<=237)&&(point.y>=226&&point.y<=236)){LightBlue(232,231); touch=ONBLUE;}
	else if((point.x>=227&&point.x<=237)&&(point.y>=297&&point.y<=307))	{LightBlue(232,302); touch=ONBLUE;}
	else if((point.x>=227&&point.x<=237)&&(point.y>=377&&point.y<=387)) {LightBlue(232,382); touch=ONBLUE;}
	else if((point.x>=233&&point.x<=243)&&(point.y>=458&&point.y<=468)) {LightBlue(238,463); touch=ONBLUE;}
	else if((point.x>=315&&point.x<=325)&&(point.y>=223&&point.y<=233)) {LightBlue(320,228); touch=ONBLUE;}
	else if((point.x>=317&&point.x<=327)&&(point.y>=460&&point.y<=470)) {LightBlue(322,465); touch=ONBLUE;}
	else if((point.x>=510&&point.x<=520)&&(point.y>=35 &&point.y<=45 )) {LightBlue(515,40 ); touch=ONBLUE;}
	else if((point.x>=510&&point.x<=520)&&(point.y>=99 &&point.y<=109)) {LightBlue(515,104); touch=ONBLUE;}
	else if((point.x>=541&&point.x<=551)&&(point.y>=160&&point.y<=170)) {LightBlue(546,165); touch=ONBLUE;}
	else if((point.x>=513&&point.x<=523)&&(point.y>=203&&point.y<=213)) {LightBlue(518,208); touch=ONBLUE;}
	else if((point.x>=510&&point.x<=520)&&(point.y>=290&&point.y<=300)) {LightBlue(515,295); touch=ONBLUE;}
	else if((point.x>=450&&point.x<=460)&&(point.y>=373&&point.y<=383)) {LightBlue(455,378); touch=ONBLUE;}
	else if((point.x>=480&&point.x<=490)&&(point.y>=454&&point.y<=464)) {LightBlue(485,459); touch=ONBLUE;}
	else if((point.x>=548&&point.x<=558)&&(point.y>=359&&point.y<=369)) {LightBlue(553,364); touch=ONBLUE;}
	else if((point.x>=541&&point.x<=551)&&(point.y>=390&&point.y<=400)) {LightBlue(546,395); touch=ONBLUE;}
	else if((point.x>=543&&point.x<=553)&&(point.y>=432&&point.y<=442)) {LightBlue(548,437); touch=ONBLUE;}
	else if((point.x>=541&&point.x<=551)&&(point.y>=470&&point.y<=480)) {LightBlue(546,475); touch=ONBLUE;}
	else if((point.x>=510&&point.x<=520)&&(point.y>=591&&point.y<=601)) {LightBlue(515,596); touch=ONBLUE;}
	else if((point.x>=625&&point.x<=635)&&(point.y>=510&&point.y<=520)) {LightBlue(630,515); touch=ONBLUE;}
	else if((point.x>=660&&point.x<=670)&&(point.y>=412&&point.y<=422)) {LightBlue(665,417); touch=ONBLUE;}
	else if((point.x>=728&&point.x<=738)&&(point.y>=448&&point.y<=458)) {LightBlue(733,453); touch=ONBLUE;}
	
	//��·ѡ��Ч��
	else if((point.x>=219&&point.x<=225)&&(point.y>=231&&point.y<=271)&&road[1 ].traffic==SMOOTH&&road[1 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(219,231,225,271); touch=1; }
	else if((point.x>=219&&point.x<=225)&&(point.y>=271&&point.y<=333)&&road[2 ].traffic==SMOOTH&&road[2 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(219,271,225,333); touch=2; }
	else if((point.x>=219&&point.x<=224)&&(point.y>=333&&point.y<=416)&&road[3 ].traffic==SMOOTH&&road[3 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(219,333,224,416); touch=3; }
	else if((point.x>=219&&point.x<=225)&&(point.y>=416&&point.y<=494)&&road[4 ].traffic==SMOOTH&&road[4 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(219,416,225,494); touch=4; }
	else if((point.x>=224&&point.x<=306)&&(point.y>=268&&point.y<=274)&&road[5 ].traffic==SMOOTH&&road[5 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(224,268,306,274); touch=5; }
	else if((point.x>=306&&point.x<=338)&&(point.y>=268&&point.y<=283)&&road[6 ].traffic==SMOOTH&&road[6 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(306,268,338,283); touch=6; }
	else if((point.x>=222&&point.x<=338)&&(point.y>=330&&point.y<=336)&&road[7 ].traffic==SMOOTH&&road[7 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(222,330,338,336); touch=7; }
	else if((point.x>=222&&point.x<=338)&&(point.y>=413&&point.y<=419)&&road[8 ].traffic==SMOOTH&&road[8 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(222,413,338,419); touch=8; }
	else if((point.x>=222&&point.x<=340)&&(point.y>=491&&point.y<=497)&&road[9 ].traffic==SMOOTH&&road[9 ].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(222,491,340,497); touch=9; }
	else if((point.x>=337&&point.x<=343)&&(point.y>=228&&point.y<=280)&&road[10].traffic==SMOOTH&&road[10].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(337,228,343,280); touch=10;}
	else if((point.x>=337&&point.x<=343)&&(point.y>=277&&point.y<=333)&&road[11].traffic==SMOOTH&&road[11].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(337,277,343,333); touch=11;}
	else if((point.x>=337&&point.x<=342)&&(point.y>=333&&point.y<=414)&&road[12].traffic==SMOOTH&&road[12].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(337,333,342,414); touch=12;}
	else if((point.x>=337&&point.x<=343)&&(point.y>=414&&point.y<=494)&&road[13].traffic==SMOOTH&&road[13].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(337,414,343,494); touch=13;}
	else if((point.x>=340&&point.x<=384)&&(point.y>=277&&point.y<=282)&&road[14].traffic==SMOOTH&&road[14].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(340,277,384,282); touch=14;}
	else if((point.x>=340&&point.x<=384)&&(point.y>=332&&point.y<=336)&&road[15].traffic==SMOOTH&&road[15].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(340,332,384,336); touch=15;}
	else if((point.x>=338&&point.x<=426)&&(point.y>=413&&point.y<=418)&&road[16].traffic==SMOOTH&&road[16].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(338,413,426,418); touch=16;}
	else if((point.x>=340&&point.x<=426)&&(point.y>=491&&point.y<=497)&&road[17].traffic==SMOOTH&&road[17].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(340,491,426,497); touch=17;}
	else if((point.x>=381&&point.x<=387)&&(point.y>=280&&point.y<=333)&&road[18].traffic==SMOOTH&&road[18].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(381,280,387,333); touch=18;}
	else if((point.x>=423&&point.x<=429)&&(point.y>=332&&point.y<=413)&&road[19].traffic==SMOOTH&&road[19].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(423,332,429,413); touch=19;}
	else if((point.x>=423&&point.x<=429)&&(point.y>=413&&point.y<=493)&&road[20].traffic==SMOOTH&&road[20].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(423,413,429,493); touch=20;}
	else if((point.x>=384&&point.x<=476)&&(point.y>=277&&point.y<=281)&&road[21].traffic==SMOOTH&&road[21].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(384,277,476,281); touch=21;}
	else if((point.x>=384&&point.x<=426)&&(point.y>=330&&point.y<=336)&&road[22].traffic==SMOOTH&&road[22].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(384,330,426,336); touch=22;}
	else if((point.x>=426&&point.x<=476)&&(point.y>=330&&point.y<=335)&&road[23].traffic==SMOOTH&&road[23].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(426,330,476,335); touch=23;}
	else if((point.x>=426&&point.x<=476)&&(point.y>=412&&point.y<=416)&&road[24].traffic==SMOOTH&&road[24].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(426,412,476,416); touch=24;}
	else if((point.x>=426&&point.x<=476)&&(point.y>=490&&point.y<=496)&&road[25].traffic==SMOOTH&&road[25].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(426,490,476,496); touch=25;}
	else if((point.x>=473&&point.x<=477)&&(point.y>=218&&point.y<=277)&&road[26].traffic==SMOOTH&&road[26].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(473,218,477,277); touch=26;}
	else if((point.x>=473&&point.x<=477)&&(point.y>=277&&point.y<=330)&&road[27].traffic==SMOOTH&&road[27].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(473,277,477,330); touch=27;}
	else if((point.x>=473&&point.x<=477)&&(point.y>=330&&point.y<=413)&&road[28].traffic==SMOOTH&&road[28].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(473,330,477,413); touch=28;}
	else if((point.x>=473&&point.x<=477)&&(point.y>=413&&point.y<=492)&&road[29].traffic==SMOOTH&&road[29].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(473,413,477,492); touch=29;}
	else if((point.x>=476&&point.x<=506)&&(point.y>=215&&point.y<=222)&&road[30].traffic==SMOOTH&&road[30].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(476,215,506,222); touch=30;}
	else if((point.x>=477&&point.x<=493)&&(point.y>=274&&point.y<=288)&&road[31].traffic==SMOOTH&&road[31].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(477,274,493,288); touch=31;}
	else if((point.x>=493&&point.x<=503)&&(point.y>=265&&point.y<=285)&&road[32].traffic==SMOOTH&&road[32].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(493,265,503,285); touch=32;}
    else if((point.x>=477&&point.x<=493)&&(point.y>=285&&point.y<=328)&&road[33].traffic==SMOOTH&&road[33].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(477,285,493,328); touch=33;}
    else if((point.x>=476&&point.x<=505)&&(point.y>=327&&point.y<=333)&&road[34].traffic==SMOOTH&&road[34].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(476,327,505,333); touch=34;}
	else if((point.x>=476&&point.x<=505)&&(point.y>=410&&point.y<=416)&&road[35].traffic==SMOOTH&&road[35].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(476,410,505,416); touch=35;}
	else if((point.x>=476&&point.x<=538)&&(point.y>=490&&point.y<=495)&&road[36].traffic==SMOOTH&&road[36].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(476,490,538,495); touch=36;}
	else if((point.x>=502&&point.x<=508)&&(point.y>=40 &&point.y<=68 )&&road[37].traffic==SMOOTH&&road[37].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(502,40,508,68  ); touch=37;}
	else if((point.x>=502&&point.x<=508)&&(point.y>=68 &&point.y<=123)&&road[38].traffic==SMOOTH&&road[38].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(502,68,508,123 ); touch=38;}
	else if((point.x>=504&&point.x<=508)&&(point.y>=123&&point.y<=218)&&road[39].traffic==SMOOTH&&road[39].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(504,123,508,218); touch=39;}
	else if((point.x>=503&&point.x<=508)&&(point.y>=218&&point.y<=265)&&road[40].traffic==SMOOTH&&road[40].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(503,218,508,265); touch=40;}
	else if((point.x>=503&&point.x<=508)&&(point.y>=265&&point.y<=330)&&road[41].traffic==SMOOTH&&road[41].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(503,265,508,330); touch=41;}
	else if((point.x>=503&&point.x<=508)&&(point.y>=330&&point.y<=413)&&road[42].traffic==SMOOTH&&road[42].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(503,330,508,413); touch=42;}
	else if((point.x>=505&&point.x<=538)&&(point.y>=65 &&point.y<=71 )&&road[43].traffic==SMOOTH&&road[43].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(505,65,538,71  ); touch=43;}
	else if((point.x>=505&&point.x<=538)&&(point.y>=120&&point.y<=126)&&road[44].traffic==SMOOTH&&road[44].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(505,120,538,126); touch=44;}
	else if((point.x>=505&&point.x<=529)&&(point.y>=215&&point.y<=221)&&road[45].traffic==SMOOTH&&road[45].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(505,215,529,221); touch=45;}
	else if((point.x>=505&&point.x<=538)&&(point.y>=327&&point.y<=333)&&road[46].traffic==SMOOTH&&road[46].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(505,327,538,333); touch=46;}
	else if((point.x>=505&&point.x<=538)&&(point.y>=410&&point.y<=416)&&road[47].traffic==SMOOTH&&road[47].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(505,410,538,416); touch=47;}
	else if((point.x>=535&&point.x<=539)&&(point.y>=68 &&point.y<=123)&&road[48].traffic==SMOOTH&&road[48].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(535,68,539,123 ); touch=48;}
	else if((point.x>=535&&point.x<=541)&&(point.y>=123&&point.y<=218)&&road[49].traffic==SMOOTH&&road[49].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(535,123,541,218); touch=49;}
	else if((point.x>=535&&point.x<=539)&&(point.y>=218&&point.y<=286)&&road[50].traffic==SMOOTH&&road[50].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(535,218,539,286); touch=50;}
	else if((point.x>=535&&point.x<=539)&&(point.y>=286&&point.y<=330)&&road[51].traffic==SMOOTH&&road[51].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(535,286,539,330); touch=51;}
	else if((point.x>=535&&point.x<=540)&&(point.y>=330&&point.y<=413)&&road[52].traffic==SMOOTH&&road[52].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(535,330,540,413); touch=52;}
	else if((point.x>=535&&point.x<=541)&&(point.y>=413&&point.y<=491)&&road[53].traffic==SMOOTH&&road[53].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(535,413,541,491); touch=53;}
	else if((point.x>=535&&point.x<=541)&&(point.y>=491&&point.y<=596)&&road[54].traffic==SMOOTH&&road[54].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(535,491,541,596); touch=54;}
	else if((point.x>=538&&point.x<=612)&&(point.y>=327&&point.y<=331)&&road[55].traffic==SMOOTH&&road[55].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(538,327,612,331); touch=55;}
	else if((point.x>=538&&point.x<=584)&&(point.y>=410&&point.y<=416)&&road[56].traffic==SMOOTH&&road[56].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(538,410,584,416); touch=56;}
	else if((point.x>=584&&point.x<=648)&&(point.y>=412&&point.y<=424)&&road[57].traffic==SMOOTH&&road[57].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(584,412,648,424); touch=57;}
	else if((point.x>=541&&point.x<=595)&&(point.y>=491&&point.y<=506)&&road[58].traffic==SMOOTH&&road[58].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(541,491,595,506); touch=58;}
	else if((point.x>=595&&point.x<=642)&&(point.y>=503&&point.y<=511)&&road[59].traffic==SMOOTH&&road[59].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(595,503,642,511); touch=59;}
	else if((point.x>=610&&point.x<=631)&&(point.y>=330&&point.y<=363)&&road[60].traffic==SMOOTH&&road[60].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(610,330,631,363); touch=60;}
	else if((point.x>=631&&point.x<=651)&&(point.y>=363&&point.y<=386)&&road[61].traffic==SMOOTH&&road[61].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(631,363,651,386); touch=61;}
	else if((point.x>=647&&point.x<=653)&&(point.y>=385&&point.y<=424)&&road[62].traffic==SMOOTH&&road[62].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(647,385,653,424); touch=62;}
	else if((point.x>=646&&point.x<=651)&&(point.y>=424&&point.y<=456)&&road[63].traffic==SMOOTH&&road[63].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(646,424,651,456); touch=63;}
	else if((point.x>=645&&point.x<=648)&&(point.y>=456&&point.y<=469)&&road[64].traffic==SMOOTH&&road[64].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(645,456,651,469); touch=64;}
	else if((point.x>=640&&point.x<=647)&&(point.y>=469&&point.y<=508)&&road[65].traffic==SMOOTH&&road[65].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(640,469,647,508); touch=65;}
	else if((point.x>=653&&point.x<=710)&&(point.y>=385&&point.y<=408)&&road[66].traffic==SMOOTH&&road[66].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(653,385,710,408); touch=66;}
	else if((point.x>=650&&point.x<=665)&&(point.y>=422&&point.y<=427)&&road[67].traffic==SMOOTH&&road[67].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(650,422,665,427); touch=67;}
	else if((point.x>=642&&point.x<=678)&&(point.y>=506&&point.y<=515)&&road[68].traffic==SMOOTH&&road[68].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(642,506,678,515); touch=68;}
	else if((point.x>=678&&point.x<=701)&&(point.y>=509&&point.y<=515)&&road[69].traffic==SMOOTH&&road[69].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(678,509,701,515); touch=69;}
	else if((point.x>=701&&point.x<=712)&&(point.y>=515&&point.y<=529)&&road[70].traffic==SMOOTH&&road[70].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(698,515,712,529); touch=70;}
	else if((point.x>=710&&point.x<=723)&&(point.y>=408&&point.y<=432)&&road[71].traffic==SMOOTH&&road[71].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(710,408,723,432); touch=71;}
	else if((point.x>=720&&point.x<=725)&&(point.y>=433&&point.y<=478)&&road[72].traffic==SMOOTH&&road[72].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(720,433,725,478); touch=72;}
	else if((point.x>=711&&point.x<=722)&&(point.y>=480&&point.y<=530)&&road[73].traffic==SMOOTH&&road[73].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(711,478,722,530); touch=73;}
	else if((point.x>=529&&point.x<=538)&&(point.y>=215&&point.y<=221)&&road[74].traffic==SMOOTH&&road[74].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(529,215,538,221); touch=74;}
	else if((point.x>=508&&point.x<=535)&&(point.y>=223&&point.y<=265)&&road[75].traffic==SMOOTH&&road[75].running==FALSE&&touch==OUTDOOR&&test!=TRUE) {OnPicture(508,223,535,265); touch=75;}

	else //����ڿհ״�
	{
		if(touch == ONBLUE) //����ѡ�е�ַ
		{
			AllBlue();
			touch=OUTDOOR;
		}
		if(touch != OUTDOOR&&touch != ONBLUE) //����ѡ�е�·
		{
			switch(touch)
			{
				case 1 :Picture(219,231,225,271);touch=OUTDOOR;break;
				case 2 :Picture(219,271,225,333);touch=OUTDOOR;break;
				case 3 :Picture(219,333,224,416);touch=OUTDOOR;break;
				case 4 :Picture(219,416,225,494);touch=OUTDOOR;break;
				case 5 :Picture(224,268,306,274);touch=OUTDOOR;break;
				case 6 :Picture(306,268,338,283);touch=OUTDOOR;break;
				case 7 :Picture(222,330,338,336);touch=OUTDOOR;break;
				case 8 :Picture(222,413,338,419);touch=OUTDOOR;break;
				case 9 :Picture(222,491,340,497);touch=OUTDOOR;break;
				case 10:Picture(337,228,343,280);touch=OUTDOOR;break;
				case 11:Picture(337,277,343,333);touch=OUTDOOR;break;
				case 12:Picture(337,333,342,414);touch=OUTDOOR;break;
				case 13:Picture(337,414,343,494);touch=OUTDOOR;break;
				case 14:Picture(340,277,384,282);touch=OUTDOOR;break;
				case 15:Picture(340,332,384,336);touch=OUTDOOR;break;
				case 16:Picture(338,413,426,418);touch=OUTDOOR;break;
				case 17:Picture(340,491,426,497);touch=OUTDOOR;break;
				case 18:Picture(381,280,387,333);touch=OUTDOOR;break;
				case 19:Picture(423,332,429,413);touch=OUTDOOR;break;
				case 20:Picture(423,413,429,493);touch=OUTDOOR;break;
				case 21:Picture(384,277,476,281);touch=OUTDOOR;break;
				case 22:Picture(384,330,426,336);touch=OUTDOOR;break;
				case 23:Picture(426,330,476,335);touch=OUTDOOR;break;
				case 24:Picture(426,412,476,416);touch=OUTDOOR;break;
				case 25:Picture(426,490,476,496);touch=OUTDOOR;break;
				case 26:Picture(473,218,477,277);touch=OUTDOOR;break;
				case 27:Picture(473,277,477,330);touch=OUTDOOR;break;
				case 28:Picture(473,330,477,413);touch=OUTDOOR;break;
				case 29:Picture(473,413,477,492);touch=OUTDOOR;break;
				case 30:Picture(476,215,506,222);touch=OUTDOOR;break;
				case 31:Picture(476,274,493,288);touch=OUTDOOR;break;
				case 32:Picture(493,265,503,285);touch=OUTDOOR;break;
				case 33:Picture(477,285,493,328);touch=OUTDOOR;break;
				case 34:Picture(476,327,505,333);touch=OUTDOOR;break;
				case 35:Picture(476,410,505,416);touch=OUTDOOR;break;
				case 36:Picture(476,490,538,495);touch=OUTDOOR;break;
				case 37:Picture(502,40,508,68  );touch=OUTDOOR;break;
				case 38:Picture(502,68,508,123 );touch=OUTDOOR;break;
				case 39:Picture(504,123,508,218);touch=OUTDOOR;break;
				case 40:Picture(503,218,508,265);touch=OUTDOOR;break;
				case 41:Picture(503,265,508,330);touch=OUTDOOR;break;
				case 42:Picture(503,330,508,413);touch=OUTDOOR;break;
				case 43:Picture(505,65,538,71  );touch=OUTDOOR;break;
				case 44:Picture(505,120,538,126);touch=OUTDOOR;break;
				case 45:Picture(505,215,529,221);touch=OUTDOOR;break;
				case 46:Picture(505,327,538,333);touch=OUTDOOR;break;
				case 47:Picture(505,410,538,416);touch=OUTDOOR;break;
				case 48:Picture(535,68,539,123 );touch=OUTDOOR;break;
				case 49:Picture(535,123,541,218);touch=OUTDOOR;break;
				case 50:Picture(535,218,539,286);touch=OUTDOOR;break;
				case 51:Picture(535,286,539,330);touch=OUTDOOR;break;
				case 52:Picture(535,330,540,413);touch=OUTDOOR;break;
				case 53:Picture(535,413,541,491);touch=OUTDOOR;break;
				case 54:Picture(535,491,541,596);touch=OUTDOOR;break;
				case 55:Picture(538,327,612,331);touch=OUTDOOR;break;
				case 56:Picture(538,410,584,416);touch=OUTDOOR;break;
				case 57:Picture(584,412,648,424);touch=OUTDOOR;break;
				case 58:Picture(541,491,595,506);touch=OUTDOOR;break;
				case 59:Picture(595,503,642,511);touch=OUTDOOR;break;
				case 60:Picture(610,330,631,363);touch=OUTDOOR;break;
				case 61:Picture(631,363,651,386);touch=OUTDOOR;break;
				case 62:Picture(647,385,653,424);touch=OUTDOOR;break;
				case 63:Picture(646,424,651,456);touch=OUTDOOR;break;
				case 64:Picture(645,456,651,469);touch=OUTDOOR;break;
				case 65:Picture(640,469,647,508);touch=OUTDOOR;break;
				case 66:Picture(653,385,710,408);touch=OUTDOOR;break;
				case 67:Picture(650,422,665,427);touch=OUTDOOR;break;
				case 68:Picture(642,506,678,515);touch=OUTDOOR;break;
				case 69:Picture(678,509,701,515);touch=OUTDOOR;break;
				case 70:Picture(698,515,712,529);touch=OUTDOOR;break;
				case 71:Picture(710,408,723,432);touch=OUTDOOR;break;
				case 72:Picture(720,433,725,478);touch=OUTDOOR;break;
				case 73:Picture(711,478,722,530);touch=OUTDOOR;break;
				case 74:Picture(529,215,538,221);touch=OUTDOOR;break;
				case 75:Picture(508,223,535,265);touch=OUTDOOR;break;
			}
			touch=OUTDOOR; //��ԭ��ǳ�û��ѡ��״̬
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}
/***************************************************************************************************************************************/
void CMyDlg::OnStoreMenu() //�ֿ��ǲ˵�
{
	if(only == 0) //δ��ǲֿ�
	{
		//��ʾ�ֿ���
		StoreFlag();

		//ʶ��ֿ��ַ�������������
		if	   ((Point1.x>=227&&Point1.x<=237)&&(Point1.y>=226&&Point1.y<=236))	{Beep(1200,500);node[51].choose=STOREFLAG;only=1;}
		else if((Point1.x>=227&&Point1.x<=237)&&(Point1.y>=297&&Point1.y<=307))	{Beep(1200,500);node[52].choose=STOREFLAG;only=1;}
		else if((Point1.x>=227&&Point1.x<=237)&&(Point1.y>=377&&Point1.y<=387)) {Beep(1200,500);node[53].choose=STOREFLAG;only=1;}	
		else if((Point1.x>=233&&Point1.x<=243)&&(Point1.y>=458&&Point1.y<=468)) {Beep(1200,500);node[54].choose=STOREFLAG;only=1;}
		else if((Point1.x>=315&&Point1.x<=325)&&(Point1.y>=223&&Point1.y<=233)) {Beep(1200,500);node[55].choose=STOREFLAG;only=1;}
		else if((Point1.x>=317&&Point1.x<=327)&&(Point1.y>=460&&Point1.y<=470)) {Beep(1200,500);node[56].choose=STOREFLAG;only=1;}
		else if((Point1.x>=450&&Point1.x<=460)&&(Point1.y>=373&&Point1.y<=383)) {Beep(1200,500);node[57].choose=STOREFLAG;only=1;}
		else if((Point1.x>=510&&Point1.x<=520)&&(Point1.y>=35 &&Point1.y<=45 )) {Beep(1200,500);node[58].choose=STOREFLAG;only=1;}
		else if((Point1.x>=510&&Point1.x<=520)&&(Point1.y>=99 &&Point1.y<=109)) {Beep(1200,500);node[59].choose=STOREFLAG;only=1;}
		else if((Point1.x>=513&&Point1.x<=523)&&(Point1.y>=203&&Point1.y<=213)) {Beep(1200,500);node[60].choose=STOREFLAG;only=1;}
		else if((Point1.x>=510&&Point1.x<=520)&&(Point1.y>=290&&Point1.y<=300)) {Beep(1200,500);node[61].choose=STOREFLAG;only=1;}
		else if((Point1.x>=480&&Point1.x<=490)&&(Point1.y>=454&&Point1.y<=464)) {Beep(1200,500);node[62].choose=STOREFLAG;only=1;}
		else if((Point1.x>=541&&Point1.x<=551)&&(Point1.y>=160&&Point1.y<=170)) {Beep(1200,500);node[63].choose=STOREFLAG;only=1;}
		else if((Point1.x>=548&&Point1.x<=558)&&(Point1.y>=359&&Point1.y<=369)) {Beep(1200,500);node[64].choose=STOREFLAG;only=1;}
		else if((Point1.x>=541&&Point1.x<=551)&&(Point1.y>=390&&Point1.y<=400)) {Beep(1200,500);node[65].choose=STOREFLAG;only=1;}
		else if((Point1.x>=543&&Point1.x<=553)&&(Point1.y>=432&&Point1.y<=442)) {Beep(1200,500);node[66].choose=STOREFLAG;only=1;}
		else if((Point1.x>=541&&Point1.x<=551)&&(Point1.y>=470&&Point1.y<=480)) {Beep(1200,500);node[67].choose=STOREFLAG;only=1;}
		else if((Point1.x>=510&&Point1.x<=520)&&(Point1.y>=591&&Point1.y<=601)) {Beep(1200,500);node[68].choose=STOREFLAG;only=1;}
		else if((Point1.x>=660&&Point1.x<=670)&&(Point1.y>=412&&Point1.y<=422)) {Beep(1200,500);node[69].choose=STOREFLAG;only=1;}
		else if((Point1.x>=625&&Point1.x<=635)&&(Point1.y>=510&&Point1.y<=520)) {Beep(1200,500);node[70].choose=STOREFLAG;only=1;}
		else if((Point1.x>=728&&Point1.x<=738)&&(Point1.y>=448&&Point1.y<=458)) {Beep(1200,500);node[71].choose=STOREFLAG;only=1;}
	}
}
/***************************************************************************************************************************************/
void CMyDlg::OnClientMenu() //�ͻ���ǲ˵�
{
	//��ʾ�ͻ���
	ClientFlag();

	//ʶ��ͻ���ַ�������������
	if	   ((Point1.x>=227&&Point1.x<=237)&&(Point1.y>=226&&Point1.y<=236))	{Beep(1200,500);node[51].choose=CLIENTFLAG;}
	else if((Point1.x>=227&&Point1.x<=237)&&(Point1.y>=297&&Point1.y<=307))	{Beep(1200,500);node[52].choose=CLIENTFLAG;}
	else if((Point1.x>=227&&Point1.x<=237)&&(Point1.y>=377&&Point1.y<=387)) {Beep(1200,500);node[53].choose=CLIENTFLAG;}
	else if((Point1.x>=233&&Point1.x<=243)&&(Point1.y>=458&&Point1.y<=468)) {Beep(1200,500);node[54].choose=CLIENTFLAG;}
	else if((Point1.x>=315&&Point1.x<=325)&&(Point1.y>=223&&Point1.y<=233)) {Beep(1200,500);node[55].choose=CLIENTFLAG;}
	else if((Point1.x>=317&&Point1.x<=327)&&(Point1.y>=460&&Point1.y<=470)) {Beep(1200,500);node[56].choose=CLIENTFLAG;}
	else if((Point1.x>=450&&Point1.x<=460)&&(Point1.y>=373&&Point1.y<=383)) {Beep(1200,500);node[57].choose=CLIENTFLAG;}
	else if((Point1.x>=510&&Point1.x<=520)&&(Point1.y>=35 &&Point1.y<=45 )) {Beep(1200,500);node[58].choose=CLIENTFLAG;}
	else if((Point1.x>=510&&Point1.x<=520)&&(Point1.y>=99 &&Point1.y<=109)) {Beep(1200,500);node[59].choose=CLIENTFLAG;}
	else if((Point1.x>=513&&Point1.x<=523)&&(Point1.y>=203&&Point1.y<=213)) {Beep(1200,500);node[60].choose=CLIENTFLAG;}
	else if((Point1.x>=510&&Point1.x<=520)&&(Point1.y>=290&&Point1.y<=300)) {Beep(1200,500);node[61].choose=CLIENTFLAG;}
	else if((Point1.x>=480&&Point1.x<=490)&&(Point1.y>=454&&Point1.y<=464)) {Beep(1200,500);node[62].choose=CLIENTFLAG;}
	else if((Point1.x>=541&&Point1.x<=551)&&(Point1.y>=160&&Point1.y<=170)) {Beep(1200,500);node[63].choose=CLIENTFLAG;}
	else if((Point1.x>=548&&Point1.x<=558)&&(Point1.y>=359&&Point1.y<=369)) {Beep(1200,500);node[64].choose=CLIENTFLAG;}
	else if((Point1.x>=541&&Point1.x<=551)&&(Point1.y>=390&&Point1.y<=400)) {Beep(1200,500);node[65].choose=CLIENTFLAG;}
	else if((Point1.x>=543&&Point1.x<=553)&&(Point1.y>=432&&Point1.y<=442)) {Beep(1200,500);node[66].choose=CLIENTFLAG;}
	else if((Point1.x>=541&&Point1.x<=551)&&(Point1.y>=470&&Point1.y<=480)) {Beep(1200,500);node[67].choose=CLIENTFLAG;}
	else if((Point1.x>=510&&Point1.x<=520)&&(Point1.y>=591&&Point1.y<=601)) {Beep(1200,500);node[68].choose=CLIENTFLAG;}
	else if((Point1.x>=660&&Point1.x<=670)&&(Point1.y>=412&&Point1.y<=422)) {Beep(1200,500);node[69].choose=CLIENTFLAG;}
	else if((Point1.x>=625&&Point1.x<=635)&&(Point1.y>=510&&Point1.y<=520)) {Beep(1200,500);node[70].choose=CLIENTFLAG;}
	else if((Point1.x>=728&&Point1.x<=738)&&(Point1.y>=448&&Point1.y<=458)) {Beep(1200,500);node[71].choose=CLIENTFLAG;}
}
/***************************************************************************************************************************************/
void CMyDlg::OnSmoothMenu() //�����˵���Ӧ
{
	//ʶ������·���������������
	if     ((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=231&&Point1.y<=271)) {Beep(300,500);road[1 ].traffic=SMOOTH;oncrowd=CheckOncrowd(1);if(plan==1) adjust=TRUE;Picture(219,231,225,271);}
	else if((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=271&&Point1.y<=333)) {Beep(300,500);road[2 ].traffic=SMOOTH;oncrowd=CheckOncrowd(2);if(plan==1) adjust=TRUE;Picture(219,271,225,333);}
	else if((Point1.x>=219&&Point1.x<=224)&&(Point1.y>=333&&Point1.y<=416)) {Beep(300,500);road[3 ].traffic=SMOOTH;oncrowd=CheckOncrowd(3);if(plan==1) adjust=TRUE;Picture(219,333,224,416);}
	else if((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=416&&Point1.y<=494)) {Beep(300,500);road[4 ].traffic=SMOOTH;oncrowd=CheckOncrowd(4);if(plan==1) adjust=TRUE;Picture(219,416,225,494);}
	else if((Point1.x>=224&&Point1.x<=306)&&(Point1.y>=268&&Point1.y<=274)) {Beep(300,500);road[5 ].traffic=SMOOTH;oncrowd=CheckOncrowd(5);if(plan==1) adjust=TRUE;Picture(224,268,306,274);}
	else if((Point1.x>=306&&Point1.x<=338)&&(Point1.y>=268&&Point1.y<=283)) {Beep(300,500);road[6 ].traffic=SMOOTH;oncrowd=CheckOncrowd(6);if(plan==1) adjust=TRUE;Picture(306,268,338,283);}
	else if((Point1.x>=222&&Point1.x<=338)&&(Point1.y>=330&&Point1.y<=336)) {Beep(300,500);road[7 ].traffic=SMOOTH;oncrowd=CheckOncrowd(7);if(plan==1) adjust=TRUE;Picture(222,330,338,336);}
	else if((Point1.x>=222&&Point1.x<=338)&&(Point1.y>=413&&Point1.y<=419)) {Beep(300,500);road[8 ].traffic=SMOOTH;oncrowd=CheckOncrowd(8);if(plan==1) adjust=TRUE;Picture(222,413,338,419);}
	else if((Point1.x>=222&&Point1.x<=340)&&(Point1.y>=491&&Point1.y<=497)) {Beep(300,500);road[9 ].traffic=SMOOTH;oncrowd=CheckOncrowd(9);if(plan==1) adjust=TRUE;Picture(222,491,340,497);}
	else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=228&&Point1.y<=280)) {Beep(300,500);road[10].traffic=SMOOTH;oncrowd=CheckOncrowd(10);if(plan==1) adjust=TRUE;Picture(337,228,343,280);}
	else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=277&&Point1.y<=333)) {Beep(300,500);road[11].traffic=SMOOTH;oncrowd=CheckOncrowd(11);if(plan==1) adjust=TRUE;Picture(337,277,343,333);}
	else if((Point1.x>=337&&Point1.x<=342)&&(Point1.y>=333&&Point1.y<=414)) {Beep(300,500);road[12].traffic=SMOOTH;oncrowd=CheckOncrowd(12);if(plan==1) adjust=TRUE;Picture(337,333,342,414);}
	else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=414&&Point1.y<=494)) {Beep(300,500);road[13].traffic=SMOOTH;oncrowd=CheckOncrowd(13);if(plan==1) adjust=TRUE;Picture(337,414,343,494);}
	else if((Point1.x>=340&&Point1.x<=384)&&(Point1.y>=277&&Point1.y<=282)) {Beep(300,500);road[14].traffic=SMOOTH;oncrowd=CheckOncrowd(14);if(plan==1) adjust=TRUE;Picture(340,277,384,282);}
	else if((Point1.x>=340&&Point1.x<=384)&&(Point1.y>=332&&Point1.y<=336)) {Beep(300,500);road[15].traffic=SMOOTH;oncrowd=CheckOncrowd(15);if(plan==1) adjust=TRUE;Picture(340,332,384,336);}
	else if((Point1.x>=338&&Point1.x<=426)&&(Point1.y>=413&&Point1.y<=418)) {Beep(300,500);road[16].traffic=SMOOTH;oncrowd=CheckOncrowd(16);if(plan==1) adjust=TRUE;Picture(338,413,426,418);}
	else if((Point1.x>=340&&Point1.x<=426)&&(Point1.y>=491&&Point1.y<=497)) {Beep(300,500);road[17].traffic=SMOOTH;oncrowd=CheckOncrowd(17);if(plan==1) adjust=TRUE;Picture(340,491,426,497);}
	else if((Point1.x>=381&&Point1.x<=387)&&(Point1.y>=280&&Point1.y<=333)) {Beep(300,500);road[18].traffic=SMOOTH;oncrowd=CheckOncrowd(18);if(plan==1) adjust=TRUE;Picture(381,280,387,333);}
	else if((Point1.x>=423&&Point1.x<=429)&&(Point1.y>=332&&Point1.y<=413)) {Beep(300,500);road[19].traffic=SMOOTH;oncrowd=CheckOncrowd(19);if(plan==1) adjust=TRUE;Picture(423,332,429,413);}
	else if((Point1.x>=423&&Point1.x<=429)&&(Point1.y>=413&&Point1.y<=493)) {Beep(300,500);road[20].traffic=SMOOTH;oncrowd=CheckOncrowd(20);if(plan==1) adjust=TRUE;Picture(423,413,429,493);}
	else if((Point1.x>=384&&Point1.x<=476)&&(Point1.y>=277&&Point1.y<=281)) {Beep(300,500);road[21].traffic=SMOOTH;oncrowd=CheckOncrowd(21);if(plan==1) adjust=TRUE;Picture(384,277,476,281);}
	else if((Point1.x>=384&&Point1.x<=426)&&(Point1.y>=330&&Point1.y<=336)) {Beep(300,500);road[22].traffic=SMOOTH;oncrowd=CheckOncrowd(22);if(plan==1) adjust=TRUE;Picture(384,330,426,336);}
	else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=330&&Point1.y<=335)) {Beep(300,500);road[23].traffic=SMOOTH;oncrowd=CheckOncrowd(23);if(plan==1) adjust=TRUE;Picture(426,330,476,335);}
	else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=412&&Point1.y<=416)) {Beep(300,500);road[24].traffic=SMOOTH;oncrowd=CheckOncrowd(24);if(plan==1) adjust=TRUE;Picture(426,412,476,416);}
	else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=490&&Point1.y<=496)) {Beep(300,500);road[25].traffic=SMOOTH;oncrowd=CheckOncrowd(25);if(plan==1) adjust=TRUE;Picture(426,490,476,496);}
	else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=218&&Point1.y<=277)) {Beep(300,500);road[26].traffic=SMOOTH;oncrowd=CheckOncrowd(26);if(plan==1) adjust=TRUE;Picture(473,218,477,277);}
	else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=277&&Point1.y<=330)) {Beep(300,500);road[27].traffic=SMOOTH;oncrowd=CheckOncrowd(27);if(plan==1) adjust=TRUE;Picture(473,277,477,330);}
	else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=330&&Point1.y<=413)) {Beep(300,500);road[28].traffic=SMOOTH;oncrowd=CheckOncrowd(28);if(plan==1) adjust=TRUE;Picture(473,330,477,413);}
	else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=413&&Point1.y<=492)) {Beep(300,500);road[29].traffic=SMOOTH;oncrowd=CheckOncrowd(29);if(plan==1) adjust=TRUE;Picture(473,413,477,492);}
	else if((Point1.x>=477&&Point1.x<=506)&&(Point1.y>=215&&Point1.y<=222)) {Beep(300,500);road[30].traffic=SMOOTH;oncrowd=CheckOncrowd(30);if(plan==1) adjust=TRUE;Picture(477,215,506,222);}
	else if((Point1.x>=476&&Point1.x<=493)&&(Point1.y>=274&&Point1.y<=288)) {Beep(300,500);road[31].traffic=SMOOTH;oncrowd=CheckOncrowd(31);if(plan==1) adjust=TRUE;Picture(476,274,493,288);}
	else if((Point1.x>=493&&Point1.x<=503)&&(Point1.y>=265&&Point1.y<=285)) {Beep(300,500);road[32].traffic=SMOOTH;oncrowd=CheckOncrowd(32);if(plan==1) adjust=TRUE;Picture(493,265,503,285);}
   	else if((Point1.x>=477&&Point1.x<=493)&&(Point1.y>=285&&Point1.y<=328)) {Beep(300,500);road[33].traffic=SMOOTH;oncrowd=CheckOncrowd(33);if(plan==1) adjust=TRUE;Picture(477,285,493,328);}
    else if((Point1.x>=476&&Point1.x<=505)&&(Point1.y>=327&&Point1.y<=333)) {Beep(300,500);road[34].traffic=SMOOTH;oncrowd=CheckOncrowd(34);if(plan==1) adjust=TRUE;Picture(476,327,505,333);}
	else if((Point1.x>=476&&Point1.x<=505)&&(Point1.y>=410&&Point1.y<=416)) {Beep(300,500);road[35].traffic=SMOOTH;oncrowd=CheckOncrowd(35);if(plan==1) adjust=TRUE;Picture(476,410,505,416);}
	else if((Point1.x>=476&&Point1.x<=538)&&(Point1.y>=490&&Point1.y<=495)) {Beep(300,500);road[36].traffic=SMOOTH;oncrowd=CheckOncrowd(36);if(plan==1) adjust=TRUE;Picture(476,490,538,495);}
	else if((Point1.x>=502&&Point1.x<=508)&&(Point1.y>=40 &&Point1.y<=68 )) {Beep(300,500);road[37].traffic=SMOOTH;oncrowd=CheckOncrowd(37);if(plan==1) adjust=TRUE;Picture(502,40,508,68  );}
	else if((Point1.x>=502&&Point1.x<=508)&&(Point1.y>=68 &&Point1.y<=123)) {Beep(300,500);road[38].traffic=SMOOTH;oncrowd=CheckOncrowd(38);if(plan==1) adjust=TRUE;Picture(502,68,508,123 );}
	else if((Point1.x>=504&&Point1.x<=508)&&(Point1.y>=123&&Point1.y<=218)) {Beep(300,500);road[39].traffic=SMOOTH;oncrowd=CheckOncrowd(39);if(plan==1) adjust=TRUE;Picture(504,123,508,218);}
	else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=218&&Point1.y<=265)) {Beep(300,500);road[40].traffic=SMOOTH;oncrowd=CheckOncrowd(40);if(plan==1) adjust=TRUE;Picture(503,218,508,265);}
	else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=265&&Point1.y<=330)) {Beep(300,500);road[41].traffic=SMOOTH;oncrowd=CheckOncrowd(41);if(plan==1) adjust=TRUE;Picture(503,265,508,330);}
	else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=330&&Point1.y<=413)) {Beep(300,500);road[42].traffic=SMOOTH;oncrowd=CheckOncrowd(42);if(plan==1) adjust=TRUE;Picture(503,330,508,413);}
	else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=65 &&Point1.y<=71 )) {Beep(300,500);road[43].traffic=SMOOTH;oncrowd=CheckOncrowd(43);if(plan==1) adjust=TRUE;Picture(505,65,538,71  );}
	else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=120&&Point1.y<=126)) {Beep(300,500);road[44].traffic=SMOOTH;oncrowd=CheckOncrowd(44);if(plan==1) adjust=TRUE;Picture(505,120,538,126);}
	else if((Point1.x>=505&&Point1.x<=529)&&(Point1.y>=215&&Point1.y<=221)) {Beep(300,500);road[45].traffic=SMOOTH;oncrowd=CheckOncrowd(45);if(plan==1) adjust=TRUE;Picture(505,215,529,221);}
	else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=327&&Point1.y<=333)) {Beep(300,500);road[46].traffic=SMOOTH;oncrowd=CheckOncrowd(46);if(plan==1) adjust=TRUE;Picture(505,327,538,333);}
	else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=410&&Point1.y<=416)) {Beep(300,500);road[47].traffic=SMOOTH;oncrowd=CheckOncrowd(47);if(plan==1) adjust=TRUE;Picture(505,410,538,416);}
	else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=68 &&Point1.y<=123)) {Beep(300,500);road[48].traffic=SMOOTH;oncrowd=CheckOncrowd(48);if(plan==1) adjust=TRUE;Picture(535,68,539,123 );}
	else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=123&&Point1.y<=218)) {Beep(300,500);road[49].traffic=SMOOTH;oncrowd=CheckOncrowd(49);if(plan==1) adjust=TRUE;Picture(535,123,541,218);}
	else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=218&&Point1.y<=286)) {Beep(300,500);road[50].traffic=SMOOTH;oncrowd=CheckOncrowd(50);if(plan==1) adjust=TRUE;Picture(535,218,539,286);}
	else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=286&&Point1.y<=330)) {Beep(300,500);road[51].traffic=SMOOTH;oncrowd=CheckOncrowd(51);if(plan==1) adjust=TRUE;Picture(535,286,539,330);}		
	else if((Point1.x>=535&&Point1.x<=540)&&(Point1.y>=330&&Point1.y<=413)) {Beep(300,500);road[52].traffic=SMOOTH;oncrowd=CheckOncrowd(52);if(plan==1) adjust=TRUE;Picture(535,330,540,413);}
	else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=413&&Point1.y<=491)) {Beep(300,500);road[53].traffic=SMOOTH;oncrowd=CheckOncrowd(53);if(plan==1) adjust=TRUE;Picture(535,413,541,491);}
	else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=491&&Point1.y<=596)) {Beep(300,500);road[54].traffic=SMOOTH;oncrowd=CheckOncrowd(54);if(plan==1) adjust=TRUE;Picture(535,491,541,596);}
	else if((Point1.x>=538&&Point1.x<=612)&&(Point1.y>=327&&Point1.y<=331)) {Beep(300,500);road[55].traffic=SMOOTH;oncrowd=CheckOncrowd(55);if(plan==1) adjust=TRUE;Picture(538,327,612,331);}
	else if((Point1.x>=538&&Point1.x<=584)&&(Point1.y>=410&&Point1.y<=416)) {Beep(300,500);road[56].traffic=SMOOTH;oncrowd=CheckOncrowd(56);if(plan==1) adjust=TRUE;Picture(538,410,584,416);}
	else if((Point1.x>=584&&Point1.x<=648)&&(Point1.y>=412&&Point1.y<=424)) {Beep(300,500);road[57].traffic=SMOOTH;oncrowd=CheckOncrowd(57);if(plan==1) adjust=TRUE;Picture(584,412,648,424);}
	else if((Point1.x>=541&&Point1.x<=595)&&(Point1.y>=491&&Point1.y<=506)) {Beep(300,500);road[58].traffic=SMOOTH;oncrowd=CheckOncrowd(58);if(plan==1) adjust=TRUE;Picture(541,491,595,506);}
	else if((Point1.x>=595&&Point1.x<=642)&&(Point1.y>=503&&Point1.y<=511)) {Beep(300,500);road[59].traffic=SMOOTH;oncrowd=CheckOncrowd(59);if(plan==1) adjust=TRUE;Picture(595,503,642,511);}
	else if((Point1.x>=610&&Point1.x<=631)&&(Point1.y>=330&&Point1.y<=363)) {Beep(300,500);road[60].traffic=SMOOTH;oncrowd=CheckOncrowd(60);if(plan==1) adjust=TRUE;Picture(610,330,631,363);}
	else if((Point1.x>=631&&Point1.x<=651)&&(Point1.y>=363&&Point1.y<=386)) {Beep(300,500);road[61].traffic=SMOOTH;oncrowd=CheckOncrowd(61);if(plan==1) adjust=TRUE;Picture(631,363,651,386);}
	else if((Point1.x>=647&&Point1.x<=653)&&(Point1.y>=385&&Point1.y<=424)) {Beep(300,500);road[62].traffic=SMOOTH;oncrowd=CheckOncrowd(62);if(plan==1) adjust=TRUE;Picture(648,385,651,424);}
	else if((Point1.x>=646&&Point1.x<=651)&&(Point1.y>=424&&Point1.y<=456)) {Beep(300,500);road[63].traffic=SMOOTH;oncrowd=CheckOncrowd(63);if(plan==1) adjust=TRUE;Picture(646,424,651,456);}
	else if((Point1.x>=645&&Point1.x<=648)&&(Point1.y>=456&&Point1.y<=469)) {Beep(300,500);road[64].traffic=SMOOTH;oncrowd=CheckOncrowd(64);if(plan==1) adjust=TRUE;Picture(645,456,651,469);}
	else if((Point1.x>=640&&Point1.x<=647)&&(Point1.y>=469&&Point1.y<=508)) {Beep(300,500);road[65].traffic=SMOOTH;oncrowd=CheckOncrowd(65);if(plan==1) adjust=TRUE;Picture(640,469,647,508);}
	else if((Point1.x>=653&&Point1.x<=710)&&(Point1.y>=385&&Point1.y<=408)) {Beep(300,500);road[66].traffic=SMOOTH;oncrowd=CheckOncrowd(66);if(plan==1) adjust=TRUE;Picture(653,385,710,408);}
	else if((Point1.x>=650&&Point1.x<=665)&&(Point1.y>=422&&Point1.y<=427)) {Beep(300,500);road[67].traffic=SMOOTH;oncrowd=CheckOncrowd(67);if(plan==1) adjust=TRUE;Picture(650,422,665,427);}
	else if((Point1.x>=642&&Point1.x<=678)&&(Point1.y>=506&&Point1.y<=515)) {Beep(300,500);road[68].traffic=SMOOTH;oncrowd=CheckOncrowd(68);if(plan==1) adjust=TRUE;Picture(642,506,678,515);}
	else if((Point1.x>=678&&Point1.x<=701)&&(Point1.y>=509&&Point1.y<=515)) {Beep(300,500);road[69].traffic=SMOOTH;oncrowd=CheckOncrowd(69);if(plan==1) adjust=TRUE;Picture(678,509,701,515);}
	else if((Point1.x>=701&&Point1.x<=712)&&(Point1.y>=515&&Point1.y<=529)) {Beep(300,500);road[70].traffic=SMOOTH;oncrowd=CheckOncrowd(70);if(plan==1) adjust=TRUE;Picture(698,515,713,525);}
	else if((Point1.x>=710&&Point1.x<=723)&&(Point1.y>=408&&Point1.y<=432)) {Beep(300,500);road[71].traffic=SMOOTH;oncrowd=CheckOncrowd(71);if(plan==1) adjust=TRUE;Picture(710,408,723,432);}
	else if((Point1.x>=720&&Point1.x<=725)&&(Point1.y>=433&&Point1.y<=478)) {Beep(300,500);road[72].traffic=SMOOTH;oncrowd=CheckOncrowd(72);if(plan==1) adjust=TRUE;Picture(720,433,725,478);}
	else if((Point1.x>=711&&Point1.x<=722)&&(Point1.y>=478&&Point1.y<=530)) {Beep(300,500);road[73].traffic=SMOOTH;oncrowd=CheckOncrowd(73);if(plan==1) adjust=TRUE;Picture(711,478,722,530);}
	else if((Point1.x>=529&&Point1.x<=538)&&(Point1.y>=215&&Point1.y<=221)) {Beep(300,500);road[74].traffic=SMOOTH;oncrowd=CheckOncrowd(74);if(plan==1) adjust=TRUE;Picture(529,215,538,221);}
	else if((Point1.x>=508&&Point1.x<=535)&&(Point1.y>=223&&Point1.y<=265)) {Beep(300,500);road[75].traffic=SMOOTH;oncrowd=CheckOncrowd(75);if(plan==1) adjust=TRUE;Picture(508,223,535,265);}

	if(m_MinRadio==1 && ing==TRUE) //���ʱ�䷽��
	{
		ChangeCircle(); //��̬�滮��Head
	
		//�ı���ʾ
		CalculateHead();
		Statistic();
	}
}
/***************************************************************************************************************************************/
void CMyDlg::OnCrowdMenu() //ӵ���˵���Ӧ
{

	//ʶ��ӵ��·���������������
	if     ((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=231&&Point1.y<=271)) {Beep(300,500);road[1 ].traffic=CROWD;oncrowd=CheckOncrowd(1);if(ing==TRUE) road[1 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=271&&Point1.y<=333)) {Beep(300,500);road[2 ].traffic=CROWD;oncrowd=CheckOncrowd(2);if(ing==TRUE) road[2 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=219&&Point1.x<=224)&&(Point1.y>=333&&Point1.y<=416)) {Beep(300,500);road[3 ].traffic=CROWD;oncrowd=CheckOncrowd(3);if(ing==TRUE) road[3 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=219&&Point1.x<=225)&&(Point1.y>=416&&Point1.y<=494)) {Beep(300,500);road[4 ].traffic=CROWD;oncrowd=CheckOncrowd(4);if(ing==TRUE) road[4 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=224&&Point1.x<=306)&&(Point1.y>=268&&Point1.y<=274)) {Beep(300,500);road[5 ].traffic=CROWD;oncrowd=CheckOncrowd(5);if(ing==TRUE) road[5 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=308&&Point1.x<=338)&&(Point1.y>=268&&Point1.y<=283)) {Beep(300,500);road[6 ].traffic=CROWD;oncrowd=CheckOncrowd(6);if(ing==TRUE) road[6 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=222&&Point1.x<=338)&&(Point1.y>=330&&Point1.y<=336)) {Beep(300,500);road[7 ].traffic=CROWD;oncrowd=CheckOncrowd(7);if(ing==TRUE) road[7 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=222&&Point1.x<=338)&&(Point1.y>=413&&Point1.y<=419)) {Beep(300,500);road[8 ].traffic=CROWD;oncrowd=CheckOncrowd(8);if(ing==TRUE) road[8 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=222&&Point1.x<=340)&&(Point1.y>=491&&Point1.y<=497)) {Beep(300,500);road[9 ].traffic=CROWD;oncrowd=CheckOncrowd(9);if(ing==TRUE) road[9 ].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=228&&Point1.y<=280)) {Beep(300,500);road[10].traffic=CROWD;oncrowd=CheckOncrowd(10);if(ing==TRUE) road[10].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=277&&Point1.y<=333)) {Beep(300,500);road[11].traffic=CROWD;oncrowd=CheckOncrowd(11);if(ing==TRUE) road[11].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=337&&Point1.x<=342)&&(Point1.y>=333&&Point1.y<=414)) {Beep(300,500);road[12].traffic=CROWD;oncrowd=CheckOncrowd(12);if(ing==TRUE) road[12].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=337&&Point1.x<=343)&&(Point1.y>=414&&Point1.y<=494)) {Beep(300,500);road[13].traffic=CROWD;oncrowd=CheckOncrowd(13);if(ing==TRUE) road[13].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=340&&Point1.x<=384)&&(Point1.y>=277&&Point1.y<=282)) {Beep(300,500);road[14].traffic=CROWD;oncrowd=CheckOncrowd(14);if(ing==TRUE) road[14].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=340&&Point1.x<=384)&&(Point1.y>=332&&Point1.y<=336)) {Beep(300,500);road[15].traffic=CROWD;oncrowd=CheckOncrowd(15);if(ing==TRUE) road[15].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=338&&Point1.x<=426)&&(Point1.y>=413&&Point1.y<=418)) {Beep(300,500);road[16].traffic=CROWD;oncrowd=CheckOncrowd(16);if(ing==TRUE) road[16].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=340&&Point1.x<=426)&&(Point1.y>=491&&Point1.y<=497)) {Beep(300,500);road[17].traffic=CROWD;oncrowd=CheckOncrowd(17);if(ing==TRUE) road[17].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=381&&Point1.x<=387)&&(Point1.y>=280&&Point1.y<=333)) {Beep(300,500);road[18].traffic=CROWD;oncrowd=CheckOncrowd(18);if(ing==TRUE) road[18].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=423&&Point1.x<=429)&&(Point1.y>=332&&Point1.y<=413)) {Beep(300,500);road[19].traffic=CROWD;oncrowd=CheckOncrowd(19);if(ing==TRUE) road[19].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=423&&Point1.x<=429)&&(Point1.y>=413&&Point1.y<=493)) {Beep(300,500);road[20].traffic=CROWD;oncrowd=CheckOncrowd(20);if(ing==TRUE) road[20].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=384&&Point1.x<=476)&&(Point1.y>=277&&Point1.y<=281)) {Beep(300,500);road[21].traffic=CROWD;oncrowd=CheckOncrowd(21);if(ing==TRUE) road[21].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=384&&Point1.x<=426)&&(Point1.y>=330&&Point1.y<=336)) {Beep(300,500);road[22].traffic=CROWD;oncrowd=CheckOncrowd(22);if(ing==TRUE) road[22].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=330&&Point1.y<=335)) {Beep(300,500);road[23].traffic=CROWD;oncrowd=CheckOncrowd(23);if(ing==TRUE) road[23].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=412&&Point1.y<=416)) {Beep(300,500);road[24].traffic=CROWD;oncrowd=CheckOncrowd(24);if(ing==TRUE) road[24].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=426&&Point1.x<=476)&&(Point1.y>=490&&Point1.y<=496)) {Beep(300,500);road[25].traffic=CROWD;oncrowd=CheckOncrowd(25);if(ing==TRUE) road[25].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=218&&Point1.y<=277)) {Beep(300,500);road[26].traffic=CROWD;oncrowd=CheckOncrowd(26);if(ing==TRUE) road[26].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=277&&Point1.y<=330)) {Beep(300,500);road[27].traffic=CROWD;oncrowd=CheckOncrowd(27);if(ing==TRUE) road[27].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=330&&Point1.y<=413)) {Beep(300,500);road[28].traffic=CROWD;oncrowd=CheckOncrowd(28);if(ing==TRUE) road[28].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=473&&Point1.x<=477)&&(Point1.y>=413&&Point1.y<=492)) {Beep(300,500);road[29].traffic=CROWD;oncrowd=CheckOncrowd(29);if(ing==TRUE) road[29].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=477&&Point1.x<=506)&&(Point1.y>=215&&Point1.y<=222)) {Beep(300,500);road[30].traffic=CROWD;oncrowd=CheckOncrowd(30);if(ing==TRUE) road[30].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=476&&Point1.x<=493)&&(Point1.y>=274&&Point1.y<=288)) {Beep(300,500);road[31].traffic=CROWD;oncrowd=CheckOncrowd(31);if(ing==TRUE) road[31].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=493&&Point1.x<=503)&&(Point1.y>=265&&Point1.y<=285)) {Beep(300,500);road[32].traffic=CROWD;oncrowd=CheckOncrowd(32);if(ing==TRUE) road[32].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
   	else if((Point1.x>=477&&Point1.x<=493)&&(Point1.y>=285&&Point1.y<=328)) {Beep(300,500);road[33].traffic=CROWD;oncrowd=CheckOncrowd(33);if(ing==TRUE) road[33].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
    else if((Point1.x>=476&&Point1.x<=505)&&(Point1.y>=327&&Point1.y<=333)) {Beep(300,500);road[34].traffic=CROWD;oncrowd=CheckOncrowd(34);if(ing==TRUE) road[34].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=476&&Point1.x<=505)&&(Point1.y>=410&&Point1.y<=416)) {Beep(300,500);road[35].traffic=CROWD;oncrowd=CheckOncrowd(35);if(ing==TRUE) road[35].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=476&&Point1.x<=538)&&(Point1.y>=490&&Point1.y<=495)) {Beep(300,500);road[36].traffic=CROWD;oncrowd=CheckOncrowd(36);if(ing==TRUE) road[36].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=502&&Point1.x<=508)&&(Point1.y>=40 &&Point1.y<=68 )) {Beep(300,500);road[37].traffic=CROWD;oncrowd=CheckOncrowd(37);if(ing==TRUE) road[37].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=502&&Point1.x<=508)&&(Point1.y>=68 &&Point1.y<=123)) {Beep(300,500);road[38].traffic=CROWD;oncrowd=CheckOncrowd(38);if(ing==TRUE) road[38].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=504&&Point1.x<=508)&&(Point1.y>=123&&Point1.y<=218)) {Beep(300,500);road[39].traffic=CROWD;oncrowd=CheckOncrowd(39);if(ing==TRUE) road[39].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=218&&Point1.y<=265)) {Beep(300,500);road[40].traffic=CROWD;oncrowd=CheckOncrowd(40);if(ing==TRUE) road[40].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=265&&Point1.y<=330)) {Beep(300,500);road[41].traffic=CROWD;oncrowd=CheckOncrowd(41);if(ing==TRUE) road[41].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=503&&Point1.x<=508)&&(Point1.y>=330&&Point1.y<=413)) {Beep(300,500);road[42].traffic=CROWD;oncrowd=CheckOncrowd(42);if(ing==TRUE) road[42].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=65 &&Point1.y<=71 )) {Beep(300,500);road[43].traffic=CROWD;oncrowd=CheckOncrowd(43);if(ing==TRUE) road[43].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=120&&Point1.y<=126)) {Beep(300,500);road[44].traffic=CROWD;oncrowd=CheckOncrowd(44);if(ing==TRUE) road[44].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=505&&Point1.x<=529)&&(Point1.y>=215&&Point1.y<=221)) {Beep(300,500);road[45].traffic=CROWD;oncrowd=CheckOncrowd(45);if(ing==TRUE) road[45].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=327&&Point1.y<=333)) {Beep(300,500);road[46].traffic=CROWD;oncrowd=CheckOncrowd(46);if(ing==TRUE) road[46].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=505&&Point1.x<=538)&&(Point1.y>=410&&Point1.y<=416)) {Beep(300,500);road[47].traffic=CROWD;oncrowd=CheckOncrowd(47);if(ing==TRUE) road[47].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=68 &&Point1.y<=123)) {Beep(300,500);road[48].traffic=CROWD;oncrowd=CheckOncrowd(48);if(ing==TRUE) road[48].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=123&&Point1.y<=218)) {Beep(300,500);road[49].traffic=CROWD;oncrowd=CheckOncrowd(49);if(ing==TRUE) road[49].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=218&&Point1.y<=286)) {Beep(300,500);road[50].traffic=CROWD;oncrowd=CheckOncrowd(50);if(ing==TRUE) road[50].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=535&&Point1.x<=539)&&(Point1.y>=286&&Point1.y<=330)) {Beep(300,500);road[51].traffic=CROWD;oncrowd=CheckOncrowd(51);if(ing==TRUE) road[51].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=535&&Point1.x<=540)&&(Point1.y>=330&&Point1.y<=413)) {Beep(300,500);road[52].traffic=CROWD;oncrowd=CheckOncrowd(52);if(ing==TRUE) road[52].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=413&&Point1.y<=491)) {Beep(300,500);road[53].traffic=CROWD;oncrowd=CheckOncrowd(53);if(ing==TRUE) road[53].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=535&&Point1.x<=541)&&(Point1.y>=491&&Point1.y<=596)) {Beep(300,500);road[54].traffic=CROWD;oncrowd=CheckOncrowd(54);if(ing==TRUE) road[54].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=538&&Point1.x<=612)&&(Point1.y>=327&&Point1.y<=331)) {Beep(300,500);road[55].traffic=CROWD;oncrowd=CheckOncrowd(55);if(ing==TRUE) road[55].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=538&&Point1.x<=584)&&(Point1.y>=410&&Point1.y<=416)) {Beep(300,500);road[56].traffic=CROWD;oncrowd=CheckOncrowd(56);if(ing==TRUE) road[56].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=584&&Point1.x<=648)&&(Point1.y>=412&&Point1.y<=424)) {Beep(300,500);road[57].traffic=CROWD;oncrowd=CheckOncrowd(57);if(ing==TRUE) road[57].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=541&&Point1.x<=595)&&(Point1.y>=491&&Point1.y<=506)) {Beep(300,500);road[58].traffic=CROWD;oncrowd=CheckOncrowd(58);if(ing==TRUE) road[58].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=595&&Point1.x<=642)&&(Point1.y>=503&&Point1.y<=511)) {Beep(300,500);road[59].traffic=CROWD;oncrowd=CheckOncrowd(59);if(ing==TRUE) road[59].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=610&&Point1.x<=631)&&(Point1.y>=330&&Point1.y<=363)) {Beep(300,500);road[60].traffic=CROWD;oncrowd=CheckOncrowd(60);if(ing==TRUE) road[60].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=631&&Point1.x<=651)&&(Point1.y>=363&&Point1.y<=386)) {Beep(300,500);road[61].traffic=CROWD;oncrowd=CheckOncrowd(61);if(ing==TRUE) road[61].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=647&&Point1.x<=653)&&(Point1.y>=385&&Point1.y<=424)) {Beep(300,500);road[62].traffic=CROWD;oncrowd=CheckOncrowd(62);if(ing==TRUE) road[62].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=646&&Point1.x<=651)&&(Point1.y>=424&&Point1.y<=456)) {Beep(300,500);road[63].traffic=CROWD;oncrowd=CheckOncrowd(63);if(ing==TRUE) road[63].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=645&&Point1.x<=648)&&(Point1.y>=456&&Point1.y<=469)) {Beep(300,500);road[64].traffic=CROWD;oncrowd=CheckOncrowd(64);if(ing==TRUE) road[64].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=640&&Point1.x<=647)&&(Point1.y>=469&&Point1.y<=508)) {Beep(300,500);road[65].traffic=CROWD;oncrowd=CheckOncrowd(65);if(ing==TRUE) road[65].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=653&&Point1.x<=710)&&(Point1.y>=385&&Point1.y<=408)) {Beep(300,500);road[66].traffic=CROWD;oncrowd=CheckOncrowd(66);if(ing==TRUE) road[66].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=650&&Point1.x<=665)&&(Point1.y>=422&&Point1.y<=427)) {Beep(300,500);road[67].traffic=CROWD;oncrowd=CheckOncrowd(67);if(ing==TRUE) road[67].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=642&&Point1.x<=678)&&(Point1.y>=506&&Point1.y<=515)) {Beep(300,500);road[68].traffic=CROWD;oncrowd=CheckOncrowd(68);if(ing==TRUE) road[68].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=678&&Point1.x<=701)&&(Point1.y>=509&&Point1.y<=515)) {Beep(300,500);road[69].traffic=CROWD;oncrowd=CheckOncrowd(69);if(ing==TRUE) road[69].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=701&&Point1.x<=712)&&(Point1.y>=515&&Point1.y<=529)) {Beep(300,500);road[70].traffic=CROWD;oncrowd=CheckOncrowd(70);if(ing==TRUE) road[70].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=710&&Point1.x<=723)&&(Point1.y>=408&&Point1.y<=432)) {Beep(300,500);road[71].traffic=CROWD;oncrowd=CheckOncrowd(71);if(ing==TRUE) road[71].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=720&&Point1.x<=725)&&(Point1.y>=433&&Point1.y<=478)) {Beep(300,500);road[72].traffic=CROWD;oncrowd=CheckOncrowd(72);if(ing==TRUE) road[72].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=711&&Point1.x<=722)&&(Point1.y>=478&&Point1.y<=530)) {Beep(300,500);road[73].traffic=CROWD;oncrowd=CheckOncrowd(73);if(ing==TRUE) road[73].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=529&&Point1.x<=538)&&(Point1.y>=215&&Point1.y<=221)) {Beep(300,500);road[74].traffic=CROWD;oncrowd=CheckOncrowd(74);if(ing==TRUE) road[74].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}
	else if((Point1.x>=508&&Point1.x<=535)&&(Point1.y>=223&&Point1.y<=265)) {Beep(300,500);road[75].traffic=CROWD;oncrowd=CheckOncrowd(75);if(ing==TRUE) road[75].then=TRUE;if(plan==1) adjust=TRUE;touch=OUTDOOR;}

	if(m_MinRadio == 1 && ing == TRUE) //���ʱ�䷽��
	{
		ChangeCircle(); //��̬�滮��Head
	
		//�ı���ʾ
		CalculateHead();
		Statistic();			
		
		return;	
	}
}
/***************************************************************************************************************************************/
void CMyDlg::OnTestButton() //���԰�ť��Ӧ
{
	if(timer == TRUE) //��;������ɾ����ʱ��
	{
		timer=FALSE;
		KillTimer(1);
	}
	
	//��������Ƿ�Ϸ�
	UpdateData(TRUE);
	//������
	if(CheckBlank(m_NeedEdit) == FALSE)
	{
		MessageBox("����δ����������!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(CheckNum(m_NeedEdit) == FALSE)
	{
		MessageBox("��������Ч������!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(atof(m_NeedEdit) > 12)
	{
		MessageBox("������ز�����12��!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	//ȼ�ͷ�
	if(CheckBlank(m_FuelEdit) == FALSE)
	{
		MessageBox("����δ����ȼ�ͷ�!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(CheckNum(m_FuelEdit) == FALSE)
	{
		MessageBox("��������Чȼ�ͷ�!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(atof(m_FuelEdit) > 10)
	{
		MessageBox("ȼ�ͷѲ�����10Ԫ/��!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	//�����
	if(CheckBlank(m_OtherEdit) == FALSE)
	{
		MessageBox("����δ���������!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	if(CheckNum(m_OtherEdit) == FALSE)
	{
		MessageBox("��������Ч�����!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	//����û���û����ͼ�ϱ��
	int havestore=UNKNOWN,haveclient=UNKNOWN;
	for(int n=51;n<=71;n++)
	{
		switch(node[n].choose)
		{
			case STOREFLAG: havestore=STOREFLAG  ;break;
			case CLIENTFLAG:haveclient=CLIENTFLAG;break;
		}
		if(havestore == STOREFLAG && haveclient == CLIENTFLAG)
			break;
	}

	if(havestore == STOREFLAG && haveclient == CLIENTFLAG)
	{	
		test=TRUE;
		//�ͷ��ϴ�ͷ���ָ����ڴ�
		if(plan != -1)
			FreeRoute(Head);
		//ͼ����
		ChooseA(atof(m_NeedEdit));
		m_DistanceStr="";
		m_PrimeStr="";
		m_WholeStr="";
		m_SpeedStr="";
		m_TrafficStr="";
		UpdateData(FALSE); //ˢ����Ļ		
		CString str;
		GetDlgItem(IDC_TestButton)->GetWindowText(str);
		if(str == "����")
		{
			MessageBox("�����Ӳֿ����ڵ�·��ʼ��˫���´�Ҫ���ĵ�·��","��ʾ",MB_OK|MB_ICONINFORMATION);
			if(ing == FALSE) //һ�����͵���԰�ť�����
			{
				CutScreen();
			}
		}
		PutScreen();

		//����ؼ�״̬
		GetDlgItem(IDC_OkButton)->EnableWindow(FALSE);
		GetDlgItem(IDC_TestButton)->SetWindowText("�ز�");
		GetDlgItem(IDC_NeedEdit)->EnableWindow(FALSE);
		GetDlgItem(IDC_FuelEdit)->EnableWindow(FALSE);
		GetDlgItem(IDC_OtherEdit)->EnableWindow(FALSE);
		GetDlgItem(IDC_MindisRadio)->EnableWindow(FALSE);
		GetDlgItem(IDC_MintimRadio)->EnableWindow(FALSE);
		GetDlgItem(IDC_StopButton)->EnableWindow(FALSE);
		GetDlgItem(IDC_ReplayButton)->EnableWindow(FALSE);
		GetDlgItem(IDC_ResetButton)->EnableWindow(TRUE);

		//��Ǵ���
		ing=TRUE;

		for(int i=1;i<=71;i++) 
		{
			node[i].running=FALSE;
			node[i].been=FALSE;
		}

		for(int store=51;store<=71;store++) //�Ҳֿ��ID
		{
			if(node[store].choose==STOREFLAG)
			{
				lastnode=store; //��ʼ���ϴνڵ�
				break;
			}
		}
		
		//����ʼ�ֿ⴦�ĺ��
		CDC *pDC=GetDC();
		pDC->SelectObject(&pen1); //������ѡ���豸������		
		pDC->MoveTo((int)node[store].x,(int)node[store].y);
		pDC->LineTo((int)node[store].x,(int)node[store].y);
		pDC->DeleteDC();

		//ȡ���ֿ����ڵ�������·�������ֿ����Head�׽ڵ�
		Head=(ROUTE*)malloc(sizeof(ROUTE));
		Start=(ROUTE*)malloc(sizeof(ROUTE));
		Head->id=0;Head->next=Start;
		Start->id=store;Start->next=NULL;
		Link=Start;

		//��ʼ�����
		xi=node[Link->id].x;
		yi=node[Link->id].y;

/**************��ʱHead->next->id�����store��ID��֮������񽻸�˫����Ӧ���������***************/
	}
	else
	{
		MessageBox("����δ��ǲֿ⼰�ͻ���ַ!","����",MB_OK|MB_ICONSTOP);
		return;
	}	
	
}
/***************************************************************************************************************************************/
void CMyDlg::OnHelpMenu() //�����˵�
{
	//����ʹ��˵���Ի���
	CHelpDlg dlg;
	dlg.DoModal();
}
/***************************************************************************************************************************************/
void CMyDlg::OnLButtonDblClk(UINT nFlags, CPoint point) //���˫��
{
	if(test == TRUE) //���԰�ť���Ž������´���
	{
		if     ((point.x>=219&&point.x<=225)&&(point.y>=231&&point.y<=271)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(1 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=219&&point.x<=225)&&(point.y>=271&&point.y<=333)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(2 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=219&&point.x<=225)&&(point.y>=333&&point.y<=416)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(3 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=219&&point.x<=225)&&(point.y>=416&&point.y<=494)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(4 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=224&&point.x<=306)&&(point.y>=268&&point.y<=274)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(5 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=306&&point.x<=338)&&(point.y>=268&&point.y<=283)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(6 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=222&&point.x<=338)&&(point.y>=330&&point.y<=336)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(7 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=222&&point.x<=338)&&(point.y>=413&&point.y<=419)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(8 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=222&&point.x<=340)&&(point.y>=491&&point.y<=497)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(9 )==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=337&&point.x<=343)&&(point.y>=228&&point.y<=280)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(10)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=337&&point.x<=343)&&(point.y>=277&&point.y<=333)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(11)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=337&&point.x<=342)&&(point.y>=333&&point.y<=414)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(12)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=337&&point.x<=343)&&(point.y>=414&&point.y<=494)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(13)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=340&&point.x<=384)&&(point.y>=277&&point.y<=283)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(14)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=340&&point.x<=384)&&(point.y>=330&&point.y<=336)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(15)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=338&&point.x<=426)&&(point.y>=410&&point.y<=418)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(16)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=340&&point.x<=426)&&(point.y>=491&&point.y<=497)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(17)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=381&&point.x<=387)&&(point.y>=280&&point.y<=333)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(18)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=423&&point.x<=429)&&(point.y>=332&&point.y<=413)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(19)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=423&&point.x<=429)&&(point.y>=413&&point.y<=493)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(20)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=384&&point.x<=476)&&(point.y>=277&&point.y<=283)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(21)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=384&&point.x<=426)&&(point.y>=330&&point.y<=336)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(22)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=426&&point.x<=476)&&(point.y>=330&&point.y<=335)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(23)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=426&&point.x<=476)&&(point.y>=410&&point.y<=416)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(24)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=426&&point.x<=476)&&(point.y>=490&&point.y<=496)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(25)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=473&&point.x<=477)&&(point.y>=218&&point.y<=277)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(26)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=473&&point.x<=477)&&(point.y>=277&&point.y<=330)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(27)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=473&&point.x<=479)&&(point.y>=330&&point.y<=413)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(28)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=473&&point.x<=479)&&(point.y>=413&&point.y<=492)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(29)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=476&&point.x<=506)&&(point.y>=215&&point.y<=222)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(30)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=477&&point.x<=493)&&(point.y>=274&&point.y<=288)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(31)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=493&&point.x<=503)&&(point.y>=265&&point.y<=285)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(32)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=477&&point.x<=493)&&(point.y>=285&&point.y<=328)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(33)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=476&&point.x<=505)&&(point.y>=327&&point.y<=333)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(34)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=476&&point.x<=505)&&(point.y>=410&&point.y<=416)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(35)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=476&&point.x<=538)&&(point.y>=489&&point.y<=495)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(36)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=502&&point.x<=508)&&(point.y>=40 &&point.y<=68 )&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(37)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=502&&point.x<=508)&&(point.y>=68 &&point.y<=123)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(38)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=504&&point.x<=508)&&(point.y>=123&&point.y<=218)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(39)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=502&&point.x<=508)&&(point.y>=218&&point.y<=265)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(40)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=502&&point.x<=508)&&(point.y>=265&&point.y<=330)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(41)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=502&&point.x<=508)&&(point.y>=330&&point.y<=413)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(42)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=505&&point.x<=538)&&(point.y>=65 &&point.y<=71 )&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(43)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=505&&point.x<=538)&&(point.y>=120&&point.y<=126)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(44)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=505&&point.x<=538)&&(point.y>=215&&point.y<=221)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(45)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=505&&point.x<=538)&&(point.y>=327&&point.y<=333)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(46)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=505&&point.x<=538)&&(point.y>=410&&point.y<=416)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(47)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=535&&point.x<=541)&&(point.y>=68 &&point.y<=123)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(48)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=535&&point.x<=541)&&(point.y>=123&&point.y<=218)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(49)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=535&&point.x<=539)&&(point.y>=218&&point.y<=286)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(50)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=535&&point.x<=539)&&(point.y>=286&&point.y<=330)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(51)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=535&&point.x<=541)&&(point.y>=330&&point.y<=413)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(52)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=535&&point.x<=541)&&(point.y>=413&&point.y<=491)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(53)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=530&&point.x<=541)&&(point.y>=491&&point.y<=596)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(54)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=538&&point.x<=612)&&(point.y>=327&&point.y<=331)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(55)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=538&&point.x<=584)&&(point.y>=410&&point.y<=416)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(56)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=584&&point.x<=648)&&(point.y>=412&&point.y<=424)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(57)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=541&&point.x<=595)&&(point.y>=491&&point.y<=506)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(58)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=595&&point.x<=642)&&(point.y>=503&&point.y<=511)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(59)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=610&&point.x<=631)&&(point.y>=330&&point.y<=363)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(60)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=631&&point.x<=651)&&(point.y>=363&&point.y<=386)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(61)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=647&&point.x<=653)&&(point.y>=385&&point.y<=424)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(62)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=646&&point.x<=651)&&(point.y>=424&&point.y<=456)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(63)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=645&&point.x<=648)&&(point.y>=456&&point.y<=469)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(64)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=640&&point.x<=647)&&(point.y>=469&&point.y<=508)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(65)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=653&&point.x<=710)&&(point.y>=385&&point.y<=408)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(66)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=650&&point.x<=665)&&(point.y>=422&&point.y<=427)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(67)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=642&&point.x<=678)&&(point.y>=506&&point.y<=515)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(68)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=678&&point.x<=701)&&(point.y>=509&&point.y<=515)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(69)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=701&&point.x<=712)&&(point.y>=515&&point.y<=529)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(70)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=710&&point.x<=723)&&(point.y>=408&&point.y<=432)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(71)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=720&&point.x<=725)&&(point.y>=433&&point.y<=478)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(72)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=711&&point.x<=722)&&(point.y>=478&&point.y<=530)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(73)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=529&&point.x<=538)&&(point.y>=215&&point.y<=221)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(74)==TRUE) {SetTimer(1,speed,NULL);}
		else if((point.x>=508&&point.x<=535)&&(point.y>=223&&point.y<=265)&&xi==node[Link->id].x&&yi==node[Link->id].y&&JoinTable(75)==TRUE) {SetTimer(1,speed,NULL);}
	}
	
	CDialog::OnLButtonDblClk(nFlags, point);
}
/***************************************************************************************************************************************/
/*��ͨ��������*/
/***************************************************************************************************************************************/
int* Array() //���û�ѡ�����˳������
{
	//�����������鲢��ѡ��ID��������
	int store,client,num=4,i; //num����ͳ����������
	for(client=51;client<=71;client++) //client��¼�����յ��ID
		if(node[client].choose == CLIENTFLAG)
			num++;
	int *order=(int*)malloc(sizeof(int)*num); //�洢���˳�������
	for(store=51;store<=71;store++) //store��¼���ǲֿ��ID
		if(node[store].choose==STOREFLAG)
			break;

	for(i=2,client=51;client<=71;client++) //�ѿͻ�ID��������
		if(node[client].choose==CLIENTFLAG)
			order[i++]=client;
	order[1]=store; //�ײֿ�
	order[num-2]=store; //β�ֿ�
	order[num-1]=0; //������־

	return order;
}
/***************************************************************************************************************************************/
int* Product(int *order,int n) //�����½�(��ָ��ı�����˳��)
{
	srand((unsigned)time(NULL)); //�������
	int *p,tmp;

	int i=0;
	while(*(order+2+i+1) != 0)//i�������һ��Ԫ����
	{
		p=order+2+(i+rand()%(n-i-4));
		
		tmp=*p;
		*p=*(order+2+i);
		*(order+2+i)=tmp;		
			
		i++;
	}	
	
	//����·��˳���Ż�
	FreeRoute(FindCircle(order));

	return order;
}
/***************************************************************************************************************************************/
ROUTE* FindRoute(int *store,int *client)
{
	node[*store].father=*store; //Ϊ�״ι�����׼��

	ASTAR *head1,*head2,*head3,*start,*inlink,*inlink1,*clink,*tlink; //1����OPEN�� 2����CLOSE��
	ASTAR *min;
	int   i; //�����Χ��id����
	int  *p; //ָ�������ָ�벢����
	
	head1=(ASTAR*)malloc(sizeof(ASTAR)); //����ͷ���ռ�(OPEN)
	head2=(ASTAR*)malloc(sizeof(ASTAR)); //����ͷ���ռ�(CLOSE)
	head3=(ASTAR*)malloc(sizeof(ASTAR)); //����ͷ���ռ�(TRAFFIC)
	inlink=head1; //���б�������
	clink=head2; //CLOSE��������
	tlink=head3; //TRAFFIC��������
	start=(ASTAR*)malloc(sizeof(ASTAR)); //����OPEN���׽��
	
	//OPEN���ʼ��
	start->id=*store; //�׽��Ϊ�ֿ�
	start->center=0;
	start->g=0;
	start->price=fabs(node[*store].x-node[*client].x)+fabs(node[*store].y-node[*client].y); //�������f(n)ֵ
	inlink->next=start; //�½��������β
	inlink=start; //�ƶ�ָ������
	inlink->next=NULL; //��ʼ��������־

	//CLOSE���ʼ��
	clink->next=NULL; //�����׽��,�����

	//TRAFFIC���ʼ��
	tlink->next=NULL;//�����

	//�����·��������OPEN��
	while(true) //�˳����������ҵ��յ�
	{
		inlink=head1->next;
		if(inlink == NULL) //����ӵ����·̫���Ѿ���չ�����н�㣬�ҳ�ӵ���ڵ���h��С��һ����min
		{
			inlink1=head3->next;
			min=inlink1;
			while(inlink1 != NULL)
			{
				if(inlink1->price-inlink1->g < min->price-min->g) //��h˭С
				{
					min=inlink1;
				}
				inlink1=inlink1->next;
			}	
			node[min->id].father=min->center; //ָ�����Ľڵ�
		}
		else
		{
			min=inlink;
		}
			
		while(inlink != NULL) //��OPEN����f(n)��С�Ľ�㣨��minָ����Ϊ��ǰ���
		{
			if((inlink->price) == (min->price) && node[inlink->id].choose == CLIENTFLAG) //��·�����û���ǵĿͻ�
			{
				node[inlink->id].choose=HIDEFLAG; //���Ϊ�����ͣ�������·��ʱ��Ҫ�����CLIENTFLAG
				min=inlink;
				break;
			}
			if((inlink->price) < (min->price) || ((inlink->price)==(min->price) && (fabs(node[*client].x-node[inlink->id].x)+fabs(node[*client].y-node[inlink->id].y) < fabs(node[*client].x-node[min->id].x)+fabs(node[*client].y-node[min->id].y))) || min->id == 30 && inlink->id == 49) //��price˭С����һ����h˭С
				min=inlink;	
			inlink=inlink->next; //����
		}

		if((min->id) == *client) //��չ��Ŀ��ͻ�
		{
			start=(ASTAR*)malloc(sizeof(ASTAR)); //������һ�����Ҳ����CLOSE��
			start->id=min->id;
			start->center=0;
			start->g=min->g;
			clink->next=start;
			clink=start;
			clink->next=NULL;

			break;
		}

		//���*store��*client���ص�֮���Ƿ�Ҫ��;������������������ͻ�
		p=client; //ָ��ָ��client��id
		while(*(p+1) != 0) //�ҵ�����pλ��
		{
			if(min->id == *p)//����·�ͻ��ᵽǰ������
			{
				if(*(client+1) == *p)//������� ��·�ͻ�ǡ��client֮��һ��
				{
					int tmp;
					tmp=*client;*client=*p;*p=tmp;
				}
				else//һ�����
				{
					int tmp;
					tmp=*client;*client=*(client+1);*(client+1)=tmp;
					tmp=*client;*client=*p;*p=tmp;
				}
			
				start=(ASTAR*)malloc(sizeof(ASTAR));//������һ�����Ҳ����CLOSE��
				start->id=min->id;
				start->center=0;
				start->g=min->g;
				clink->next=start;
				clink=start;
				clink->next=NULL;
				
				goto ADJUST;
			}
			p++;//ָ����������
		}

	    i=0; //ÿ�ζ��Ǵ����ڽ���еĵ�һ����ʼ����
NEXT:	while((node[min->id].aroundid[i]) != 0) //�鼰��ǰ�����������ڽ��
		{
			crowd=CheckTraffic(min->id,node[min->id].aroundid[i]); //���Ľ���������ڽ��������·�Ƿ�ӵ��
			
			//���������TRAFFIC��
			//�������Ȳ�����Χ���Ƿ��Ѿ���CLOSE����,�ų�����ǰ��Щ����ĵ�ӽ�ȥ
			inlink=head2;
			while(inlink != NULL)
			{
				if((inlink->id) == (node[min->id].aroundid[i]))
					break;
				inlink=inlink->next; //����
			}
			if(inlink == NULL && crowd == FALSE) //û��CLOSE���У����node[min->id].aroundid[i]��ӽ�TRAFFIC��
			{
				inlink1=head3->next; //����TRAFFIC��ͷ
				while(inlink1 != NULL) //���Ҹ����ڽ���Ƿ��Ѿ���TRAFFIC������
				{
					if(inlink1->id == node[min->id].aroundid[i] && inlink1->center == min->id) //��ͬ���
						break;
					inlink1=inlink1->next;
				}
				if(inlink1 == NULL) //������ӵ�����
				{
					start=(ASTAR*)malloc(sizeof(ASTAR));
					start->id=node[min->id].aroundid[i];
					start->center=min->id;
					start->g=(min->g)+sqrt(pow(node[min->id].x-node[node[min->id].aroundid[i]].x,2)+pow(node[min->id].y-node[node[min->id].aroundid[i]].y,2));//�������g(n)
					start->price=(start->g)+fabs(node[node[min->id].aroundid[i]].x-node[*client].x)+fabs(node[node[min->id].aroundid[i]].y-node[*client].y);//�������f(n)
					tlink->next=start;
					tlink=start;
					tlink->next=NULL;
				}		
			}

			//Ϊ���ڽ��ѡ�����ǵĸ�����ǰ������
			if(  plan == 0 //���·��
			  ||(plan == 1 && crowd) //���ʱ���·����
			  ||(CheckSurvive(min->id)) //�������·����
			  ||(node[min->id].aroundid[i] == *client) //�յ������ǰ
			  ||(CheckSurvive(node[min->id].aroundid[i]))) //�ӵ���·����
			{
				//����IDΪmin->node�����Ľ������ڽ���Ƿ��Ѿ���OPEN����
				inlink=head1->next; //����OPEN��ͷ
				while(inlink != NULL)
				{
					double newprice=min->g+sqrt(pow(node[min->id].x-node[node[min->id].aroundid[i]].x,2)+pow(node[min->id].y-node[node[min->id].aroundid[i]].y,2))+fabs(node[node[min->id].aroundid[i]].x-node[*client].x)+fabs(node[node[min->id].aroundid[i]].y-node[*client].y); //�����µ�price
					if((inlink->id) == (node[min->id].aroundid[i])) //���ڽ��ȷʵ�Ѿ���OPEN����	
					{
						if((newprice) < (inlink->price)) //�������ǰ�Ǹ�priceС����²��޸��丸���
						{
							node[node[min->id].aroundid[i]].father=(min->id);//����ǰ���Ľ������Ϊ�����ڽ��ĸ����
							inlink->g=(min->g)+sqrt(pow(node[min->id].x-node[node[min->id].aroundid[i]].x,2)+pow(node[min->id].y-node[node[min->id].aroundid[i]].y,2));//�޸�g(n)
							inlink->price=newprice;//�޸�f(n)
						}
						i++;
						goto NEXT; //�����¸����ڽ��
					}
					inlink=inlink->next;	
				}	

				//�����Ƿ��Ѿ���CLOSE���У������������Ľ�㣩
				inlink=head2;
				while(inlink != NULL)
				{
					if((inlink->id) == (node[min->id].aroundid[i]))
					{
						i++;
						goto NEXT;
					}
					inlink=inlink->next;
				}
				
				//��ִ�е���һ��˵���Ǽ�û��OPEN��Ҳû��CLOSE�е�������
				node[node[min->id].aroundid[i]].father=(min->id); //ָ��ø��ڵ�			
			
				start=(ASTAR*)malloc(sizeof(ASTAR)); //�˽���Ѿ�����ô����OPEN����
				start->id=node[min->id].aroundid[i];
				start->center=0;
				start->g=(min->g)+sqrt(pow(node[min->id].x-node[node[min->id].aroundid[i]].x,2)+pow(node[min->id].y-node[node[min->id].aroundid[i]].y,2));//�������g(n)
				start->price=(start->g)+fabs(node[node[min->id].aroundid[i]].x-node[*client].x)+fabs(node[node[min->id].aroundid[i]].y-node[*client].y);//�������f(n)
				
				inlink=head1;//��OPEN���β�����½�����
				while(inlink->next != NULL)
					inlink=inlink->next;
				inlink->next=start;
				inlink=start;
				inlink->next=NULL;
			}
			i++; //��һ�����
		}

		//��ǰ��㿼����ϣ�����CLOSE��
		start=(ASTAR*)malloc(sizeof(ASTAR));		
		start->id=min->id;start->center=0;start->g=min->g;
		clink->next=start;
		clink=start;
		clink->next=NULL;
	
		//�������OPEN����ɾ��
		if(min->center == 0) //���Ǹ����ſ���ɾ��
		{
			inlink1=head1;inlink=head1->next;
			while(inlink->id != min->id)
			{
				inlink1=inlink;
				inlink=inlink->next;
			}
			inlink1->next=inlink->next;
			free(inlink);inlink=NULL;
		}
	}

ADJUST:
	//�ͷ�OPEN��
	FreeAstar(head1);

	//Ѱ��CLOSE�������һ��Ԫ��ȡ��g(n)·��ֵ�Ժ��Head->id
	inlink=head2;
	while(inlink->next != NULL)
		inlink=inlink->next;

	//�����ݻ�·������	
	ROUTE  *head; //���·������ͷ���
	head=(ROUTE*)malloc(sizeof(ROUTE));
	link=head;
	link->id=(int)inlink->g;link->next=NULL;

	//�ͷ�CLOSE��
	FreeAstar(head2);
	//�ͷ�TRAFFIC��
	FreeAstar(head3);

	node[*store].father=UNKNOWN;
	Goback(*client); //Ѱ��·�ߡ�װ��������ʱֻҪ֪��client��һ����׷�������·�ߣ�
	
	return head;
}
/***************************************************************************************************************************************/
ROUTE* FindCircle(int *order) //����Ѳ������
{
	//���ڽ��ղ���������
	ROUTE *head;
	head=(ROUTE*)malloc(sizeof(ROUTE)); //��Ѳ������ͷָ��
	Start=(ROUTE*)malloc(sizeof(ROUTE));
	head->id=0;head->next=Start;
	Start->id=*(order+1);Start->next=NULL;
	lin=Start;

	for(int i=1;*(order+i+1)!=0;i++) //�������е�˳�����������·�̡���·��
	{		
		//�ұ�β
		while(lin->next!=NULL)
			lin=lin->next;
			
		//�ۼ�·�߳���	
		lin->next=FindRoute(order+i,order+i+1); //����ȡǰ���ν�����������
		head->id+=(lin->next)->id; //�Ѳ�����·�߳����ۼӵ�����ͷ�����

		//����������
		lin->next=((lin->next)->next)->next;	

		//��ո������ĸ������Ϣ
		for(int j=1;j<=71;j++)
			node[j].father=UNKNOWN;
	}

	//����ղ���ʱ���صĿͻ����
	for(i=51;i<=71;i++)
		if(node[i].choose==HIDEFLAG)
			node[i].choose=CLIENTFLAG;

	return head;
}
/***************************************************************************************************************************************/
ROUTE* Cool(int *order,int num) //�˻��㷨������Ѳ������
{	
	double    r;   //r:���ڿ��ƽ��µĿ���
	double    T;   //T:ϵͳ�¶�,ϵͳ��ʼӦ��Ҫ����һ�����µ�״̬ 
	double    T_min; //T_min:�¶ȵ����ޣ����¶�T�ﵽT_min����ֹͣ����
	double    dE,de;    //dE:��ֵ�� de:����� 
		
	int *order1=(int*)malloc(sizeof(int)*num);//������������
	for(int i=0;i<num;i++)
		order1[i]=order[i];

	
	ROUTE    *old,*fresh,*remember; //old:��ǰ����fresh:�½�������ʱ���죩��remember:��������
	int      stand=0; //����һ��������������������
	int      times=0; //ͬ�ļ�����ʷ��С�Ƚϴ�С��������
	
	//�񵴳�ʼ����10��
	if(num > 4) 
	{
		for(i=0;i<10;i++)
			order=Product(order,num);
	}

	//���߳�ʼ��
	remember=old=FindCircle(order);
	if(mode == 1)	    remove("mindistance.txt"); //����������ģʽ
	old=Recool(old,1); //����д���ļ����Ž�
	if(num > 4) order=Product(order,num); //Ϊ��һ��fresh׼��


	r=0.998;
	T=5000;
	T_min=10;
	stand=0;

	while(T > T_min) //δ������¶Ⱦͼ�������
	{
		fresh=FindCircle(order); //����������

		dE=(double)((fresh->id)-(old->id)); //�½���ɽ�ȳ���
		de=(double)((fresh->id)-(remember->id)); //�½�������ȳ���

		if(dE < 0 && de < 0)
			fresh=Recool(fresh,1); //fresh���ļ����Ž�Ƚϣ�С�򸲸�

		if(stand <= 50)//50�α������
		{
			//fresh��remember�Ƚ�
			if(de > 0)//�ȼ���Ĵ�
			{
				stand++; //˵���ж�����������һ�α������
			}
			else//�ȼ����С
			{
				stand--; //˵�����������ƶ��һ�α������
			}

			//fresh��old�Ƚ�
			if (dE <= 0) //�½��С
			{
				if(old != NULL && old != remember && old != fresh)
					FreeRoute(old); //�����ɽ�
				old=fresh; //�����½�

				//���Ҿ����飬����������
				if(num > 4) order=Product(order,num);
			}
			else //�½���˵�
			{
				if(exp(-dE/T) > (double)(rand()%101)/100 && stand == 0) //exp(-dE/T)�ĸ��ʽ��ܴ�fresh
				{
					//�Դ�ʱ��old�����������remember�Ƚ�
					if(remember->id > old->id)
					{
						if(remember != NULL && remember != fresh && remember != old)
							FreeRoute(remember); //��������
						remember=old;//�����¼�������
						for(int i=0;i<num;i++)//�����¼��������˳��
							order1[i]=order[i];	
					}
					old=fresh;//��ǿ�����½�
				}
				
				if(num > 4) order=Product(order,num);
			}
		}
		else //��ֹ���Ա���
		{
			stand=0;//���¸����¸���������5�α������
			if(old != NULL && old != fresh && old != remember)
				FreeRoute(old); //��������
			old=remember; //�Ѽ��������������
			//�½��ɼ������order1����
			if(num>4) order=Product(order1,num);
		}

		//�ͷ�fresh
		if(fresh != NULL && fresh != old && fresh != remember)
			FreeRoute(fresh); //��������
		
		T=r*T;  //�����˻�
	}

	free(order1);order1=NULL; //�ͷż�������
	if(remember != NULL && remember != old) FreeRoute(remember);
	if(adjust == TRUE) remove("adjust.txt");
	
	old=Recool(old,5); //old��ȡ�ļ����Ž�

	finalArray=order;
	return old;
}
/***************************************************************************************************************************************/
ROUTE* Recool(ROUTE *old,int times) //������
{
	int  record; //����¼ֵ
	FILE *rp,*wp; //����д�ļ�ָ��

	if(adjust == FALSE)	rp=fopen("mindistance.txt","r");
	else				rp=fopen("adjust.txt","r");

	if(rp == NULL) //û���ļ����ڣ�д����������
	{
		if(adjust == FALSE)	wp=fopen("mindistance.txt","w");
		else				wp=fopen("adjust.txt","w"); //��д�ķ�ʽ��
		
		link=old; //д����������
		while(link != NULL)
		{
			fprintf(wp,"%d ",link->id);
			link=link->next;
		}

		fclose(wp);
		return old;
	}
	else
	{
		fscanf(rp,"%d ",&record); //����ɼ�¼
		fclose(rp);
		if(old->id <= record) //�¼�¼
		{
			if(adjust == FALSE)	wp=fopen("mindistance.txt","w");
			else				wp=fopen("adjust.txt","w");
			
			link=old; //д����������
			while(link != NULL)
			{
				fprintf(wp,"%d ",link->id);
				link=link->next;
			}

			fclose(wp);
			return old;
		}
		else //����
		{
			if(times == 5) //�ﵽ�����Ҵ���
			{
				if(adjust == FALSE)	rp=fopen("mindistance.txt","r");
				else				rp=fopen("adjust.txt","r");

				fscanf(rp,"%d ",&record);
				ROUTE *history=(ROUTE*)malloc(sizeof(ROUTE));
				history->id=record;
				history->next=NULL;

				link=history;
				while(!feof(rp)) //һֱ�����ļ�β��
				{
					fscanf(rp,"%d ",&record);
				
					Start=(ROUTE*)malloc(sizeof(ROUTE));
					Start->id=record;
					Start->next=NULL;
					link->next=Start;
					link=Start;
				}
				fclose(rp);

				FreeRoute(old);
				return history;
			}
			else return old;
		}
	}
}
/***************************************************************************************************************************************/
BOOL CheckBlank(CString str) //���������Ϣ�Ƿ�Ϊ��
{
	if(str == "")
		return FALSE;
	else
		return TRUE;
}
/***************************************************************************************************************************************/
BOOL CheckNum(CString str) //������������Ƿ�Ƿ�(ֻ��������)
{
	BOOL firstnum=FALSE,firstpoint=FALSE; //�ֱ��ʾ��λ����δ���롢��һ��С����δ����
	
	if(atof(str) <= 0)
	{
		return FALSE;
	}

	for(int i=0;i<str.GetLength();i++)
	{
		if(str.GetAt(i) >= '0' && str.GetAt(i) <= '9') //�����������
		{
			firstnum=TRUE;
			continue;
		}
		else if(str.GetAt(i) == '.' && firstnum == TRUE && firstpoint == FALSE) //��ȷС����ʽ(.ֻ������һ���Ҹ�λ��������)
		{
			firstpoint=TRUE;
			continue;
		}
		else //����������Ƿ�
			return FALSE;
	}
	
	return TRUE;
}
/***************************************************************************************************************************************/
BOOL CheckSurvive(int one) //����Ƿ���·����
{
	if(one >= 51 && one <= 71) //�ǿͻ����
	{
		return FALSE;
	}
	else
	{
		int i=0;
		//���ȼ���ǲ�����·���ߵ����
		while(node[one].roadid[i]!=0)
		{
			int k=node[one].roadid[i];
			if(road[k].traffic == SMOOTH) //������һ����·���߾������˳�����ʱû�е�������־
				break;
			i++;
		}
		if(node[one].roadid[i] == 0) //���е�·���Ѳ��꣬����δ���ҵ�������
		{
			return TRUE; //�������ý��
		}
		else
		{
			return FALSE;
		}
	}
}
/***************************************************************************************************************************************/
BOOL CheckTraffic(int one,int two) //���������㹲�е�·�Ƿ�ӵ��
{	
	//���Ѿ�ȷ�����������������·��ռ����
	for(int i=0;node[one].roadid[i]!=0;i++) //��first��Χ����·��second��ͬ
	{
		int r=node[one].roadid[i]; //������Χ��·id,�ӿ��ٶ���
		for(int j=0;node[two].roadid[j]!=0;j++) //��second��Χ����·��first��ͬ
		{	
			int m=node[two].roadid[j];
			if(r == m) //ȷ��one��two������ͬ�ĵ�·
			{
				if(road[r].traffic == SMOOTH) //����
					return TRUE;
				else //æµ
					return FALSE;
			}	
		}
	}

	return TRUE;
}
/***************************************************************************************************************************************/
BOOL CheckNode(int first,int second) //����first����Χ�Ƿ����second��
{
	for(int j=0;node[first].aroundid[j]!=0;j++)
		if(node[first].aroundid[j] == second) //��Χȷʵ��second��
			return TRUE;

	return FALSE;
}
/***************************************************************************************************************************************/
void ChangeCircle() //��̬�滮��·��
{
	//�Ҿɱ����һ���ͻ���˭����linksָ��
	ROUTE *links;
	int   nextplace=0;

	//��ʣ��·�����ĸ��ͻ����뵱ǰ���ڽ�����
	if(node[Link->id].choose != CLIENTFLAG) //�ų���һ�������ǿͻ��������
	{
		links=Link;
		while(links->next != NULL)
		{
			if(links->id >= 51 && links->id <= 71 && node[links->id].choose != STOREFLAG) //�ͻ���
			{
				nextplace=links->id;
				break;
			}
			links=links->next;
		}
		if(nextplace == 0) //ֻʣ���һ���ֿ�����
		{
			nextplace=links->id;
		}
	}
	else
	{
		nextplace=Link->id;
	}
			
	//�򿪷�����ʱ���صĿͻ����
	for(int i=51;i<=71;i++)
		if(node[i].choose == HIDEFLAG)
			node[i].choose=CLIENTFLAG;	
		
/************************************/
/*��û�����͵��Ŀͻ�����������*/
/************************************/
		
	//�����ڴ�
	int num=3;
	for(i=51;i<=71;i++)
		if(node[i].choose == CLIENTFLAG && node[i].been == FALSE)
			num++;
	int *order;
	order=(int*)malloc(sizeof(int)*num);
	
	//��Head����û�����͵Ŀͻ�������order���
	i=2;
	links=Link;
	while(links->next != NULL)
	{
		if(links->id != nextplace && links->id >= 51 && links->id <= 71)
		{
			*(order+i)=links->id;
			i++;
		}
		links=links->next;
	}
	*(order+1)=nextplace;
	*(order+num-2)=links->id;
	*(order+num-1)=0;

//��ʱ��orderΪ�׵�ַ������洢�ľ���Head->id·�������ʣ���˳��

	//���Һ���minplace�ĵ�һ��ָ��
	links=Link;
	while(links->id != nextplace)
		links=links->next;
	
	if(CheckCool(links) == TRUE) //��Ҫ��ʣ��ͻ������˻�
	{
		//����·�����µ�Head
		if(Link->id < 51) //����һ������ͨ��㣬������ͨ���---��һ�����ľ���
		{
			FreeRoute(Link->next);
			Link->next=FindRoute(&(Link->id),&(nextplace))->next->next;
		}
		//���±��minplace
		links=Link;
		while(links->id != nextplace)
			links=links->next;
		links->next=Cool(order,num)->next->next; //�ٴ������˻��㷨��СѲ��
	}
	else
	{
		if(Link->id < 51) //����һ������ͨ��㣬������ͨ���---��һ�����ľ���
		{
			Link->next=FindRoute(&(Link->id),&(nextplace))->next->next;
		}

		ROUTE *front=Link;
		while(front->next != NULL)
			front=front->next;

		front->next=links->next;
	}
}
/***************************************************************************************************************************************/
void Goback(int son) //����·�ߡ�װ������
{
	if(son != UNKNOWN)//δ��store�ĸ����UNKNOWN
	{
		Goback(node[son].father);//�ݹ���ȥ
		
		Start=(ROUTE*)malloc(sizeof(ROUTE));//��̬�����½��
		Start->id=son;
		link->next=Start;
		link=Start;
		link->next=NULL;
	}
	else
	{
		return;
	}
}
/***************************************************************************************************************************************/
void FreeAstar(ASTAR *h) //�ͷ�ASTAR����
{
	ASTAR *inlink=h;
	while(inlink != NULL)
	{
		h=inlink;
		inlink=inlink->next;
		free(h);
		h=NULL;
	}
}
/***************************************************************************************************************************************/
void  FreeRoute(ROUTE *h) //�ͷ�ROUTE����
{
	ROUTE *inlink=h;
	while(inlink != NULL)
	{
		h=inlink;
		inlink=inlink->next;
		free(h);
		h=NULL;
	}
}
/***************************************************************************************************************************************/
void AddRunning(int first,int second) //��·ռ�����
{
	//���Ѿ�ȷ�����������������·��ռ����
	for(int i=0;node[first].roadid[i]!=0;i++) //��first��Χ����·��second��ͬ
	{
		int r=node[first].roadid[i]; //������Χ��·id,�ӿ��ٶ���
		for(int j=0;node[second].roadid[j]!=0;j++) //��second��Χ����·��first��ͬ
		{	
			if(r == node[second].roadid[j]) //ȷ��first��secondռ�õĵ�·һ��
				road[r].running=TRUE;
		}
	}
}
/***************************************************************************************************************************************/
void ChooseA(double need) //ȷ������
{
	//ɸѡ���������
	if	   (need > 0 && need <= 4 ) {P=120; O=0.08; M=2+need;FprintfA(2.0);}
	else if(need > 4 && need <= 8 ) {P=160; O=0.12; M=5+need;FprintfA(5.0);}
	else if(need > 8 && need <= 12) {P=200; O=0.16; M=8+need;FprintfA(8.0);}	
}
/***************************************************************************************************************************************/
void FprintfA(double G) //����д���ļ�
{
	FILE *wp=fopen("CarA.txt","w");	
	fprintf(wp,"%lf %lf %lf ",P,G,O);
	fclose(wp);
}
/***************************************************************************************************************************************/
void CalculateTime() //����ʱ��
{
	T=0;
	ROUTE *s=Head->next;
	while(s->next != NULL)
	{
		T+=(CalculateDis(s->id,s->next->id)/30)/CalculateSpeed(s->id,s->next->id);
		s=s->next;
	}
}
/***************************************************************************************************************************************/
void CalculateHead() //����Head������·��
{
	Head->id=0;
	ROUTE *l=Head->next;		
	while(l->next != NULL)
	{
		Head->id+=(int)CalculateDis(l->id,l->next->id); //�ۼ�·��
		l=l->next;
	}
}
/***************************************************************************************************************************************/
double CalculateDis(int first,int second) //�������ڵ����
{
	return sqrt(pow(node[first].x-node[second].x,2)+pow(node[first].y-node[second].y,2));
}
/***************************************************************************************************************************************/
double CalculateSpeed(int first,int second) //�����ٶ�
{
	if(CheckTraffic(first,second) == FALSE) //���ڵ�·ӵ��
	{
		return 3.6+(double)(rand()%6)/10; //�ٶ�Ϊ3.6km/h����
	}
	else //����
	{
		return P/M*3.6-7+rand()%15;	
	}
}
/***************************************************************************************************************************************/
BOOL CMyDlg::JoinTable(int r) //��⴫��·��r�Ƿ��ͬ�ϴ�·���ν�,���������Head����
{
	int first,second;
	
	//����r·�εĽڵ���������
	//�����ҿͻ����
	for(int i=51;i<=71;i++)
	{
		for(int j=0;node[i].roadid[j]!=0;j++)
		{
			if(node[i].roadid[j] == r && CheckNode(i,lastnode) == TRUE && (node[i].choose == CLIENTFLAG || node[i].choose == STOREFLAG)) //Ԥʾ�ýڵ㺬���βε�·,�����������ϴε���Ľڵ�
			{
				lastnode=i; //�����ϴνڵ�

				//��i����ڵ����ӵ�Head������
				Start=(ROUTE*)malloc(sizeof(ROUTE));
				Start->id=i;Start->next=NULL;
				first=Link->id;
				Link->next=Start;
				Link=Start;second=Link->id;
				sonx=node[second].x;
				sony=node[second].y;

				//�ı���Ϣ��ӳ
				CalculateHead();
				ChooseA(atof(m_NeedEdit));
				V=CalculateSpeed(first,second);
				Statistic();

				return TRUE;
			}
		}
	}

	//�������ͨ���
	for(i=1;i<=49;i++)
	{
		for(int j=0;node[i].roadid[j]!=0;j++)
		{
			if(node[i].roadid[j] == r && CheckNode(i,lastnode) == TRUE) //Ԥʾ�ýڵ㺬���βε�·,�����������ϴε���Ľڵ�
			{
				lastnode=i; //�����ϴνڵ�

				//��i����ڵ����ӵ�Head������
				Start=(ROUTE*)malloc(sizeof(ROUTE));
				Start->id=i;Start->next=NULL;
				first=Link->id;
				Link->next=Start;
				Link=Start;second=Link->id;
				sonx=node[second].x;
				sony=node[second].y;
					
				//�ı���Ϣ��ӳ
				CalculateHead();
				ChooseA(atof(m_NeedEdit));
				V=CalculateSpeed(first,second);
				Statistic();

				return TRUE;
			}
		}
	}

	return FALSE; //�˵�·���ɼ�
}
/***************************************************************************************************************************************/
void CMyDlg::CutScreen() //��ͼ
{
	memDC1.BitBlt(0,0,rt1.Width(),rt1.Height(),pDC1,0,0,SRCCOPY);
}
/***************************************************************************************************************************************/
void CMyDlg::PutScreen() //��ʾ��ͼ
{
	pDC1->BitBlt(179,11,700,700,&memDC1,179,11,SRCCOPY);
}
/***************************************************************************************************************************************/
void CMyDlg::ChangeAnytime(int first,int second) //�ٶ���·����ʱ����
{
	V=CalculateSpeed(first,second); //����V
	m_SpeedStr.Format("%.2lf",V); //�����һ���ٶ�\·��
	if(V >= 3.1 && V <= 4.1)  {m_TrafficStr="ӵ��";speed=60;SetTimer(1,60,NULL);} //�����ٶȸı�·��
	else                      {m_TrafficStr="����";speed=15;SetTimer(1,5,NULL);}
	UpdateData(FALSE);
}
/***************************************************************************************************************************************/
void CMyDlg::DrawLine() //���н�·��
{
	//�����Ƶ�
	CDC *pDC=GetDC();
	pDC->SelectObject(&pen1); //������ѡ���豸������		
	//����ʼ��
	pDC->MoveTo((int)xi,(int)yi);
	//��������
	if     (xi < sonx)	yi=((yi-sony)/(xi-sonx))*((++xi)-sonx)+sony;
	else if(xi > sonx)	yi=((yi-sony)/(xi-sonx))*((--xi)-sonx)+sony;
	else //б�ʲ��������
	{
		if     (yi < sony)  	yi++;
		else if(yi > sony)	    yi--;
	}
	//�����Ƶ�
	pDC->LineTo((int)xi,(int)yi);
	pDC->DeleteDC();
}
/***************************************************************************************************************************************/
void CMyDlg::Map() //��ͼͼ��
{
	CDC memDC,*pDC=GetDC();
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.LoadBitmap(IDB_Map);
	memDC.SelectObject(&bmp);
	pDC->BitBlt(179,11,600,600,&memDC,0,0,SRCCOPY);
	memDC.DeleteDC();
	pDC->DeleteDC();
	bmp.DeleteObject();
}
/***************************************************************************************************************************************/
void CMyDlg::TrafficFlag(int m) //��֮ǰ�����ӵ�µĵ�·���ػ�
{
	if(road[m].traffic == CROWD && ((timer == FALSE && road[m].then == FALSE) || timer == TRUE))
	{
		switch(m)
		{
			case 1 :OnPicture(219,231,225,271);break;
			case 2 :OnPicture(219,271,225,333);break;
			case 3 :OnPicture(219,333,225,416);break;
			case 4 :OnPicture(219,416,225,494);break;
			case 5 :OnPicture(224,268,306,274);break;
			case 6 :OnPicture(306,268,338,283);break;
			case 7 :OnPicture(222,330,338,336);break;
			case 8 :OnPicture(222,413,338,419);break;
			case 9 :OnPicture(222,491,340,497);break;
			case 10:OnPicture(337,228,343,280);break;
			case 11:OnPicture(337,277,343,333);break;
			case 12:OnPicture(337,333,343,414);break;
			case 13:OnPicture(337,414,343,494);break;
			case 14:OnPicture(340,277,384,283);break;
			case 15:OnPicture(340,330,384,336);break;
			case 16:OnPicture(338,410,426,418);break;
			case 17:OnPicture(340,491,426,497);break;
			case 18:OnPicture(381,280,387,333);break;
			case 19:OnPicture(423,332,429,413);break;
			case 20:OnPicture(423,413,429,493);break;
			case 21:OnPicture(384,277,476,282);break;
			case 22:OnPicture(384,330,426,336);break;
			case 23:OnPicture(426,330,476,335);break;
			case 24:OnPicture(426,410,476,416);break;
			case 25:OnPicture(426,490,476,496);break;
			case 26:OnPicture(473,218,477,277);break;
			case 27:OnPicture(473,277,477,330);break;
			case 28:OnPicture(473,330,479,413);break;
			case 29:OnPicture(473,413,479,492);break;
			case 30:OnPicture(476,215,506,222);break;
			case 31:OnPicture(476,274,493,288);break;
			case 32:OnPicture(493,265,503,285);break;
			case 33:OnPicture(477,285,493,328);break;
			case 34:OnPicture(476,327,505,333);break;
			case 35:OnPicture(476,410,505,416);break;
			case 36:OnPicture(476,489,538,495);break;
			case 37:OnPicture(502,40,508,68  );break;
			case 38:OnPicture(502,68,508,123 );break;
			case 39:OnPicture(504,123,508,218);break;
			case 40:OnPicture(502,218,508,265);break;
			case 41:OnPicture(502,265,508,330);break;
			case 42:OnPicture(502,330,508,413);break;
			case 43:OnPicture(505,65,538,71  );break;
			case 44:OnPicture(505,120,538,126);break;
			case 45:OnPicture(505,215,538,221);break;
			case 46:OnPicture(505,327,538,333);break;
			case 47:OnPicture(505,410,538,416);break;
			case 48:OnPicture(535,68,541,123 );break;
			case 49:OnPicture(535,123,541,218);break;
			case 50:OnPicture(535,218,539,286);break;
			case 51:OnPicture(535,286,539,330);break;
			case 52:OnPicture(535,330,541,413);break;
			case 53:OnPicture(535,413,541,491);break;
			case 54:OnPicture(535,491,541,596);break;
			case 55:OnPicture(538,327,612,331);break;
			case 56:OnPicture(538,410,584,416);break;
			case 57:OnPicture(584,412,648,424);break;
			case 58:OnPicture(541,491,595,506);break;
			case 59:OnPicture(595,503,642,511);break;
			case 60:OnPicture(610,330,631,363);break;
			case 61:OnPicture(631,363,651,386);break;
			case 62:OnPicture(647,385,653,424);break;
			case 63:OnPicture(646,424,651,456);break;
			case 64:OnPicture(645,456,651,469);break;
			case 65:OnPicture(640,469,647,508);break;
			case 66:OnPicture(653,385,710,408);break;
			case 67:OnPicture(650,422,665,427);break;
			case 68:OnPicture(642,506,678,515);break;
			case 69:OnPicture(678,509,701,515);break;
			case 70:OnPicture(698,515,712,529);break;
			case 71:OnPicture(710,408,723,432);break;
			case 72:OnPicture(720,433,725,478);break;
			case 73:OnPicture(711,478,722,530);break;
			case 74:OnPicture(529,215,538,221);break;
			case 75:OnPicture(508,223,535,265);break;
		}
	}
	
}
/***************************************************************************************************************************************/
void CMyDlg::StoreFlag() //�ֿ���ͼ��
{
	CDC memDC,*pDC=GetDC(); //�����豸�����Ķ���
	memDC.CreateCompatibleDC(pDC); //��̬����λͼ�ؼ�
	CBitmap bmp; //����λͼ����
	bmp.LoadBitmap(IDB_StoreFlag); //����λͼ��ַ
	memDC.SelectObject(&bmp); //��λͼѡ���豸������
	TransparentBlt(pDC->m_hDC,Point2.x-6,Point2.y-29,28,29,memDC,0,0,28,29,RGB(255,255,255)); //͸����ʾλͼ��ָ��λ��	
	memDC.DeleteDC(); //ɾ���豸�����Ķ���
	pDC->DeleteDC(); //ɾ���豸������ָ��
	bmp.DeleteObject(); //ɾ��λͼ����
}
/***************************************************************************************************************************************/
void CMyDlg::ClientFlag() //�ͻ���ͼ��
{
	CDC memDC,*pDC=GetDC();
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.LoadBitmap(IDB_ClientFlag);
	memDC.SelectObject(&bmp);
	TransparentBlt(pDC->m_hDC,Point2.x-6,Point2.y-29,28,29,memDC,0,0,28,29,RGB(255,255,255));	
	memDC.DeleteDC();
	pDC->DeleteDC();
	bmp.DeleteObject();
}
/***************************************************************************************************************************************/
void CMyDlg::BeenFlag() //�카ͼ��
{
	CDC memDC,*pDC=GetDC();
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.LoadBitmap(IDB_BeenFlag);
	memDC.SelectObject(&bmp);
	TransparentBlt(pDC->m_hDC,Point2.x-6,Point2.y-29,28,29,memDC,0,0,28,29,RGB(255,255,255));	
	memDC.DeleteDC();
	pDC->DeleteDC();
	bmp.DeleteObject();
}
/***************************************************************************************************************************************/
void CMyDlg::ShowBeen() //ɸѡ��
{
	for(int i=51;i<=71;i++) //��51-71���е����Ŀͻ���
	{
		if(node[i].been == TRUE)
		{
			switch(i)
			{
				case 51:Point2.x=232;Point2.y=231;BeenFlag();break;
				case 52:Point2.x=232;Point2.y=302;BeenFlag();break;
				case 53:Point2.x=232;Point2.y=382;BeenFlag();break;
				case 54:Point2.x=238;Point2.y=463;BeenFlag();break;
				case 55:Point2.x=320;Point2.y=228;BeenFlag();break;
				case 56:Point2.x=320;Point2.y=465;BeenFlag();break;
				case 57:Point2.x=455;Point2.y=378;BeenFlag();break;
				case 58:Point2.x=515;Point2.y=40 ;BeenFlag();break;
				case 59:Point2.x=515;Point2.y=104;BeenFlag();break;
				case 60:Point2.x=518;Point2.y=208;BeenFlag();break;
				case 61:Point2.x=515;Point2.y=295;BeenFlag();break;
				case 62:Point2.x=485;Point2.y=459;BeenFlag();break;
				case 63:Point2.x=546;Point2.y=165;BeenFlag();break;
				case 64:Point2.x=553;Point2.y=364;BeenFlag();break;
				case 65:Point2.x=546;Point2.y=395;BeenFlag();break;
				case 66:Point2.x=558;Point2.y=447;BeenFlag();break;
				case 67:Point2.x=546;Point2.y=475;BeenFlag();break;
				case 68:Point2.x=515;Point2.y=596;BeenFlag();break;
				case 69:Point2.x=680;Point2.y=432;BeenFlag();break;
				case 70:Point2.x=607;Point2.y=538;BeenFlag();break;
				case 71:Point2.x=733;Point2.y=453;BeenFlag();break;
			}
		}
	}
}
/***************************************************************************************************************************************/
void CMyDlg::DarkBlue(int x,int y) //��������
{
	CDC *pDC=GetDC();
	pDC->SelectObject(&pen2);
	pDC->MoveTo(x,y);
	pDC->LineTo(x,y);	
	pDC->DeleteDC();
}
/***************************************************************************************************************************************/
void CMyDlg::LightBlue(int x,int y) //��ǳ����
{
	CDC *pDC=GetDC();
	pDC->SelectObject(&pen3);
	pDC->MoveTo(x,y);
	pDC->LineTo(x,y);
	pDC->DeleteDC();
}
/***************************************************************************************************************************************/
void CMyDlg::AllBlue() //����������
{
	DarkBlue(232,231);DarkBlue(232,302);DarkBlue(232,382);DarkBlue(238,463);DarkBlue(320,228);
	DarkBlue(322,465);DarkBlue(515,40) ;DarkBlue(515,104);DarkBlue(546,165);DarkBlue(518,208);
	DarkBlue(515,295);DarkBlue(455,378);DarkBlue(485,459);DarkBlue(553,364);DarkBlue(546,395);
	DarkBlue(548,437);DarkBlue(546,475);DarkBlue(515,596);DarkBlue(630,515);DarkBlue(665,417);DarkBlue(733,453);
}
/***************************************************************************************************************************************/
void CMyDlg::OnPicture(int x1,int y1,int x2,int y2) //��·ѡ��
{
	CDC memDC,*pDC=GetDC();
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.LoadBitmap(IDB_OnMap);
	memDC.SelectObject(&bmp);
	pDC->BitBlt(x1,y1,x2-x1,y2-y1,&memDC,x1-179,y1-11,SRCCOPY);
	memDC.DeleteDC();
	pDC->DeleteDC();
	bmp.DeleteObject();
}
/***************************************************************************************************************************************/
void CMyDlg::Picture(int x1,int y1,int x2,int y2) //��·�ػ�
{
	CDC memDC,*pDC=GetDC();
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.LoadBitmap(IDB_Map);
	memDC.SelectObject(&bmp);
	pDC->BitBlt(x1,y1,x2-x1,y2-y1,&memDC,x1-179,y1-11,SRCCOPY);
	memDC.DeleteDC();
	pDC->DeleteDC();
	bmp.DeleteObject();
}
/***************************************************************************************************************************************/
void CMyDlg::Statistic() //��������̡��ܳɱ���ʱ�䡢�ٶȡ�·��
{
	//�����
	D=Head->id*1.0/30;
	m_DistanceStr.Format("%.2lf",D);
	
	//�ٶ����ⲿ�Ѹ�
	m_SpeedStr.Format("%.2lf",V);

	//ʱ��
	CalculateTime();//����ʱ��
	m_WholeStr.Format("%.2lf",T);

	//�ܳɱ�
	S=D*O*atof(m_FuelEdit)+atof(m_OtherEdit)+T*50;//ʱ������д�����
	m_PrimeStr.Format("%.1lf",S);

	//·��
	if(V>=3.1 && V<=4.1)  {m_TrafficStr="ӵ��";speed=60;}
	else				  {m_TrafficStr="����";speed=15;}

	UpdateData(FALSE);
}
/***************************************************************************************************************************************/
void CMyDlg::OnCarMenu() 
{
	FILE *rp;
	if((rp=fopen("CarA.txt","r"))==NULL) //��û��С����Ϣ¼�루��δ��Ѱ·����ԣ�	
	{
		MessageBox("����С����Ϣ!","����",MB_OK|MB_ICONSTOP);
		return;
	}
	else
	{
		fclose(rp);
		CCarDlg dlg;
		dlg.DoModal();
	}
}
/***************************************************************************************************************************************/
void CMyDlg::OnLawMenu() 
{
	CLawDlg dlg;
	dlg.DoModal();	
}
/***************************************************************************************************************************************/
BOOL CheckCool(ROUTE *left) //����Ƿ���Ҫ��ʣ�����������˻�
{
	while(left->next != NULL) //�������
	{
		if(oncrowd == FALSE && CheckTraffic(left->id,left->next->id) == FALSE) //��·�ζ³�
			return TRUE;
		left=left->next;
	}
	return FALSE;
}
/***************************************************************************************************************************************/
BOOL CheckClient(int first,int second) //����first���second��֮���Ƿ����δ���͵��Ŀͻ����
{

	for(int i=0;node[first].aroundid[i]!=0;i++)
		if(node[node[first].aroundid[i]].choose == CLIENTFLAG 
		&& node[node[first].aroundid[i]].been == FALSE) //�����first�����Χ����δ���͵Ŀͻ����
		{
			for(int j=0;node[second].aroundid[j]!=0;j++)
				if(node[first].aroundid[i] == node[second].aroundid[j])
					return TRUE;
		}

	return FALSE;
}
/***************************************************************************************************************************************/
BOOL CheckOncrowd(int r) //���ͻ�������ڵ�·�Ƿ�ӵ��
{
	int s;
	for(int n=51;n<=71;n++)
	{
		if(node[n].choose == CLIENTFLAG || node[n].choose == STOREFLAG)
		{
			for(int i=0;(s=node[n].roadid[i])!=0;i++)
				if(s == r)
					return TRUE;
		}
	}
	return FALSE;
}

void CMyDlg::OnQueryMenu() 
{
	CQueryDlg dlg;
	dlg.DoModal();
}
