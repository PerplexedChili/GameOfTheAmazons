// AmazonsView.cpp : implementation of the CAmazonsView class
//

#include "stdafx.h"
#include "AmazonsApp.h"
#include "math.h"
#include "MainFrm.h"

#include "AmazonsDoc.h"
#include "AmazonsView.h"
#include "AmazonsAI.h"
#include "GlobalFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STATE_UNSELECT 0	//�ȴ�ѡ������
#define STATE_SELECTED 1	//�ȴ����ӣ���ȡ������
#define STATE_MOVED 2		//�ȴ��ż�
#define STATE_SHOOTED 3		//�����

#define GAME_STATE_NOT_START 0
#define GAME_STATE_PLAYING 1
#define GAME_STATE_SUSPEND 2
#define GAME_STATE_OVER 3

#define  TIMER_INTERVAL 200//UIʱ�Ӽ������λms

#define TEMP_FILE_NAME _T("$Amazons_temp.amz")

//����AI
CAmazonsAIInterface *g_pAI = new CAmazonsAI;

/////////////////////////
//�߳�ͨ��
SPosition g_posSelected, g_posMoveTo, g_posShootAt;
CAmazonsDoc* g_pDoc;
CEvent g_eventContinue(FALSE, FALSE);
bool g_bPlayer1Turn = false;
/////////////////////////////////////////////////////////////////////////////
// CAmazonsView

IMPLEMENT_DYNCREATE(CAmazonsView, CView)

