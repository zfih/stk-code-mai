//
// Created by LasseKB on 05/11/2019.
//

#ifndef SUPERTUXKART_MAI_DQNMODEL_HPP
#define SUPERTUXKART_MAI_DQNMODEL_HPP

#include "mai_model.hpp"

class MAIDQNModel : public MAIModel, public torch::nn::Module {
private:
	int m_kartID;
	std::vector<PlayerAction> m_actions;

	std::shared_ptr<torch::nn::LinearImpl> m_inLayer;
	std::shared_ptr<torch::nn::LinearImpl> m_hiddenLayerOne;
	std::shared_ptr<torch::nn::LinearImpl> m_hiddenLayerTwo;
	std::shared_ptr<torch::nn::LinearImpl> m_outLayer;
public:
	MAIDQNModel(int kartID);
	~MAIDQNModel();

	PlayerAction getAction(/*State state*/);
	int getKartID();
};

#endif //SUPERTUXKART_MAI_MDQNODEL_HPP
