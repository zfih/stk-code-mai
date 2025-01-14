//
// Created by LasseKB on 07/11/2019.
//

#ifndef SUPERTUXKART_MAI_ALGORITHM_HPP
#define SUPERTUXKART_MAI_ALGORITHM_HPP

#include <torch/torch.h>
#include "input/input.hpp"
#include "mai_model.hpp"

class MAIAlgorithm {
private:

public:
	virtual void run() = 0;
	virtual std::vector<PlayerAction> runOnce() = 0;
};

#endif //SUPERTUXKART_MAI_ALGORITHM_HPP
