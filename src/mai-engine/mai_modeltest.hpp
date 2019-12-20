//
// Created by zfih on 07/11/2019.
//

#ifndef SUPERTUXKART_MAI_MODELTEST_HPP
#define SUPERTUXKART_MAI_MODELTEST_HPP

#include "mai_model.hpp"

class MAIModelTest : public MAIModel, public torch::nn::Module {
private:
	int m_kartID;

public:
	MAIModelTest(int kartID);
	~MAIModelTest();

	std::vector<PlayerAction> getAction(/*State state*/);
	int getKartID();
};

#endif //SUPERTUXKART_MAI_MODELTEST_HPP