//
// Created by zfih on 03/11/2019.
//

#include "mai_engine.hpp"

MAIEngine *MAIEngine::m_mai_engine = nullptr;

MAIEngine::MAIEngine()
{
	m_world = World::getWorld();
	m_mai_engine = this;
	m_state_manager = StateManager::get();
	m_state_manager->getGameState();
	m_mai_model = new MAIModelTest(0); // TODO: GO AWAY FROM TEST MODEL!
}

MAIEngine::~MAIEngine()
{
	m_world = nullptr; // we don't delete world, others may be using it.

	delete m_mai_engine;
	m_mai_engine = nullptr;
}

PlayerAction MAIEngine::getAction()
{
	return m_mai_model->getAction();
}

MAIEngine* MAIEngine::getMAIEngine()
{
	return m_mai_engine;
}