BEGIN_MESSAGE_MAP(CAmazonsView, CView)
	//{{AFX_MSG_MAP(CAmazonsView)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_FILE_OPEN, OnCmdFileOpen)
	ON_COMMAND(ID_SETTINGS, OnCmdSettings)
	ON_COMMAND(ID_NEW_GAME, OnCmdNewGame)
	ON_WM_TIMER()
	ON_COMMAND(ID_GAME_START, OnCmdGameStart)
	ON_UPDATE_COMMAND_UI(ID_GAME_START, OnUpdateGameStart)
	ON_COMMAND(ID_PASS, OnCmdPass)
	ON_UPDATE_COMMAND_UI(ID_PASS, OnUpdatePass)
	ON_COMMAND(ID_BACKWARD_ONE, OnCmdBackwardOne)
	ON_UPDATE_COMMAND_UI(ID_BACKWARD_ONE, OnUpdateBackwardOne)
	ON_COMMAND(ID_BACKWARD_TWO, OnCmdBackwardTwo)
	ON_UPDATE_COMMAND_UI(ID_BACKWARD_TWO, OnUpdateBackwardTwo)
	ON_COMMAND(ID_FORWARD_ONE, OnCmdForwardOne)
	ON_UPDATE_COMMAND_UI(ID_FORWARD_ONE, OnUpdateForwardOne)
	ON_COMMAND(ID_FORWARD_TWO, OnCmdForwardTwo)
	ON_UPDATE_COMMAND_UI(ID_FORWARD_TWO, OnUpdateForwardTwo)
	ON_COMMAND(ID_SHOW_HISTORY_DLG, OnCmdShowHistoryDlg)
	ON_UPDATE_COMMAND_UI(ID_SHOW_HISTORY_DLG, OnUpdateShowHistoryDlg)
	ON_COMMAND(ID_GAMESUSPEND, OnCmdGameSuspend)
	ON_UPDATE_COMMAND_UI(ID_GAMESUSPEND, OnUpdateGamesuspend)
	ON_COMMAND(ID_MARK_THE_LATEST_MOVE, OnCmdMarkCurMove)
	ON_COMMAND(ID_COPY_MOVES_TO_CLIPBOARD, OnCmdCopyMovesToClipboard)
	ON_COMMAND(ID_SUGGEST_A_MOVE, OnCmdSuggestAMove)
	ON_UPDATE_COMMAND_UI(ID_SUGGEST_A_MOVE, OnUpdateSuggestAMove)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_STOP_COMPUTING, OnCmdStopComputing)
	ON_UPDATE_COMMAND_UI(ID_STOP_COMPUTING, OnUpdateStopComputing)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_MESSAGE(WM_AI_COMPUTING_FINISHED, OnAIComputingFinished)
	ON_MESSAGE(WM_NETWORK_CONNECTED_TO_SERVER, OnNetworkConnectedToServer)
	ON_MESSAGE(WM_NETWORK_GET_PLAYER_LIST, OnNetworkGetPlayerList)
	ON_MESSAGE(WM_NETWORK_CONNECTED_TO_OPPONENT, OnNetworkConnectedToOpponent)
	ON_MESSAGE(WM_NETWORK_DISCONNECT_TO_OPPONENT, OnNetworkDisConnectToOpponent)
	ON_MESSAGE(WM_NETWORK_MOVE, OnNetworkMove)
	ON_MESSAGE(WM_NETWORK_LOG, OnNetworkLog)
	ON_MESSAGE(WM_NETWORK_GAME_CONTROL, OnNetworkGameControl)
	ON_WM_CREATE()
	ON_COMMAND(ID_NETWORK, &CAmazonsView::OnCmdNetwork)
	ON_COMMAND(ID_FILE_SAVE, &CAmazonsView::OnCmdFileSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAmazonsView construction/destruction

CAmazonsView::CAmazonsView()
{
	// TODO: add construction code here
	//��ͼ�ñ���
	m_ptTopLeft1 = CPoint(20, 20);
	m_ptTopLeft2 = m_ptTopLeft1 + CPoint(30, 30);
	m_nRightMinWidth = 270;
	m_clrDark = RGB(207, 140, 72);
	m_clrLight = RGB(253, 206, 162);
	m_clrArrow = RGB(128, 0, 0);
	m_clrViewBg = RGB(226, 224, 226);
	m_clrPlateBg = RGB(255, 255, 255);
	///////
	ResetVariables();
	m_pDlgHistory = new CHistoryDlg(this);
	m_pDlgHistory->Create();//CDialog(CHistoryDlg::IDD, pParent)
	m_networkPlayer.m_pView = this;
	m_bThreadStart = false;
	m_bHistoryDlgOpened = false;
	m_pThread = NULL;
	m_bSetTimer = false;
	m_bHasInitDCAndBitmap = false;

	m_strMoves = "Moves: ";
	m_strDepth = "Depth: ";
	m_strNodes = "Nodes: ";

}
void CAmazonsView::ResetVariables()
{
	m_bMarkLastAction = false;
	//
	m_nGameState = GAME_STATE_NOT_START;
	m_bComputerMoving = false;
	m_bComputerThinking = false;
	//
	m_nCurState = STATE_UNSELECT;
	m_posSelected = m_posMoveTo = m_posShootAt = SPosition(-1, -1);

	m_strPlayer1TotalTime = _T("00:00:00");
	m_strPlayer2TotalTime = _T("00:00:00");
	m_strPlayer1Timer = _T("00:00:00");
	m_strPlayer2Timer = _T("00:00:00");
	m_strTotalTimer = _T("00:00:00");
	m_strGameState = _T("�ȴ���ʼ");
	m_strDepth.Format(_T("Depth:"));
	m_strNodes.Format(_T("Nodes:"));
	m_strMoves.Format(_T("Moves:"));
	m_nPlayerTimer = 0;
	m_nTotalTimer = 0;
}
CAmazonsView::~CAmazonsView()
{	
}

CAsyncSocket g_socket;

int CAmazonsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	if (!m_bHasInitDCAndBitmap)
	{
		CDC *pDC = GetDC();
		m_clientMemDC.CreateCompatibleDC(pDC);
		m_bitmapMemDC.CreateCompatibleDC(pDC);
		m_bmClient.CreateCompatibleBitmap(pDC, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		m_clientMemDC.SelectObject(m_bmClient);
		m_clientMemDC.SetBkMode(TRANSPARENT);

		m_bmBlackPiece.LoadBitmap(IDB_BLACK);
		m_bmRedPiece.LoadBitmap(IDB_RED);
		m_bmSelectMark.LoadBitmap(IDB_SELECT);

		ReleaseDC(pDC);
		//pDC->DeleteDC();
		m_bHasInitDCAndBitmap = true;
	}
	return 0;
}

void CAmazonsView::OnDestroy()
{
	CView::OnDestroy();
	::DeleteFile(TEMP_FILE_NAME);
	m_clientMemDC.DeleteDC();
	m_bitmapMemDC.DeleteDC();
	m_networkPlayer.Stop();

	delete g_pAI;
	delete m_pDlgHistory;
	delete m_pThread;

	// TODO: Add your message handler code here
}

void CAmazonsView::OnInitialUpdate()
{
	if (!m_bThreadStart)
	{
		m_pThread = AfxBeginThread(Thread_AICompute, GetSafeHwnd(), THREAD_PRIORITY_ABOVE_NORMAL);
		m_bThreadStart = true;
	}
	if (!m_bSetTimer){
		SetTimer(1, TIMER_INTERVAL, NULL);
		m_bSetTimer = true;
	}	g_pDoc = GetDocument();
	
	CView::OnInitialUpdate();
}

/////////////////////////////////////////////////////////////////////////////
// CAmazonsView drawing

void CAmazonsView::OnDraw(CDC* pDC)
{
	int plate[10][10];//��ȡplate���ݽ���չʾ��ֻ��
	CAmazonsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CAmazonsGameControllerInterface *pAmazons = pDoc->pAmazons;
	pAmazons->GetPlate(plate);

	// TODO: add draw code for native data here
	double dRatioX, dRatioY;
	int n;

	CRect rectClient;

	//��������
	CPoint ptCellTopLeft(0, 0);
	CBrush bgBrush(m_clrPlateBg);
	CBrush brushDark(m_clrDark);
	CBrush brushLigth(m_clrLight);
	CBrush brushArrow(m_clrArrow);
	int i, j;
	CString str;

	//�������
	int nMargin = 15;
	CPoint ptTemp;
	CFont font, *pOldFont;
	font.CreatePointFont(100, _T("΢���ź�"));
	///////////
	//
	//ˢ����
	GetClientRect(rectClient);
	m_clientMemDC.SelectObject(bgBrush);
	m_clientMemDC.FillSolidRect(rectClient, m_clrViewBg);
	//
	//ȷ�����̵Ŀ�ȣ�Ҳ����С���ӵĿ��
	n = (m_ptTopLeft2.x - m_ptTopLeft1.x)*2;
	dRatioX = ((double)rectClient.Width() - 2 * m_ptTopLeft1.x - m_nRightMinWidth - n) / 400;
	dRatioY = ((double)rectClient.Height() - 2 * m_ptTopLeft1.y - n) / 400;
	if (dRatioX > dRatioY)	dRatioX = dRatioY;
	m_nCellWidth = (int)(40 * dRatioX);//�����������ڣ�������ұߺ����������������ߺ��ϱߵĲ�һ��
	if (m_nCellWidth < 10) m_nCellWidth = 10;
	//
	//��������������򣬼����Ӻ����һȦ����һ��ľ�������
	//�ұ�������ʾ����
	m_rectLeft = CRect(m_ptTopLeft1, CSize(m_nCellWidth * 10 + n, m_nCellWidth * 10 + n));
	m_rectRight = CRect(CPoint(m_ptTopLeft1.x + m_rectLeft.Width(), m_ptTopLeft1.y),
		CSize(rectClient.Width() - m_ptTopLeft1.x * 2 - m_rectLeft.Width(), m_rectLeft.Width()));
	m_clientMemDC.SelectObject(&bgBrush);
	m_clientMemDC.Rectangle(m_rectLeft);
	//
	//����ұ�����İ�Χ�򣬵�����
	/*m_clientMemDC.Rectangle(CRect(m_ptTopLeft2, CSize(m_nCellWidth * 10, m_nCellWidth * 10)));
	m_clientMemDC.SelectStockObject(NULL_BRUSH);
	m_clientMemDC.Rectangle(m_rectLeft);
	m_clientMemDC.Rectangle(m_rectRight);*/
	//
	///������
	for (i = 0; i < 10; i++)//д����
	{
		str.Format(_T("%d"), i);
		m_clientMemDC.TextOut(int(m_ptTopLeft2.x + (i + 0.5)*m_nCellWidth), m_ptTopLeft2.y - 21, str);//����
		m_clientMemDC.TextOut(int(m_ptTopLeft2.x + (i + 0.5)*m_nCellWidth), m_ptTopLeft2.y + 7 + 10 * m_nCellWidth, str);//����
		m_clientMemDC.TextOut(m_ptTopLeft2.x - 18, int(m_ptTopLeft2.y + (i + 0.3)*m_nCellWidth), str);//����
		m_clientMemDC.TextOut(m_ptTopLeft2.x + 10 + 10 * m_nCellWidth, int(m_ptTopLeft2.y + (i + 0.3)*m_nCellWidth), str);//����
	}
	//11������
	CPoint ptLineBegin = m_ptTopLeft2;
	CPoint ptLineEnd = m_ptTopLeft2 + CPoint(m_nCellWidth*10, 0);
	for (i = 0; i < 11; i++)
	{
		m_clientMemDC.MoveTo(ptLineBegin);
		m_clientMemDC.LineTo(ptLineEnd);
		ptLineBegin.y += m_nCellWidth;
		ptLineEnd.y += m_nCellWidth;
	}
	//11������
	ptLineBegin = m_ptTopLeft2;
	ptLineEnd = m_ptTopLeft2 + CPoint(0, m_nCellWidth * 10);
	for (i = 0; i < 11; i++)
	{
		m_clientMemDC.MoveTo(ptLineBegin);
		m_clientMemDC.LineTo(ptLineEnd);
		ptLineBegin.x += m_nCellWidth;
		ptLineEnd.x += m_nCellWidth;
	}
	//
	BITMAP pieceBmpInfo, slectMarkBmpInfo;
	m_bmRedPiece.GetBitmap(&pieceBmpInfo);
	m_bmSelectMark.GetBitmap(&slectMarkBmpInfo);
	//
	//���С����
	int nCellValue = 0;
	for (i = 0; i < 10; i++)
	{
		for (j = 0; j < 10; j++)
		{
			nCellValue = plate[i][j];
			ptCellTopLeft = CPoint(j*m_nCellWidth + 1, i*m_nCellWidth + 1) + m_ptTopLeft2;

			//�ñ���������
			if ((i + j) % 2 == 0)//ǳɫ����
				m_clientMemDC.SelectObject(&brushLigth);
			else//��ɫ����
				m_clientMemDC.SelectObject(&brushDark);
			m_clientMemDC.SelectStockObject(NULL_PEN);
			m_clientMemDC.Rectangle(CRect(ptCellTopLeft, CSize(m_nCellWidth, m_nCellWidth)));

			//����ͼ��
			if (pAmazons->IsArrow(nCellValue))//�ż�ͼ��
			{
				m_clientMemDC.SelectObject(&brushArrow);
				m_clientMemDC.Rectangle(CRect(ptCellTopLeft, CSize(m_nCellWidth, m_nCellWidth)));
			}
			else if (pAmazons->IsPlayer1(nCellValue))//����ͼ��(���1�úڵ�)
			{
				m_bitmapMemDC.SelectObject(&m_bmBlackPiece);
				TransparentBlt(m_clientMemDC.m_hDC, ptCellTopLeft.x, ptCellTopLeft.y, m_nCellWidth, m_nCellWidth,
					m_bitmapMemDC.m_hDC, 0, 0, pieceBmpInfo.bmWidth, pieceBmpInfo.bmHeight, RGB(255, 255, 255));
				//TRACE("(%d,%d)\n",i,j);
			}
			else if (pAmazons->IsPlayer2(nCellValue))//����ͼ��(���2�ú��)
			{
				m_bitmapMemDC.SelectObject(&m_bmRedPiece);
				TransparentBlt(m_clientMemDC.m_hDC, ptCellTopLeft.x, ptCellTopLeft.y, m_nCellWidth, m_nCellWidth,
					m_bitmapMemDC.m_hDC, 0, 0, pieceBmpInfo.bmWidth, pieceBmpInfo.bmHeight, RGB(255, 255, 255));
				//TRACE("(%d,%d)\n",i,j);
			}

			//��ʾѡ�б��
			bool showMark = false;
			if (m_nCurState == STATE_SELECTED)
				showMark = (i == m_posSelected.i && j == m_posSelected.j);
			else if (m_nCurState == STATE_MOVED)
				showMark = (m_posMoveTo.i == i && m_posMoveTo.j == j);
			if (showMark)
			{
				m_bitmapMemDC.SelectObject(&m_bmSelectMark);
				TransparentBlt(m_clientMemDC.m_hDC, ptCellTopLeft.x, ptCellTopLeft.y, m_nCellWidth, m_nCellWidth,
					m_bitmapMemDC.m_hDC, 0, 0, pieceBmpInfo.bmWidth, pieceBmpInfo.bmHeight, RGB(255, 255, 255));
			}
		}
	}
	if (m_bMarkTheAction)
	{
		MarkTheAction(m_posSelected, m_posMoveTo, false);
		MarkTheAction(m_posMoveTo, m_posShootAt, true);
	}

	//�����ұ�
	//�ұ�(Ӧ����Ϊ�̶��߶ȣ������ػ�ʱ���Ĳ��ֲ�����ȫ�ػ�,70+120+55+100=345)
	m_rectRight.bottom = m_rectRight.top + 345;
	pOldFont = m_clientMemDC.SelectObject(&font);
	m_clientMemDC.SelectStockObject(NULL_BRUSH);
	m_clientMemDC.SelectStockObject(BLACK_PEN);
	//m_clientMemDC.Rectangle(m_rectRight);
	////���ĸ�����
	ptTemp = m_rectRight.TopLeft();
	ptTemp.x = ptTemp.x + nMargin;
	ptTemp.y = ptTemp.y + nMargin;
	/////////////////////��һ���֣�������Ϣ
	m_clientMemDC.MoveTo(ptTemp.x, ptTemp.y); m_clientMemDC.LineTo(m_rectRight.right, ptTemp.y);
	m_clientMemDC.MoveTo(ptTemp.x, ptTemp.y - 1); m_clientMemDC.LineTo(m_rectRight.right, ptTemp.y - 1);
	//
	//���ϡ����1--��/�췽--����/��   ����
	//���¡����1--��/�췽--����/�� 
	CString drawText = _T("���ϡ����1 - ");
	drawText += pAmazons->m_bPlayer1Black ? _T("�ڷ� - ") : _T("�췽 - ");
	drawText += GetPlayerName(pAmazons->m_nPlayer1Type);
	m_clientMemDC.TextOut(ptTemp.x + nMargin, ptTemp.y + 15, drawText);
	//
	drawText = _T("���¡����2 - ");
	drawText += pAmazons->m_bPlayer1Black ? _T("�췽 - ") : _T("�ڷ� - ");
	drawText += GetPlayerName(pAmazons->m_nPlayer2Type);
	m_clientMemDC.TextOut(ptTemp.x + nMargin, ptTemp.y + 35, drawText);
	//
	drawText = pAmazons->m_bPlayer1First ? GetPlayer1Color() : GetPlayer2Color();
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 200, ptTemp.y + 24 , drawText + _T("��"));

	/////////////////////�ڶ����֣���ʱ
	ptTemp.y = ptTemp.y + 70;
	m_clientMemDC.MoveTo(ptTemp.x, ptTemp.y); m_clientMemDC.LineTo(m_rectRight.right, ptTemp.y);
	m_clientMemDC.MoveTo(ptTemp.x, ptTemp.y - 1); m_clientMemDC.LineTo(m_rectRight.right, ptTemp.y - 1);
	//���Ʊ��
	m_clientMemDC.MoveTo(ptTemp.x + nMargin, ptTemp.y + 20); m_clientMemDC.LineTo(ptTemp.x + nMargin + 240, ptTemp.y + 20);//��0
	m_clientMemDC.MoveTo(ptTemp.x + nMargin, ptTemp.y + 45); m_clientMemDC.LineTo(ptTemp.x + nMargin + 240, ptTemp.y + 45);//��1
	m_clientMemDC.MoveTo(ptTemp.x + nMargin, ptTemp.y + 70); m_clientMemDC.LineTo(ptTemp.x + nMargin + 240, ptTemp.y + 70);//��2
	m_clientMemDC.MoveTo(ptTemp.x + nMargin, ptTemp.y + 95); m_clientMemDC.LineTo(ptTemp.x + nMargin + 240, ptTemp.y + 95);//��3
	m_clientMemDC.MoveTo(ptTemp.x + nMargin, ptTemp.y + 20);    m_clientMemDC.LineTo(ptTemp.x + nMargin, ptTemp.y + 95);	//��0
	m_clientMemDC.MoveTo(ptTemp.x + nMargin + 70, ptTemp.y + 20); m_clientMemDC.LineTo(ptTemp.x + nMargin + 70, ptTemp.y + 95);	//��1
	m_clientMemDC.MoveTo(ptTemp.x + nMargin + 155, ptTemp.y + 20); m_clientMemDC.LineTo(ptTemp.x + nMargin + 155, ptTemp.y + 95);//��2
	m_clientMemDC.MoveTo(ptTemp.x + nMargin + 239, ptTemp.y + 20); m_clientMemDC.LineTo(ptTemp.x + nMargin + 239, ptTemp.y + 95);//��3(��1pxϵͳ���)
	//m_clientMemDC.TextOut(ptTemp.x + nMargin + 10, ptTemp.y + 22, m_strTotalTimer);	//��1��1
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 15, ptTemp.y + 48, "����ʱ");			//��2��2
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 10, ptTemp.y + 73, "������ʱ");		//��3��3
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 30 + 70, ptTemp.y + 23, pAmazons->m_bPlayer1Black ? _T("�ڷ�") : _T("�췽"));		//��2��1
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 15 + 70, ptTemp.y + 48, m_strPlayer1TotalTime);									//��2��2
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 15 + 70, ptTemp.y + 73, m_strPlayer1Timer);										//��2��3
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 30 + 155, ptTemp.y + 23, pAmazons->m_bPlayer1Black ? _T("�췽") : _T("�ڷ�"));	//��3��1
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 15 + 155, ptTemp.y + 48, m_strPlayer2TotalTime);									//��3��2
	m_clientMemDC.TextOut(ptTemp.x + nMargin + 15 + 155, ptTemp.y + 73, m_strPlayer2Timer);										//��3��3
	//m_clientMemDC.Rectangle(m_rectRight2);
	/////////////////////�������֣�˭����
	ptTemp.y = ptTemp.y + 120;
	m_clientMemDC.MoveTo(ptTemp.x, ptTemp.y); m_clientMemDC.LineTo(m_rectRight.right, ptTemp.y);
	m_clientMemDC.MoveTo(ptTemp.x, ptTemp.y - 1); m_clientMemDC.LineTo(m_rectRight.right, ptTemp.y - 1);
	m_clientMemDC.SetTextColor(RGB(255, 0, 128));
	m_clientMemDC.TextOut(ptTemp.x + nMargin, ptTemp.y + 15, m_strGameState);
	m_clientMemDC.SetTextColor(RGB(0, 0, 0));
	/////////////////////���Ĳ��֣�״̬��ʾ
	ptTemp.y = ptTemp.y + 55;
	m_clientMemDC.MoveTo(ptTemp.x, ptTemp.y); m_clientMemDC.LineTo(m_rectRight.right, ptTemp.y);
	m_clientMemDC.MoveTo(ptTemp.x, ptTemp.y - 1); m_clientMemDC.LineTo(m_rectRight.right, ptTemp.y - 1);
	//
	m_clientMemDC.TextOut(ptTemp.x + nMargin, ptTemp.y + 15, m_strMoves);
	m_clientMemDC.TextOut(ptTemp.x + nMargin, ptTemp.y + 40, m_strDepth);
	m_clientMemDC.TextOut(ptTemp.x + nMargin, ptTemp.y + 65, m_strNodes);
	///////////////
	m_clientMemDC.SelectObject(pOldFont);
	m_clientMemDC.SelectStockObject(NULL_BRUSH);
	m_clientMemDC.SelectStockObject(BLACK_PEN);

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &m_clientMemDC, 0, 0, SRCCOPY);

	//pDC->GetClipBox(rectClient);//�������������
	//if (rectClient != m_rectRight)
		//m_bThereIsMark = false;//�����ػ����̲�����ǻ���ʧ

}

