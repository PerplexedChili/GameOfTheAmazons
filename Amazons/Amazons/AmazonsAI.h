#ifndef AMAZONS_AI__H
#define AMAZONS_AI__H

#include "AmazonsInterface.h"

class CAmazonsAI : public CAmazonsAIInterface
{
public:
	virtual void AICompute(
		int plate[10][10],			//��������
		SPosition pieces[],			//8�����ӵ�λ�ã�����Ϊ9
		bool isPlayer1,				//�Ƿ�����Ϊ���1���м��㣬������Ϊ���2���м���
		SPosition& outPosSelected,	//�������
		SPosition& outPosMoveTo,	//�������
		SPosition& outPosShootAt);	//�������
};
#endif