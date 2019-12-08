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
	int selectAction(float state[]);
	void saveToTargetModel();
	void addFakeReplayData();

	MAIDQNModel *m_policyNet;
	MAIDQNModel *m_targetNet;
	int m_stepsDone;
	torch::optim::Optimizer *m_optimiser;
	StateStruct m_lastState;
	int m_lastActionIndex;
	int m_runOnceIteration;
	
	struct
	{
		std::vector<StateStruct> states;
		std::vector<int> actionIndices;
		std::vector<StateStruct> nextStates;
		std::vector<float> rewards;
	} replayMemory;

public:
	MAIDQNTrainer(MAIDQNModel *model);
	//~MAIDQNTrainer();
	void run();
	ActionStruct runOnce();
};

#endif //SUPERTUXKART_MAI_DQNTRAINER_HPP
