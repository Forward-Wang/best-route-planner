// HelpDlg2.cpp : implementation file
//

#include "stdafx.h"
#include "������������·���滮ģ��ϵͳ.h"
#include "HelpDlg2.h"
#include "HelpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHelpDlg2 dialog


CHelpDlg2::CHelpDlg2(CWnd* pParent /*=NULL*/)
	: CDialog(CHelpDlg2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHelpDlg2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CHelpDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHelpDlg2)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHelpDlg2, CDialog)
	//{{AFX_MSG_MAP(CHelpDlg2)
	ON_BN_CLICKED(IDC_LastButton, OnLastButton)
	ON_BN_CLICKED(IDC_EnterButton2, OnEnterButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHelpDlg2 message handlers

void CHelpDlg2::OnLastButton() //��һҳ��ť
{
	//�ر�ʹ��˵��2�Ի���
	OnOK();

	//����ʹ��˵��1�Ի���
	CHelpDlg dlg;
	dlg.DoModal();	
}

void CHelpDlg2::OnEnterButton2() //�رհ�ť 
{
	OnOK();	
}
