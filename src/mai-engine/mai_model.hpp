//
// Created by LasseKB on 05/11/2019.
//

#ifndef SUPERTUXKART_MAI_MODEL_HPP
#define SUPERTUXKART_MAI_MODEL_HPP

#include <torch/torch.h>
#include "input/input.hpp"

class MAIModel {
private:

public:
	virtual PlayerAction getAction(/*State state*/) = 0;
};

#endif //SUPERTUXKART_MAI_MODEL_HPP
