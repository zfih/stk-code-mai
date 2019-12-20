//
// Created by zfih on 03/11/2019.
//

#include "mai_engine.hpp"
#include "mai_dqnmodel.hpp"
#include "mai_dqntrainer.hpp"
#include "config/user_config.hpp"

MAIEngine *MAIEngine::m_mai_engine = nullptr;

MAIEngine::MAIEngine()
{
	m_world = World::getWorld();
	//m_game_state = new GameState(m_world);
	m_mai_engine = this;
	m_state_manager = StateManager::get();
	m_state_manager->getGameState();
	//m_mai_model = new MAIModelTest(0); // TODO: GO AWAY FROM TEST MODEL!
	//int playerID = m_world->getNumKarts() - 1;
	//std::cout << playerID << "<-\n";
	m_mai_model = new MAIDQNModel();
	m_mai_algorithm = new MAIDQNTrainer(dynamic_cast<MAIDQNModel*>(m_mai_model));
	m_game_state_count = 0;
}

MAIEngine::~MAIEngine()
{
	m_world = nullptr; // we don't delete world, others may be using it.

	delete m_mai_engine;
	m_mai_engine = nullptr;
}

std::vector<PlayerAction> MAIEngine::getAction()
{
	//return PlayerAction::PA_ACCEL;
	/*m_mai_algorithm->run();
	return m_mai_model->getAction();*/
	if (UserConfigParams::m_training)
		return m_mai_algorithm->runOnce();
	return m_mai_model->getAction();
}

void MAIEngine::update(){
	//if (UserConfigParams::m_training)
		//std::cout << "Training" << std::endl;
    //m_game_state->update();
}

void MAIEngine::saveState(){
    //GameState state = m_game_state->copyGameState();
    //m_game_states.push_back(&state);
    //m_game_state_count++;
}

void MAIEngine::setStateAsCurrent(int id){
    //m_game_states[m_game_state_count-1]->makeStateCurrentState();
    //m_game_state = m_game_states[m_game_state_count-1];
}

MAIEngine* MAIEngine::getMAIEngine()
{
	return m_mai_engine;
}
