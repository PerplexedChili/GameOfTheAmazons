#ifndef AMAZONS_AI_GAME_CONTROLLER_H
#define AMAZONS_AI_GAME_CONTROLLER_H

#include "StdAfx.h"
#include "AmazonsInterface.h"

/*
�������ݱ�ʾ
��1����10*10������洢
��2�����1�����Ӷ�ӦֵΪ1��2��3��4�����2�����Ӷ�ӦֵΪ5��6��7��8
��3���յ�λ��ֵΪ0���ż�λ��Ϊ-1
*/

struct SNode//����Ҫʹ�ã������ɾ������
{
	int n;//����ʱ�������,��ʽ��ɾ��֮
	int nValue;
	int nSelected;
	int nMoveTo;
	int nShootAt;
	SNode* pParent;//��ĸ(Alpha-Beta������)
	SNode* pBrother;//�ֵ�
	SNode* pChild;//����
};

class CAmazonsGameController : public CAmazonsGameControllerInterface
{
private:
	int m_plate[10][10];
	SPosition m_pieces[9];
	//
	SHistoryRecord m_histroy[92];
	int m_nHistoryPtr;//��ʼֵΪ0�����˺�Ϊ93
	int m_nCurHisttoryPtr;//������ǰ�������ʱ������ֵ��仯����m_nHistoryPtr����

public:
	CAmazonsGameController(){};
	~CAmazonsGameController(){};

public:
	virtual void Init();
	virtual void NewGame();
	//
	virtual void GetPlate(int outPlate[10][10]);
	virtual int  GetPlateCellValue(int i, int j);
	virtual void GetPieces(SPosition outPieces[9]);
	//
	virtual void Move(SPosition posFrom, SPosition posTo);
	virtual void Shoot(SPosition posShootAt);
	virtual void CancelMove(SPosition posFrom, SPosition posTo);
	virtual void CancelShoot(SPosition posShootAt);
	virtual bool CanMoveOrShoot(SPosition posFrom, SPosition posTo);
	//
	virtual bool IsPieces(int n);
	virtual bool IsEmpty(int n);
	virtual bool IsArrow(int n);
	virtual bool IsPlayer1(int n);
	virtual bool IsPlayer2(int n);
	virtual int IsGameOver();//����ֵ��0=δ��ʤ����1=���1Ӯ��2=���2Ӯ��3=ƽ��
	virtual bool IsPlayer1Turn();//�Ƿ��ֵ����1���ӣ�����true��ʾ�ֵ����1������false��ʾ�ֵ����2
	virtual bool IsPlayer1Turn(int nHistoryPtr);//�Ƿ��ֵ����1���ӣ�����true��ʾ�ֵ����1������false��ʾ�ֵ����2
	//
	virtual void SaveToFile(CString filePathName);
	virtual void LoadFromFile(CString filePathName);
	virtual void SaveTempFile(CString filePathName);
	//
	virtual int GetTotalTime(bool bPlayer1);
	virtual void GetHistory(SHistoryRecord history[], int &outPtr, int &outCurPtr);
	virtual SHistoryRecord GetCurHistoryMove();
	virtual void AddToHistory(SPosition posSelect, SPosition posMoveTo, SPosition posShootAt, int time);//AddToHistory��ʹָ����ƣ��Ӷ��öԷ���ʼ����
	virtual void Forward(int step);
	virtual void Backward(int step);
	virtual bool CanForward(int step);
	virtual bool CanBackward(int step);

public:
	static void Move(int plate[10][10], SPosition pieces[9], SPosition posFrom, SPosition posTo);
	static void Shoot(int plate[10][10], SPosition pieces[9], SPosition posShootAt);
	static void CancelMove(int plate[10][10], SPosition pieces[9], SPosition posFrom, SPosition posTo);
	static void CancelShoot(int plate[10][10], SPosition pieces[9], SPosition posShootAt);
	static bool CanMoveOrShoot(int plate[10][10], SPosition pieces[9], SPosition posFrom, SPosition posTo);
};

#endif