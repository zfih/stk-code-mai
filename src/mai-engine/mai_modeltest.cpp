//
// Created by zfih on 07/11/2019.
//

#include "mai_modeltest.hpp"
#include "modes/standard_race.hpp" // This should probably not be here.

MAIModelTest::MAIModelTest(int kartID)
{
	m_kartID = kartID;
}

MAIModelTest::~MAIModelTest(){ }

std::vector<PlayerAction> MAIModelTest::getAction()
{
	return {PlayerAction::PA_ACCEL};
}

int MAIModelTest::getKartID()
{
	return m_kartID;
}