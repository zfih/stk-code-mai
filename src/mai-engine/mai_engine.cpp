//
// Created by zfih on 03/11/2019.
//

#include "mai_engine.hpp"

MAIEngine *MAIEngine::m_mai_engine = nullptr;

MAIEngine::MAIEngine()
{
	m_world = World::getWorld();
	m_game_state = new GameState;
	//m_game_states.emplace_back(&m_game_state); TODO: FIX
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

void MAIEngine::update(){ // TODO: DO NOT CALL YET!
    m_game_state->update();
}

void MAIEngine::saveState(){
    m_game_states.push_back(m_game_state->copyGameState());
}

void MAIEngine::setStateAsCurrent(int id){
    m_game_states[id].makeStateCurrentState();
    m_game_state = &m_game_states[id];
}

MAIEngine* MAIEngine::getMAIEngine()
{
	return m_mai_engine;
}