void CAmazonsView::MarkTheAction(SPosition posActBgn, SPosition posActEnd, bool bShoot)
{
	if (posActBgn.i == -1 || posActBgn.j == -1) return;
	if (posActEnd.i == -1 || posActEnd.j == -1) return;

	int i, j;
	CPoint ptBgn, ptEnd;
	CPen penMove(PS_SOLID, 2, RGB(0, 255, 0));
	CPen penShoot(PS_SOLID, 2, RGB(0, 20, 255));
	CBrush brushMove(RGB(0, 255, 0));

	if (bShoot){
		m_clientMemDC.SelectObject(&penShoot);
	}
	else{
		m_clientMemDC.SelectObject(&penMove);
		m_clientMemDC.SelectObject(&brushMove);
	}

	i = posActBgn.i; j = posActBgn.j;
	ptBgn = CPoint(int((j + 0.5)*m_nCellWidth) + 1, int((i + 0.5)*m_nCellWidth) + 1) + m_ptTopLeft2;
	i = posActEnd.i; j = posActEnd.j;
	ptEnd = CPoint(int((j + 0.5)*m_nCellWidth) + 1, int((i + 0.5)*m_nCellWidth) + 1) + m_ptTopLeft2;
	m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	if (!bShoot)
		m_clientMemDC.Ellipse(ptBgn.x - 5, ptBgn.y - 5, ptBgn.x + 5, ptBgn.y + 5);
	//�����ͷ(���¾�ָ��ͷ����)(��ͷһ���������)
	ptBgn = ptEnd;
	if (j == posActBgn.j&&i < posActBgn.i){//��ֱ����
		ptEnd = CPoint(ptBgn.x - 4, ptBgn.y + 8); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
		ptEnd = CPoint(ptBgn.x + 4, ptBgn.y + 8); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	}
	if (j == posActBgn.j&&i > posActBgn.i){//��ֱ����
		ptEnd = CPoint(ptBgn.x - 4, ptBgn.y - 8); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
		ptEnd = CPoint(ptBgn.x + 4, ptBgn.y - 8); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	}
	if (i == posActBgn.i&&j < posActBgn.j){//ˮƽ����
		ptEnd = CPoint(ptBgn.x + 8, ptBgn.y - 4); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
		ptEnd = CPoint(ptBgn.x + 8, ptBgn.y + 4); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	}
	if (i == posActBgn.i&&j > posActBgn.j){//ˮƽ����
		ptEnd = CPoint(ptBgn.x - 8, ptBgn.y - 4); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
		ptEnd = CPoint(ptBgn.x - 8, ptBgn.y + 4); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	}
	if (i<posActBgn.i&&j < posActBgn.j){//��б����
		ptEnd = CPoint(ptBgn.x + 2, ptBgn.y + 8); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
		ptEnd = CPoint(ptBgn.x + 8, ptBgn.y + 2); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	}
	if (i > posActBgn.i&&j>posActBgn.j){//��б����
		ptEnd = CPoint(ptBgn.x - 8, ptBgn.y - 2); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
		ptEnd = CPoint(ptBgn.x - 2, ptBgn.y - 8); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	}
	if (i<posActBgn.i&&j>posActBgn.j){//��б����
		ptEnd = CPoint(ptBgn.x - 8, ptBgn.y + 2); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
		ptEnd = CPoint(ptBgn.x - 2, ptBgn.y + 8); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	}
	if (i > posActBgn.i&&j < posActBgn.j){//��б����
		ptEnd = CPoint(ptBgn.x + 2, ptBgn.y - 8); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
		ptEnd = CPoint(ptBgn.x + 8, ptBgn.y - 2); m_clientMemDC.MoveTo(ptBgn); m_clientMemDC.LineTo(ptEnd);
	}

	m_clientMemDC.SelectStockObject(BLACK_PEN);
	m_clientMemDC.SelectStockObject(NULL_BRUSH);
}

