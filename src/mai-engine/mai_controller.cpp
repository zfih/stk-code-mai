#include "mai_controller.hpp"


MAIController::MAIController(AbstractKart* kart, int local_player_id, HandicapLevel h) : LocalPlayerController(kart, local_player_id, h)
{
	m_mai_engine = new MAIEngine();
}

MAIController::~MAIController()
= default;

void MAIController::update(int ticks) {
	const PlayerAction act = m_mai_engine->getAction();

	static unsigned long updateCount = 1;

	if (World::getWorld()->getPhase() == WorldStatus::RACE_PHASE || World::getWorld()->getPhase() == WorldStatus::SET_PHASE || World::getWorld()->getPhase() == WorldStatus::GO_PHASE)
		LocalPlayerController::action(act, UINT16_MAX);

	if(updateCount % 1000 == 0)
	{
		LocalPlayerController::action(PlayerAction::PA_STEER_RIGHT, UINT16_MAX);
	}
	else if(updateCount % 500 == 0)
	{
		LocalPlayerController::action(PlayerAction::PA_STEER_LEFT, UINT16_MAX);
	}
	else if(updateCount % 50 == 0)
	{
		LocalPlayerController::action(PlayerAction::PA_STEER_RIGHT, 0);
		LocalPlayerController::action(PlayerAction::PA_STEER_LEFT, 0);
	}

	LocalPlayerController::update(ticks);
	updateCount++;
}

bool MAIController::action(PlayerAction action, int value, bool dry_run)
{
	return LocalPlayerController::action(action, value);
}