#include "mai_controller.hpp"


MAIController::MAIController(AbstractKart* kart, int local_player_id, HandicapLevel h) : LocalPlayerController(kart, local_player_id, h)
{
	m_mai_engine = new MAIEngine();
}

MAIController::~MAIController()
= default;

void MAIController::update(int ticks) {
    m_mai_engine->update();
	const PlayerAction act = m_mai_engine->getAction();

	static unsigned long updateCount = 1;

	if (World::getWorld()->getPhase() == WorldStatus::RACE_PHASE || World::getWorld()->getPhase() == WorldStatus::SET_PHASE || World::getWorld()->getPhase() == WorldStatus::GO_PHASE)
        MAIController::action(act, UINT16_MAX, false);

	LocalPlayerController::update(ticks);
	updateCount++;
}

bool MAIController::action(PlayerAction action, int value, bool dry_run)
{
    if(action == PlayerAction::PA_TEST_1){
        Log::info("MAIController", "Test 1");
    }
    if(action == PlayerAction::PA_TEST_2){
        Log::info("MAIController", "Test 2");
    }

	return LocalPlayerController::action(action, value);
}