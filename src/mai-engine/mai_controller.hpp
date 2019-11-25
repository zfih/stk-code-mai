#ifndef HEADER_MAI_CONTROLLER_HPP
#define HEADER_MAI_CONTROLLER_HPP

#include "mai-engine/mai_engine.hpp"
#include "karts/controller/local_player_controller.hpp"

class LocalPlayerController;

class MAIController : public LocalPlayerController
{
private:
	void resetActions();
public:
	MAIController(AbstractKart* kart, int local_player_id, HandicapLevel h);
	~MAIController();

	MAIEngine *m_mai_engine;

    void update(int ticks);
	bool action(PlayerAction action, int value, bool dry_run);
};

#endif // HEADER_MAI_CONTROLLER_HPP
