#ifndef AMAZONS_INTERFACE_H
#define AMAZONS_INTERFACE_H

#include "StdAfx.h"

//�������
#define PLAYER_HUMAN	0
#define PLAYER_COMPUTER 1
#define PLAYER_NETWORK	2

//λ������
struct SPosition
{
	int i;//��
	int j;//��

	SPosition()
	{
	}

	SPosition(int i, int j)
	{
		this->i = i;
		this->j = j;
	}
};

//��ʷ����
struct SHistoryRecord
{
	int spentTime;
	SPosition posSelected;//ѡ��
	SPosition posMoveTo;//����
	SPosition posShootAt;//�ż�
};

class CAmazonsGameControllerInterface
{
public:
	bool m_bPlayer1Black;//���1�Ƿ��Ǻ��
	bool m_bPlayer1First;//���1�Ƿ�����
	int m_nPlayer1Type;//���1����
	int m_nPlayer2Type;//���2����

public:
	virtual void Init(){};
	virtual void NewGame(){};
	//
	virtual void GetPlate(int outPlate[10][10]){};
	virtual void GetPieces(SPosition outPieces[9]){};
	virtual int  GetPlateCellValue(int i, int j){ return 0; }
	//
	virtual void Move(SPosition posFrom, SPosition posTo){};
	virtual void Shoot(SPosition posShootAt){};
	virtual void CancelMove(SPosition posFrom, SPosition posTo){};
	virtual void CancelShoot(SPosition posShootAt){};
	virtual bool CanMoveOrShoot(SPosition posFrom, SPosition posTo){ return true; };
	//
	virtual bool IsPieces(int n){ return true; };
	virtual bool IsEmpty(int n){ return true; };
	virtual bool IsArrow(int n){ return true; };
	virtual bool IsPlayer1(int n){ return true; };
	virtual bool IsPlayer2(int n){ return true; };
	virtual int  IsGameOver(){ return 0; };//����ֵ��0=δ��ʤ����1=���1Ӯ��2=���2Ӯ��3=ƽ��
	virtual bool IsPlayer1Turn(){ return true; };//�Ƿ��ֵ����1���ӣ�����true��ʾ�ֵ����1������false��ʾ�ֵ����2
	virtual bool IsPlayer1Turn(int nHistoryPtr){ return true; };//�Ƿ��ֵ����1���ӣ�����true��ʾ�ֵ����1������false��ʾ�ֵ����2
	//
	virtual void SaveToFile(CString filePathName){};
	virtual void LoadFromFile(CString filePathName){};
	virtual void SaveTempFile(CString filePathName){};
	//
	virtual int GetTotalTime(bool bPlayer1){ return 0; };
	virtual void GetHistory(SHistoryRecord history[], int &outHistoryPtr, int &outCurPtr){};
	virtual SHistoryRecord GetCurHistoryMove(){ return SHistoryRecord(); };
	virtual void AddToHistory(SPosition posSelect, SPosition posMoveTo, SPosition posShootAt, int time){};//AddToHistory��ʹָ����ƣ��Ӷ��öԷ���ʼ����
	virtual void Forward(int step){};
	virtual void Backward(int step){};
	virtual bool CanForward(int step){ return true; }
	virtual bool CanBackward(int step){ return true; }
};

class CAmazonsAIInterface
{
public:
	int m_nOutMaxValue;//(���)�����������ֵ
	int m_nOutDepth;//(���)�������
	int m_nOutNumOfMoves;//(���)�����ŷ���
	int m_nOutNumOfNodes;//(���)��������Ľڵ���(���������Ľڵ���)(������)
	int m_nOutNumOfSkippedNodes;//(���)����������Ľڵ���(���ӵ��Ľڵ�Ҳ�����)

public:
	virtual void AICompute(
		int plate[10][10],			//��������
		SPosition pieces[],			//8�����ӵ�λ�ã�����Ϊ9
		bool isPlayer1,				//�Ƿ�����Ϊ���1���м��㣬������Ϊ���2���м���
		SPosition& outPosSelected,	//�������
		SPosition& outPosMoveTo,	//�������
		SPosition& outPosShootAt){};	//�������
	virtual void StopComputing(){};
	//
};

#endif