/////////////////////////////////////////////////////////////////////////////
// CAmazonsView printing

BOOL CAmazonsView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CAmazonsView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CAmazonsView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CAmazonsView diagnostics

#ifdef _DEBUG
void CAmazonsView::AssertValid() const
{
	CView::AssertValid();
}

void CAmazonsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAmazonsDoc* CAmazonsView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAmazonsDoc)));
	return (CAmazonsDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAmazonsView message handlers

BOOL CAmazonsView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return true;
	//return CView::OnEraseBkgnd(pDC);
}

void CAmazonsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_nGameState != GAME_STATE_PLAYING || m_bComputerThinking)
		return;

	//TRACE("point(%d,%d)\n",point.x,point.y);
	//return 0L;
	CString str;
	CPoint pointUnit(0, 0);
	CAmazonsDoc* pDoc = GetDocument();
	CAmazonsGameControllerInterface* pAmazons = pDoc->pAmazons;

	//����Ƿ���������
	point = point - m_ptTopLeft2;
	if (point.x < 0 || point.x > m_nCellWidth * 10 || point.y < 0 || pointUnit.y > m_nCellWidth * 10)
		return;

	//������ĸ�������
	int i = point.y / m_nCellWidth;
	int j = point.x / m_nCellWidth;
	int cellValue = pAmazons->GetPlateCellValue(i, j);
	SPosition pos(i, j);
	//TRACE("Click:(%d,%d)\n", i, j);

	bool redraw = false;
	if (m_nCurState == STATE_UNSELECT)
	{
		if (IsCurPlayerPiece(pAmazons, cellValue))
		{
			//ѡ������
			m_posMoveTo.i = -1;
			m_posMoveTo.j = -1;
			m_posShootAt = m_posMoveTo;
			m_posSelected = pos;
			m_nCurState = STATE_SELECTED;
			redraw = true;
			//TRACE("Selected\n");
		}
	}
	else if (m_nCurState == STATE_SELECTED)
	{
		if (IsCurPlayerPiece(pAmazons, cellValue))
		{
			//ѡ����������
			m_posSelected = pos;
			m_nCurState = STATE_SELECTED;
			redraw = true;
			//TRACE("SelectedLoop\n");
		}
		else if (pAmazons->IsEmpty(cellValue))
		{
			//����
			if (pAmazons->CanMoveOrShoot(m_posSelected, pos))
			{
				m_posMoveTo = pos;
				pAmazons->Move(m_posSelected, pos);
				m_nCurState = STATE_MOVED;
				redraw = true;
			}
		}
	}
	else if (m_nCurState == STATE_MOVED)
	{
		if (m_posMoveTo.i == i && m_posMoveTo.j == j)
		{
			//ȡ������
			pAmazons->CancelMove(m_posSelected, pos);
			m_posMoveTo.i = -1;
			m_posMoveTo.j = -1;
			m_nCurState = STATE_SELECTED;
			redraw = true;
		}
		else if (pAmazons->IsEmpty(cellValue))
		{
			//�ż�
			if (pAmazons->CanMoveOrShoot(m_posMoveTo, pos))
			{
				m_posShootAt = pos;
				pAmazons->Shoot(pos);
				m_nCurState = STATE_UNSELECT;
				redraw = true;
				HumanMoveFinished();
			}
		}
	}

	if (redraw)
		InvalidateRect(NULL);

	CView::OnLButtonDown(nFlags, point);
}

