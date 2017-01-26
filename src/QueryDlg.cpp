// QueryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "������������·���滮ģ��ϵͳ.h"
#include "QueryDlg.h"
#include <winsock.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueryDlg dialog
CQueryDlg::CQueryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQueryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQueryDlg)
	routeEdit = _T("");
	//}}AFX_DATA_INIT
}


void CQueryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQueryDlg)
	DDX_Control(pDX, IDC_RecordList, recordList);
	DDX_Text(pDX, IDC_RouteEdit, routeEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQueryDlg, CDialog)
	//{{AFX_MSG_MAP(CQueryDlg)
	ON_NOTIFY(NM_CLICK, IDC_RecordList, OnClickRecordList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueryDlg message handlers

BOOL CQueryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
		// �б���ͼ�ؼ��������
	recordList.SetExtendedStyle(LVS_EX_FLATSB	//��ƽ�����ʾ������
	|LVS_EX_FULLROWSELECT							//��������ѡ��
	|LVS_EX_HEADERDRAGDROP							//���������϶�
	|LVS_EX_ONECLICKACTIVATE						//����ѡ����
	|LVS_EX_GRIDLINES);								//����������
	
	//���ñ�ͷ
	recordList.InsertColumn(1,"��¼��",LVCFMT_CENTER,72,1); //����������
	recordList.InsertColumn(2,"�ֿ�ص�",LVCFMT_CENTER,160,2); //�����Ա���
	recordList.InsertColumn(3,"��ַ��",LVCFMT_CENTER,72,3); //����������
	recordList.InsertColumn(4,"������",LVCFMT_CENTER,72,4); //����Ӣ����
	recordList.InsertColumn(5,"�ͼ�",LVCFMT_CENTER,72,5); //����ƽ����
	recordList.InsertColumn(6,"�������",LVCFMT_CENTER,90,6); //����ƽ����
	recordList.InsertColumn(7,"�ܳɱ�",LVCFMT_CENTER,120,7); //����ƽ����
	recordList.InsertColumn(8,"�����",LVCFMT_CENTER,120,8); //����ƽ����
	recordList.InsertColumn(9,"Ԥ��ʱ��",LVCFMT_CENTER,120,9); //����ƽ����


	//�������ݿ�����
	ReadData();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQueryDlg::ReadData()
{

}

//ѡ���б����
void CQueryDlg::OnClickRecordList(NMHDR* pNMHDR, LRESULT* pResult) 
{

}
