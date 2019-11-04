//
// Created by zfih on 03/11/2019.
//

#ifndef SUPERTUXKART_MAI_ENGINE_HPP
#define SUPERTUXKART_MAI_ENGINE_HPP

#include <torch/torch.h>
#include "modes/world.hpp"

class World;


class MAIEngine {
private:
	static MAIEngine *m_mai_engine;
	World *m_world;

public:
	MAIEngine();
	~MAIEngine();

	PlayerAction getAction();
	static MAIEngine *getMAIEngine();

};

#endif //SUPERTUXKART_MAI_ENGINE_HPP
