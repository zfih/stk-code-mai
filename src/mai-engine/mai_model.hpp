//
// Created by LasseKB on 05/11/2019.
//

#ifndef SUPERTUXKART_MAI_MODEL_HPP
#define SUPERTUXKART_MAI_MODEL_HPP

#include <torch/torch.h>
#include "input/input.hpp"

//struct ActionStruct {
//	PlayerAction action;
//	uint16_t value;
//};

struct StateStruct {
	float downTrack;
	float distToMid;
	float rotation;
	float velX;
	float velY;
	float velZ;

	// queueing
	unsigned long id;
    bool operator<(const StateStruct& rhs) const
    {
        return id < rhs.id;
    }
};

class MAIModel {
private:

public:
	virtual std::vector<PlayerAction> getAction(/*State state*/) = 0;
};

#endif //SUPERTUXKART_MAI_MODEL_HPP
