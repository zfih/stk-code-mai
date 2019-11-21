//
// Created by LasseKB on 07/11/2019.
//

#ifndef SUPERTUXKART_MAI_DQNTRAINER_HPP
#define SUPERTUXKART_MAI_DQNTRAINER_HPP

#include "mai_algorithm.hpp"
#include "mai_dqnmodel.hpp"

class MAIDQNTrainer : public MAIAlgorithm {
private:
	void optimiseModel();
	PlayerAction selectAction(float state);
	void saveToTargetModel();

	MAIDQNModel *m_policyNet;
	MAIDQNModel *m_targetNet;
	int m_stepsDone;
	torch::optim::Optimizer *m_optimiser;
	float m_lastState;
	PlayerAction m_lastAction;
	int m_runOnceIteration;
	
	struct
	{
		std::vector<float> states;
		std::vector<PlayerAction> actions;
		std::vector<float> nextStates;
		std::vector<float> rewards;
	} replayMemory;
public:
	MAIDQNTrainer(MAIDQNModel *model);
	//~MAIDQNTrainer();
	void run();
	PlayerAction runOnce();
};

#endif //SUPERTUXKART_MAI_DQNTRAINER_HPP
