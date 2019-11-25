#include "mai_controller.hpp"


MAIController::MAIController(AbstractKart* kart, int local_player_id, HandicapLevel h) : LocalPlayerController(kart, local_player_id, h)
{
	m_mai_engine = new MAIEngine();
}

MAIController::~MAIController()
= default;

void MAIController::update(int ticks) {
    m_mai_engine->update();
	const ActionStruct act = m_mai_engine->getAction();

	static unsigned long updateCount = 1;

	//if (World::getWorld()->getPhase() == WorldStatus::RACE_PHASE || World::getWorld()->getPhase() == WorldStatus::SET_PHASE || World::getWorld()->getPhase() == WorldStatus::GO_PHASE)
	if (World::getWorld()->getPhase() != WorldStatus::READY_PHASE)
		//MAIController::resetActions();
		MAIController::action(act.action, act.value, false);

	LocalPlayerController::update(ticks);
	updateCount++;
}

bool MAIController::action(PlayerAction action, int value, bool dry_run)
{
    if(action == PlayerAction::PA_TEST_1){
        //m_mai_engine->saveState();
    }
    if(action == PlayerAction::PA_TEST_2){
        //m_mai_engine->setStateAsCurrent(0);
    }

	return LocalPlayerController::action(action, value);
}

void MAIController::resetActions()
{
	MAIController::action(PA_ACCEL, 0, false);
	MAIController::action(PA_BRAKE, 0, false);
	MAIController::action(PA_STEER_LEFT, 0, false);
	MAIController::action(PA_STEER_RIGHT, 0, false);
}