void CAmazonsView::OnTimer(UINT nIDEvent)
{
	UpdateComputerMove();
	UpdateRightInfo();
	InvalidateRect(m_rectRight);
	CView::OnTimer(nIDEvent);
}

void CAmazonsView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	OnCmdGameStart();
	CView::OnRButtonDown(nFlags, point);
}

bool CAmazonsView::IsCurPlayerPiece(CAmazonsGameControllerInterface *pAmazons, int piece)
{
	//�Ƿ�����Ҷ�Ӧ������
	if (pAmazons->IsPlayer1Turn())
		return pAmazons->IsPlayer1(piece);
	else
		return pAmazons->IsPlayer2(piece);
}

void CAmazonsView::HumanMoveFinished()
{
	if (IsOpponentNetplayer())
		m_networkPlayer.SendMoveMsg(m_posSelected, m_posMoveTo, m_posShootAt);

	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	pAmazons->AddToHistory(m_posSelected, m_posMoveTo, m_posShootAt, m_nPlayerTimer);
	pAmazons->SaveTempFile(TEMP_FILE_NAME);
	UpdateHistoryDlg();
	OpponentTurn();
}

void CAmazonsView::StartAIComputing()
{
	g_eventContinue.SetEvent();//�����ź�
	m_bComputerThinking = true;
}

UINT CAmazonsView::Thread_AICompute(LPVOID pParam)
{
	TRACE("ComputingThreadStarted\n");
	HWND  hWnd = (HWND)pParam;
	int plate[10][10];
	SPosition pieces[9];
	while (1)
	{
		::WaitForSingleObject(g_eventContinue, INFINITE);
		g_pDoc->pAmazons->GetPlate(plate);
		g_pDoc->pAmazons->GetPieces(pieces);
		g_pAI->AICompute(plate, pieces, g_pDoc->pAmazons->IsPlayer1Turn(), g_posSelected, g_posMoveTo, g_posShootAt);
		::PostMessage(hWnd, WM_AI_COMPUTING_FINISHED, 0, 0);
	}
	return 0;
}

