#pragma once

#include "NetMsg.h"

class CMyAsyncSocket;
class CGameServerDlg;

struct SClientInfo
{
	SNetPlayerInfo m_playerInfo;
	CMyAsyncSocket *m_pSocket;
	int m_lastMsgTime;
	int m_nOpponentId;

	void CloseSocket()
	{
		if (m_pSocket != NULL)
		{
			delete m_pSocket;
			m_pSocket = NULL;
		}
	}
};

class CGameServer
{
public:
	CGameServer();
	~CGameServer();

public:
	bool IsStarted(){ return m_pSocketServer != NULL; }
	void Init(CGameServerDlg* pDlg);
	void Start();
	void Stop();
	bool CheckAlive();//���ɾ����һ���ͻ��ˣ�����true

	void OnReceive(CMyAsyncSocket *pS);
	void OnSend(CMyAsyncSocket *pS);
	void OnAccept(CMyAsyncSocket *pS);
	void OnConnect(CMyAsyncSocket *pS);
	void OnClose(CMyAsyncSocket *pS);

private:
	void RecvMsg(CMyAsyncSocket *pS);
	void SendMsg(CMyAsyncSocket *pS, SNetMsgBase *pMsg);
	void AcceptClient();//����յ��˿ͻ��ˣ�����true
	void RemoveClient(CMyAsyncSocket *pS);
	//
	void ReadMsgFromBufferAndDispatch(CMyAsyncSocket *pS);
	void SendNetMsgScPlayerId(CMyAsyncSocket *pS, int id);
	void SendNetMsgScPlayerList(CMyAsyncSocket *pS, int id);
	void SendNetMsgScConnect(CMyAsyncSocket *pS, int inviterId, int inviteeId);
	void SendNetMsgScConnectConfirm(CMyAsyncSocket *pS, int inviterId, int inviteeId, int answer);
	void SendNetMsgScCancelChallenge(CMyAsyncSocket *pS);
	void SendNetMsgScMove(CMyAsyncSocket *pS, int ijArray[6]);
	void SendNetMsgScGameControl(CMyAsyncSocket *pS, int action);
	void SendNetMsgScError(CMyAsyncSocket *pS, CString strError);
	void HandleNetMsgCsPlayerId(CMyAsyncSocket *pS, SNetMsgCsRequireId &msg);
	void HandleNetMsgCsPlayerInfo(SNetMsgCsPlayerInfo &msg);
	void HandleNetMsgCsRequirePlayerList(SNetMsgCsRequirePlayerList &msg);
	void HandleNetMsgCsChallenge(SNetMsgCsChallenge &msg);
	void HandleNetMsgCsChallengeConfirm(SNetMsgCsChallengeConfirm &msg);
	void HandleNetMsgCsCancelChallenge(SNetMsgCsCancelChallenge &msg);
	void HandleNetMsgCsMove(SNetMsgCsMove &msg);
	void HandleNetMsgCsGameControl(SNetMsgCsGameControl &msg);
	//
	SClientInfo *GetClientInfoById(int playerId);
	void PushClientListToAllClients(int excepPlayerId);//���пͻ������ӻ��߶Ͽ�����ʱ�����¿ͻ����б�
	void TipSocketError(CString msg);
	void Log(CString str);

private:
	CMyAsyncSocket *m_pSocketServer;
	CGameServerDlg *m_pDlg;
	int m_nCounter;//�������������id������ͬ�����������
	char m_recvBuffer[4096 * 8];
	int m_nRecvBufferLen;
	char m_sendBuffer[4096 * 8];
	int m_nPort;
	CString m_strIp;
	CString m_strTempLog;

public:
	CArray<SClientInfo> clientArray;
	CString m_strLog;

};
