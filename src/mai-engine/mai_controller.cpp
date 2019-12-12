#include <modes/standard_race.hpp>
#include <config/user_config.hpp>
#include "mai_controller.hpp"
#include "mai_dqntrainer.hpp"


class m_no;

MAIController::MAIController(AbstractKart* kart, int local_player_id, HandicapLevel h) : LocalPlayerController(kart, local_player_id, h)
{
	m_mai_engine = new MAIEngine();
}

MAIController::~MAIController()
= default;

void MAIController::update(int ticks) {
    static unsigned long updateCount = 1;

    static std::vector<PlayerAction> act;

    if(!UserConfigParams::m_mai_no_network && updateCount % 20 == 0){
        m_mai_engine->update();
        act = m_mai_engine->getAction();
    }

    auto *srWorld = dynamic_cast<StandardRace*>(World::getWorld());
    float downTrack = srWorld->getDistanceDownTrackForKart(m_kart->getWorldKartId(),true);
    float downTrackNoChecklines = srWorld->getDistanceDownTrackForKart(m_kart->getWorldKartId(),false);
    float distToMid = srWorld->getDistanceToCenterForKart(m_kart->getWorldKartId());
	float turn = m_kart->getHeading();
    float trackLength = Track::getCurrentTrack()->getTrackLength();

    downTrack = downTrack > 0.0f ? downTrack : -(trackLength - downTrackNoChecklines);
    downTrack += trackLength * srWorld->getFinishedLapsOfKart(m_kart->getWorldKartId());

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << std::setw(6) << updateCount; // update count
    ss << " | " << std::setw(9) << downTrack << " (" << std::setw(9) << downTrackNoChecklines << ")"; // distance down track
    ss << " ||||";
    ss << " | " << std::setw(9) << distToMid;
	if (act.size() > 0) {
		ss << " | " << std::setw(10) << KartActionStrings[act[0]];
		if (act.size() > 1) {
			ss << " | " << std::setw(10) << KartActionStrings[act[1]];
		}
		else {
			ss << " | " << std::setw(10) << "None";
		}
	}
	//ss << " | " << std::setw(5) << act.value;
	ss << " | " << std::setw(5) << turn;
	ss << " | " << std::setw(5) << m_kart->getVelocity().x() << ", " << std::setw(5) << m_kart->getVelocity().y() << ", " << std::setw(5) << m_kart->getVelocity().z();
    if(UserConfigParams::m_training) ss << " | [" << "t" << "]";
    ss << " | ";
    std::string string = ss.str();

    std::cout << "\r" << string << std::flush;

	//if (World::getWorld()->getPhase() == WorldStatus::RACE_PHASE || World::getWorld()->getPhase() == WorldStatus::SET_PHASE || World::getWorld()->getPhase() == WorldStatus::GO_PHASE)
	if (!UserConfigParams::m_mai_no_network && updateCount % 20 == 0) {
		if (World::getWorld()->getPhase() != WorldStatus::READY_PHASE) {
			MAIController::resetActions();
			for (PlayerAction pAct : act) {
				MAIController::action(pAct, UINT16_MAX, false);
			}
		}
	}

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