LRESULT CAmazonsView::OnAIComputingFinished(WPARAM wParam, LPARAM lParam)
{
	m_posSelected = m_posMoveTo = m_posShootAt = SPosition(-1, -1);
	m_bComputerMoving = true;
	m_nComputerMoveTimer = 0;
	return 0L;
}

void CAmazonsView::Select(SPosition posSelected)
{
	CAmazonsGameControllerInterface *pAI = GetDocument()->pAmazons;
	m_posSelected = posSelected;
	InvalidateRect(m_rectLeft);
}

void CAmazonsView::Move(SPosition posSelected, SPosition posMoveTo)
{
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	m_posSelected = posSelected;
	m_posMoveTo = posMoveTo;
	pAmazons->Move(posSelected, posMoveTo);
	InvalidateRect(m_rectLeft);
}

void CAmazonsView::Shoot(SPosition posShootAt)
{
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	m_posShootAt = posShootAt;
	pAmazons->Shoot(posShootAt);
	InvalidateRect(m_rectLeft);
}

bool CAmazonsView::IsGameOver()
{
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	int nWhoWin = pAmazons->IsGameOver();

	if (nWhoWin == 1)
	{
		m_strGameState = pAmazons->m_bPlayer1Black ? _T("�ڷ�Ӯ") : _T("�췽Ӯ");
		m_nGameState = GAME_STATE_OVER;
	}
	else if (nWhoWin == 2)
	{
		m_strGameState = pAmazons->m_bPlayer1Black ? _T("�췽Ӯ") : _T("�ڷ�Ӯ");
		m_nGameState = GAME_STATE_OVER;
	}
	else if (nWhoWin == 3)
	{
		m_strGameState = _T("ƽ��");
		m_nGameState = GAME_STATE_OVER;
	}

	return nWhoWin >= 1;
}

bool CAmazonsView::IsOpponentNetplayer()
{
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	if (pAmazons->IsPlayer1Turn())
		return pAmazons->m_nPlayer2Type == PLAYER_NETWORK;
	else
		return pAmazons->m_nPlayer1Type == PLAYER_NETWORK;
}

void CAmazonsView::OpponentTurn(bool isStartGame /*= false*/)
{
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;

	if (IsGameOver())
	{
		AfxMessageBox(m_strGameState, MB_OK);
		return;
	}

	if (pAmazons->IsPlayer1Turn())
	{
		if (pAmazons->m_nPlayer1Type == PLAYER_COMPUTER)
			StartAIComputing();
	}
	else
	{
		if (pAmazons->m_nPlayer2Type == PLAYER_COMPUTER)
			StartAIComputing();
	}
	m_nPlayerTimer = 0;
}

void CAmazonsView::OnCmdFileOpen()
{
	// TODO: Add your command handler code here
	CFileDialog dlg(true, _T("amz"), _T("*.amz"));
	if (dlg.DoModal() == IDOK)
	{
		CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
		pAmazons->LoadFromFile(dlg.GetPathName());
		ResetVariables();
		UpdateHistoryDlg();
		IsGameOver();
		InvalidateRect(NULL);
		CMainFrame *pMainFrame = (CMainFrame *)AfxGetMainWnd();
		pMainFrame->m_strDocName = dlg.GetFileName();
		pMainFrame->UpdateApplicationTitle();

		//��ȡ��ǰ���裬����mark
		CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
		SHistoryRecord record = pAmazonAi->GetCurHistoryMove();
		m_posSelected = record.posSelected;
		m_posMoveTo = record.posMoveTo;
		m_posShootAt = record.posShootAt;
	}
}

void CAmazonsView::OnCmdFileSave()
{
	CFileDialog dlg(false, _T("amz"), _T("*.amz"));
	if (dlg.DoModal() == IDOK)
	{
		CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
		pAmazons->SaveToFile(dlg.GetPathName());
	}
}

void CAmazonsView::OnCmdNewGame()//"����Ϸ"�˵������Ӧ��������Ҫ�����������ݣ��Կ�ʼ����Ϸ
{
	// TODO: Add your command handler code here
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	pAmazons->NewGame();
	ResetVariables();
	UpdateHistoryDlg();
	InvalidateRect(NULL);
}

void CAmazonsView::OnCmdSettings()
{
	// TODO: Add your command handler code here
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	m_dlgSettings.Init(pAmazons->m_nPlayer1Type,
		pAmazons->m_nPlayer2Type,
		pAmazons->m_bPlayer1Black, 
		pAmazons->m_bPlayer1First);
	if (m_dlgSettings.DoModal() == IDOK)
	{
		pAmazons->m_nPlayer1Type = m_dlgSettings.m_nPlayer1Type;
		pAmazons->m_nPlayer2Type = m_dlgSettings.m_nPlayer2Type;
		pAmazons->m_bPlayer1Black = m_dlgSettings.m_nPlayer1IsBlack==0 ? true : false;
		pAmazons->m_bPlayer1First = m_dlgSettings.m_nPlayer1IsFirst==0 ? true : false;
		OnCmdNewGame();
	}
}

void CAmazonsView::OnCmdShowHistoryDlg()
{
	// TODO: Add your command handler code here
	if (!m_bHistoryDlgOpened)
	{
		m_bHistoryDlgOpened = true;
		m_pDlgHistory->UpdateHistoryText(GetDocument()->pAmazons);
		m_pDlgHistory->ShowWindow(SW_SHOW);
		m_pDlgHistory->UpdateData(false);
	}
}

void CAmazonsView::OnCmdGameStart()
{  
	OnCommandGameStartWrap(TRUE);
}

void CAmazonsView::OnCommandGameStartWrap(bool sendMsg)
{
	if (m_nGameState == GAME_STATE_PLAYING) return;

	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	if (pAmazons->m_nPlayer1Type == PLAYER_NETWORK || pAmazons->m_nPlayer2Type == PLAYER_NETWORK)
	{
		if (!m_networkPlayer.HasOpponent())
		{
			OnCmdNetwork();
			return;
		}

		if (sendMsg)
			m_networkPlayer.SendGameControlMsg(NET_GAME_CONTROLL_START);
	}

	OpponentTurn(true);
	m_nGameState = GAME_STATE_PLAYING;
}

void CAmazonsView::OnCmdGameSuspend()
{
	// TODO: Add your command handler code here
	m_nGameState = GAME_STATE_SUSPEND;
}

void CAmazonsView::OnCmdPass()
{
	// TODO: Add your command handler code here
	StartAIComputing();
}

