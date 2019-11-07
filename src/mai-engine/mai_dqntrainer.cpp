//
// Created by LasseKB on 07/11/2019.
//

// Based on https://radicalrafi.github.io/posts/pytorch-cpp-intro/ and https://github.com/goldsborough/examples/blob/cpp/cpp/mnist/mnist.cpp

#include "mai_dqntrainer.hpp"

MAIDQNTrainer::MAIDQNTrainer(MAIDQNModel *model) {
	MAIDQNTrainer::m_policyNet = model;
	MAIDQNTrainer::m_targetNet = new MAIDQNModel(model->getKartID());
}

void MAIDQNTrainer::run() {
	std::cout << "Running is fun :D\n";
}