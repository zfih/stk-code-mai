//
// Created by LasseKB on 07/11/2019.
//

#ifndef SUPERTUXKART_MAI_DQNTRAINER_HPP
#define SUPERTUXKART_MAI_DQNTRAINER_HPP

#include "mai_algorithm.hpp"
#include "mai_dqnmodel.hpp"

class MAIDQNTrainer : public MAIAlgorithm {
private:
	MAIDQNModel *m_policyNet;
	MAIDQNModel *m_targetNet;
	int m_stepsDone;
	struct
	{
		std::vector<float> states;
		std::vector<PlayerAction> actions;
		std::vector<float> nextStates;
		std::vector<float> rewards;
	} replayMemory;

	void optimiseModel();
	PlayerAction selectAction(float state);
	torch::optim::RMSprop m_optimiser;
public:
	MAIDQNTrainer(MAIDQNModel *model);
	//~MAIDQNTrainer();
	void run();
};

#endif //SUPERTUXKART_MAI_DQNTRAINER_HPP