void CAmazonsView::OnCmdBackwardOne()
{
	// TODO: Add your command handler code here
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	if (pAmazonAi->CanBackward(1))
	{
		pAmazonAi->Backward(1);
		ForwardOrBackwardHistoryEnd();
	}
}

void CAmazonsView::OnCmdForwardOne()
{
	// TODO: Add your command handler code here
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	if (pAmazonAi->CanForward(1))
	{
		pAmazonAi->Forward(1);
		ForwardOrBackwardHistoryEnd();
	}
}

void CAmazonsView::OnCmdBackwardTwo()
{
	// TODO: Add your command handler code here
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	if (pAmazonAi->CanBackward(2))
	{
		pAmazonAi->Backward(2);
		ForwardOrBackwardHistoryEnd();
	}
	CAmazonsDoc* pDoc = GetDocument();
}

void CAmazonsView::OnCmdForwardTwo()
{
	// TODO: Add your command handler code here
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	if (pAmazonAi->CanForward(2))
	{
		pAmazonAi->Forward(2);
		ForwardOrBackwardHistoryEnd();
	}
}

void CAmazonsView::OnCmdMarkCurMove()
{

}

void CAmazonsView::OnCmdCopyMovesToClipboard()
{
	// TODO: Add your command handler code here
}

void CAmazonsView::OnCmdSuggestAMove()
{
	// TODO: Add your command handler code here
	CAmazonsDoc* pDoc = GetDocument();
	if (m_bComputerThinking) return;
	StartAIComputing();
}

void CAmazonsView::OnCmdStopComputing()
{
	// TODO: Add your command handler code here
	g_pAI->StopComputing();
}

void CAmazonsView::OnCmdNetwork()
{
	m_networkPlayer.m_nplayerCamp = 0;
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	if (pAmazonAi->m_nPlayer1Type == PLAYER_NETWORK)
		m_networkPlayer.m_nplayerCamp = 1;
	if (pAmazonAi->m_nPlayer2Type == PLAYER_NETWORK)
		m_networkPlayer.m_nplayerCamp = 2;

	CMainFrame *pMainFrame = (CMainFrame *)AfxGetApp()->GetMainWnd();
	m_dlgNetwork.m_pNetworkPlayer = &m_networkPlayer;
	m_dlgNetwork.m_nPort = pMainFrame->m_nPort;
	m_dlgNetwork.m_strPlayerName = pMainFrame->m_strNetWorkName;
	m_dlgNetwork.m_dwIp = GblRevertByByte(GblGetIPLong(pMainFrame->m_strIp));

	if (m_dlgNetwork.DoModal() == IDOK)
	{
		pMainFrame->m_nPort = m_dlgNetwork.m_nPort;
		pMainFrame->m_strIp = GblGetIPString(GblRevertByByte(m_dlgNetwork.m_dwIp));
		pMainFrame->m_strNetWorkName = m_dlgNetwork.m_strPlayerName;
	}
}

void CAmazonsView::OnUpdateShowHistoryDlg(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_bHistoryDlgOpened);
}

void CAmazonsView::OnUpdateBackwardOne(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	pCmdUI->Enable((!m_bComputerThinking) && pAmazonAi->CanBackward(1));
}

void CAmazonsView::OnUpdateForwardOne(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	pCmdUI->Enable((!m_bComputerThinking) && pAmazonAi->CanForward(1));
}

void CAmazonsView::OnUpdateBackwardTwo(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	pCmdUI->Enable((!m_bComputerThinking) && pAmazonAi->CanBackward(2));
}

void CAmazonsView::OnUpdateForwardTwo(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	pCmdUI->Enable((!m_bComputerThinking) && pAmazonAi->CanForward(2));
}

void CAmazonsView::OnUpdateSuggestAMove(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_bComputerThinking);
}

void CAmazonsView::OnUpdateGameStart(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_nGameState == GAME_STATE_NOT_START || m_nGameState == GAME_STATE_SUSPEND);
}

void CAmazonsView::OnUpdateGamesuspend(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_nGameState == GAME_STATE_PLAYING && !m_bComputerThinking);

}

void CAmazonsView::OnUpdatePass(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_nGameState == GAME_STATE_PLAYING && (!m_bComputerThinking));
}

void CAmazonsView::OnUpdateStopComputing(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bComputerThinking);
}

