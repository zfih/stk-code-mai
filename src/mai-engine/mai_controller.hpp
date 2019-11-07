#ifndef HEADER_MAI_CONTROLLER_HPP
#define HEADER_MAI_CONTROLLER_HPP

#include "mai-engine/mai_engine.hpp"
#include <karts/controller/player_controller.hpp>

class MAIController : public PlayerController
{
public:
	MAIController(AbstractKart *kart);
	~MAIController();


	MAIEngine *m_mai_engine;

    void update(int ticks);
};

#endif // HEADER_MAI_CONTROLLER_HPP
