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
public:
	MAIDQNTrainer(MAIDQNModel *model);
	~MAIDQNTrainer();
	void run();
};

#endif //SUPERTUXKART_MAI_DQNTRAINER_HPP
