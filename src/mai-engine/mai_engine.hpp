//
// Created by zfih on 03/11/2019.
//

#ifndef SUPERTUXKART_MAI_ENGINE_HPP
#define SUPERTUXKART_MAI_ENGINE_HPP

#include <torch/torch.h>
#include "modes/world.hpp"
#include "game_state.hpp"
#include "mai_modeltest.hpp"
#include "mai_algorithm.hpp"

class World;

class MAIEngine {
private:
	static MAIEngine *m_mai_engine;
	World *m_world;

	//GameState *m_game_state;
	//std::vector<GameState*> m_game_states;
	int m_game_state_count;

    StateManager *m_state_manager;

	MAIModel *m_mai_model;
	MAIAlgorithm* m_mai_algorithm;

public:
	bool training;
	
	MAIEngine();
	~MAIEngine();

	ActionStruct getAction();
	static MAIEngine *getMAIEngine();

    void update();

    void saveState();

    void setStateAsCurrent(int id);
};

#endif //SUPERTUXKART_MAI_ENGINE_HPP
