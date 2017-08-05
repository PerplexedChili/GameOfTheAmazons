#include "StdAfx.h"
#include "AmazonsGameController.h"
#include "AmazonsAI.h"

void CAmazonsAI::AICompute(
	int plate[10][10],			//��������
	SPosition pieces[],			//8�����ӵ�λ�ã�����Ϊ9
	bool isPlayer1,				//�Ƿ�����Ϊ���1���м��㣬������Ϊ���2���м���
	SPosition& outPosSelected,	//�������
	SPosition& outPosMoveTo,	//�������
	SPosition& outPosShootAt)	//�������
{
	m_nOutMaxValue = 0;
	m_nOutDepth = 0;
	m_nOutNumOfMoves = 0;
	m_nOutNumOfNodes = 0;
	m_nOutNumOfSkippedNodes = 0;

	bool bFinished = false;

	while (!bFinished)
	{
		//���ѡ��һ������
		int piece = 0;
		if (isPlayer1)
			piece = rand() % 4 + 1;
		else
			piece = rand() % 4 + 5;
		outPosSelected = pieces[piece];

		//�������λ��
		SPosition pos;
		pos.i = rand() % 10;
		pos.j = rand() % 10;

		if (CAmazonsGameController::CanMoveOrShoot(plate, pieces, outPosSelected, pos))
		{
			CAmazonsGameController::Move(plate, pieces, outPosSelected, pos);
			outPosMoveTo = pos;
		}
		else
			continue;

		//����ż�
		pos.i = rand() % 10;
		pos.j = rand() % 10;
		if (CAmazonsGameController::CanMoveOrShoot(plate, pieces, outPosMoveTo, pos))
		{
			outPosShootAt = pos;
			bFinished = true;
		}
		else
		{
			CAmazonsGameController::CancelMove(plate, pieces, outPosSelected, outPosMoveTo);
		}
	}
}