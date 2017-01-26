// ������������·���滮ģ��ϵͳDlg.h : header file
//

#if !defined(AFX_DLG_H__BCD3097B_3692_47A4_9320_BEB211C62255__INCLUDED_)
#define AFX_DLG_H__BCD3097B_3692_47A4_9320_BEB211C62255__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMyDlg dialog

class CMyDlg : public CDialog
{
// Construction
public:
	CMyDlg(CWnd* pParent = NULL);	// standard constructor
	void WriteData();
//��������
	CMenu m_Menu;//�����˵�
	CPen pen1;//·����ɫ
	CPen pen2;//��ַ����
	CPen pen3;//��ԭǳ��

//�Զ��庯��
	void   DarkBlue(int x,int y);//��������
	void   LightBlue(int x,int y);//��ǳ����
	void   AllBlue();//����������
	void   StoreFlag();//�ֿ���ͼ��
	void   ClientFlag();//�ͻ���ͼ��
	void   BeenFlag();//�카ͼ��
	void   ShowBeen();//ɸѡ�Ѿ������Ŀͻ�
	void   CutScreen();//��ͼ
	void   PutScreen();//��ʾ��ͼ
	void   ChangeAnytime(int first,int second);//�ٶ���·����ʱ����
	void   DrawLine();//���н�����
	void   Map();//��ͼͼ��
	void   Picture(int x1,int y1,int x2,int y2);//��·�ػ�
	void   OnPicture(int x1,int y1,int x2,int y2);//ѡ�е�·
	void   TrafficFlag(int m);//��֮ǰ�����ӵ�µĵ�·���ػ�
	void   Statistic();//��������̡��ܳɱ���ʱ�䡢�ٶȡ�·��
	BOOL   JoinTable(int r);//����û�˫���ĵ�·�Ƿ�Ϸ�
// Dialog Data
	//{{AFX_DATA(CMyDlg)
	enum { IDD = IDD_MY_DIALOG };
	CString	m_DistanceStr;
	int		m_MinRadio;
	CString	m_PrimeStr;
	CString	m_SpeedStr;
	CString	m_TrafficStr;
	CString	m_WholeStr;
	CString	m_FuelEdit;
	CString	m_NeedEdit;
	CString	m_OtherEdit;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnOkButton();
	afx_msg void OnStopButton();
	afx_msg void OnReplayButton();
	afx_msg void OnResetButton();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnClientMenu();
	afx_msg void OnStoreMenu();
	afx_msg void OnSmoothMenu();
	afx_msg void OnCrowdMenu();
	afx_msg void OnTestButton();
	afx_msg void OnHelpMenu();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnCarMenu();
	afx_msg void OnLawMenu();
	afx_msg void OnQueryMenu();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLG_H__BCD3097B_3692_47A4_9320_BEB211C62255__INCLUDED_)
