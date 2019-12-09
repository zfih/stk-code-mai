//
// Created by LasseKB on 05/11/2019.
//

#ifndef SUPERTUXKART_MAI_DQNMODEL_HPP
#define SUPERTUXKART_MAI_DQNMODEL_HPP

#include "mai_model.hpp"
#include "karts/abstract_kart.hpp"

class MAIDQNModel : public MAIModel/*, public torch::nn::Module*/ {
private:
	int chooseBest(torch::TensorAccessor<float, 1, torch::DefaultPtrTraits, int64_t> theVals);
	int chooseProbability(torch::TensorAccessor<float, 1, torch::DefaultPtrTraits, int64_t> theVals);

	int m_kartID;
	AbstractKart* m_kart;
	std::vector<std::vector<PlayerAction>> m_actions;

	torch::nn::Module *m_module;
	std::shared_ptr<torch::nn::LinearImpl> m_inLayer;
	std::shared_ptr<torch::nn::LinearImpl> m_hiddenLayerOne;
	std::shared_ptr<torch::nn::LinearImpl> m_hiddenLayerTwo;
	std::shared_ptr<torch::nn::LinearImpl> m_outLayer;
public:
	MAIDQNModel();
	//MAIDQNModel(int kartID);
	~MAIDQNModel();

	torch::nn::Module *getModule();
	std::vector<PlayerAction> getAction(/*State state*/);
	int getAction(float state[]);
	torch::Tensor pseudoForward(float state[]);
	torch::Tensor forward(torch::Tensor x);
	int getKartID();
	int getNumActions();
	std::vector<PlayerAction> getAction(int index);
};

#endif //SUPERTUXKART_MAI_MDQNODEL_HPP