void CAmazonsView::UpdateHistoryDlg()
{
	if (!m_bHistoryDlgOpened) return;

	m_pDlgHistory->UpdateHistoryText(GetDocument()->pAmazons);
	m_pDlgHistory->UpdateData(FALSE);
	m_pDlgHistory->m_editHistory.SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

void CAmazonsView::OnHistoryDlgClosed()
{
	//������close�ˣ����ܴ���ֱ��������
	m_pDlgHistory->ShowWindow(SW_HIDE);
	m_bHistoryDlgOpened = false;
}

void CAmazonsView::UpdateComputerMove()
{
	//������������
	if (!m_bComputerMoving) return;

	m_nComputerMoveTimer += TIMER_INTERVAL;
	if (m_nCurState == STATE_UNSELECT)
	{
		Select(g_posSelected);
		m_nCurState = STATE_SELECTED;
	}
	if (m_nCurState == STATE_SELECTED && m_nComputerMoveTimer > 300)
	{
		Move(g_posSelected, g_posMoveTo);
		m_nCurState = STATE_MOVED;
		m_nComputerMoveTimer = 0;
	}
	if (m_nCurState == STATE_MOVED && m_nComputerMoveTimer > 300)
	{
		Shoot(g_posShootAt);
		m_nCurState = STATE_SHOOTED;
		m_nComputerMoveTimer = 0;
	}
	if (m_nCurState == STATE_SHOOTED && m_nComputerMoveTimer > 200)
	{
		m_nCurState = STATE_UNSELECT;
		m_bComputerMoving = false;
		m_bComputerThinking = false;
		ComputerMoveFinished();
	}

}
void CAmazonsView::UpdateRightInfo()
{
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;

	if (m_nGameState == GAME_STATE_NOT_START)
	{
		m_strGameState = _T("��������������ʼ��ť��������Ҽ�����ʼ��Ϸ");
	}
	else if (m_nGameState == GAME_STATE_PLAYING)
	{
		int playerColor = 0;//1Ϊ�ڣ�2Ϊ��
		int playerType = 0;

		if (pAmazons->IsPlayer1Turn())
		{
			playerColor = pAmazons->m_bPlayer1Black ? 1 : 2;
			playerType = pAmazons->m_nPlayer1Type;
		}
		else
		{
			playerColor = pAmazons->m_bPlayer1Black ? 2 : 1;
			playerType = pAmazons->m_nPlayer2Type;
		}

		m_strGameState = playerColor == 1 ? _T("�ڷ�") : _T("�췽");
		if      (playerType == PLAYER_HUMAN)    m_strGameState += _T("  ��������");
		else if (playerType == PLAYER_COMPUTER) m_strGameState += _T("  ��������");
		else if (playerType == PLAYER_NETWORK)  m_strGameState += _T("  �����������");

		if (m_bComputerThinking)
		{
			m_strDepth.Format(_T("Depth: %d"), g_pAI->m_nOutDepth);
			m_strNodes.Format(_T("Nodes: %d %d"), g_pAI->m_nOutNumOfSkippedNodes, g_pAI->m_nOutNumOfNodes);
			m_strMoves.Format(_T("Moves: %d,%d"), g_pAI->m_nOutNumOfMoves, g_pAI->m_nOutMaxValue);
		}

		UpdateTimeCounter();

	}
	else if (m_nGameState == GAME_STATE_SUSPEND)
	{
		m_strGameState = _T("��ͣ��,��������������ʼ��ť��������Ҽ�������");
	}
	else if (m_nGameState == GAME_STATE_OVER)
	{
		//IsGameOver������״̬������ʲôҲ������
	}
}

void CAmazonsView::UpdateTimeCounter()
{
	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	m_nTotalTimer += TIMER_INTERVAL;
	m_nPlayerTimer += TIMER_INTERVAL;

	if (pAmazons->IsPlayer1Turn())
		GetTimeString(m_nPlayerTimer, m_strPlayer1Timer);
	else
		GetTimeString(m_nPlayerTimer, m_strPlayer2Timer);

	GetTimeString(pAmazons->GetTotalTime(false), m_strPlayer2TotalTime);
	GetTimeString(pAmazons->GetTotalTime(true), m_strPlayer1TotalTime);
	GetTimeString(m_nTotalTimer, m_strTotalTimer);
}

void CAmazonsView::ComputerMoveFinished()
{
	if (IsOpponentNetplayer())
		m_networkPlayer.SendMoveMsg(m_posSelected, m_posMoveTo, m_posShootAt);

	CAmazonsGameControllerInterface *pAmazons = GetDocument()->pAmazons;
	pAmazons->AddToHistory(m_posSelected, m_posMoveTo, m_posShootAt, m_nPlayerTimer);
	pAmazons->SaveTempFile(TEMP_FILE_NAME);
	UpdateHistoryDlg();
	OpponentTurn();
}

void CAmazonsView::GetTimeString(int nTime, CString& strTime)
{
	nTime /= 1000;
	int nHour, nMinute, nSecond;
	CString str;
	nHour = nTime / 3600;
	nSecond = nTime % 60;
	nMinute = (nTime - 3600 * nHour - nSecond) / 60;
	strTime.Empty();
	if (nHour < 10)   str.Format(_T("0%d:"), nHour); else str.Format(_T("%d:"), nHour);     strTime += str;
	if (nMinute < 10) str.Format(_T("0%d:"), nMinute); else str.Format(_T("%d:"), nMinute); strTime += str;
	if (nSecond < 10) str.Format(_T("0%d"), nSecond); else str.Format(_T("%d"), nSecond); strTime += str;
}

void CAmazonsView::ForwardOrBackwardHistoryEnd()
{
	m_nCurState = STATE_UNSELECT;
	m_nGameState = GAME_STATE_SUSPEND;

	//��ȡ��ǰ���裬����mark
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	SHistoryRecord record = pAmazonAi->GetCurHistoryMove();
	m_posSelected = record.posSelected;
	m_posMoveTo = record.posMoveTo;
	m_posShootAt = record.posShootAt;

	IsGameOver();
	UpdateHistoryDlg();
	InvalidateRect(m_rectLeft);
}

TCHAR * CAmazonsView::GetPlayerName(int playerType)
{
	TCHAR *playerNames[3] = {
		_T("��"),
		_T("����"),
		_T("����"),
	};
	return playerNames[playerType];
}

TCHAR * CAmazonsView::GetPlayer1Color()
{
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	return pAmazonAi->m_bPlayer1Black ? _T("�ڷ�") : _T("�췽");
}

TCHAR * CAmazonsView::GetPlayer2Color()
{
	CAmazonsGameControllerInterface *pAmazonAi = GetDocument()->pAmazons;
	return pAmazonAi->m_bPlayer1Black ? _T("�췽") : _T("�ڷ�");
}

LRESULT CAmazonsView::OnNetworkConnectedToServer(WPARAM wParam, LPARAM lParam)
{
	if(m_dlgNetwork.m_hWnd != NULL)
		m_dlgNetwork.UpdateConnectStatus();
	return 0L;
}

LRESULT CAmazonsView::OnNetworkGetPlayerList(WPARAM wParam, LPARAM lParam)
{
	if (m_dlgNetwork.m_hWnd != NULL)
		m_dlgNetwork.UpdatePlayerList();
	return 0L;
}

LRESULT CAmazonsView::OnNetworkConnectedToOpponent(WPARAM wParam, LPARAM lParam)
{
	if (m_dlgNetwork.m_hWnd != NULL)
		m_dlgNetwork.UpdateOpponentInfo();
	return 0L;
}

LRESULT CAmazonsView::OnNetworkDisConnectToOpponent(WPARAM wParam, LPARAM lParam)
{
	if (m_dlgNetwork.m_hWnd != NULL)
		m_dlgNetwork.UpdateOpponentInfo();
	return 0L;
}

LRESULT CAmazonsView::OnNetworkLog(WPARAM wParam, LPARAM lParam)
{
	if (m_dlgNetwork.m_hWnd != NULL)
		m_dlgNetwork.UpdateLog();
	return 0L;
}
LRESULT CAmazonsView::OnNetworkMove(WPARAM wParam, LPARAM lParam)
{
	m_posSelected = m_posMoveTo = m_posShootAt = SPosition(-1, -1);
	m_networkPlayer.GetMove(g_posSelected, g_posMoveTo, g_posShootAt);
	m_bComputerMoving = true;
	m_nComputerMoveTimer = 0;
	return 0L;
}
LRESULT CAmazonsView::OnNetworkGameControl(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NET_GAME_CONTROLL_START)
	{
		OnCommandGameStartWrap(FALSE);
	}
	else if (wParam == NET_GAME_CONTROLL_PAUSE)
	{
		OnCmdGameSuspend();
	}
	else if (wParam == NET_GAME_CONTROLL_CONTINUE)
	{
		AfxMessageBox(_T("��ʵ��"));
	}
	else if (wParam == NET_GAME_CONTROLL_STOP)
	{
		AfxMessageBox(_T("��ʵ��"));
	}
	else
	{
		AfxMessageBox(_T("�����ָ��"));
	}
	return 0L;
}


