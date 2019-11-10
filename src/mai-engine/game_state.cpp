//
// Created by zfih on 06/11/2019.
//

#include "game_state.hpp"
#include "mai_engine.hpp"

GameState::GameState(){
    m_world = World::getWorld();
    init();
}

GameState::GameState(const GameState &state) { // Copy constructor
    m_world             = state.m_world; // TODO: make copyable world
    m_transform_events  = state.m_transform_events;
    m_physic_info       = state.m_physic_info;
    m_bonus_info        = state.m_bonus_info;
    m_kart_replay_event = state.m_kart_replay_event;
    m_last_saved_time   = state.m_last_saved_time;
    m_count_transforms  = state.m_count_transforms;
    m_complete_replay   = state.m_complete_replay;
    m_incorrect_replay  = state.m_incorrect_replay;
    m_max_frames        = state.m_max_frames;
    m_previous_steer    = state.m_previous_steer;
}

GameState::~GameState() {
    // destroy
}


void GameState::init() {
    m_transform_events.resize(race_manager->getNumberOfKarts());
    m_physic_info.resize(race_manager->getNumberOfKarts());
    m_bonus_info.resize(race_manager->getNumberOfKarts());
    m_kart_replay_event.resize(race_manager->getNumberOfKarts());

    for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
    {
        m_transform_events[i].resize(m_max_frames);
        m_physic_info[i].resize(m_max_frames);
        m_bonus_info[i].resize(m_max_frames);
        m_kart_replay_event[i].resize(m_max_frames);
    }

    m_count_transforms.resize(race_manager->getNumberOfKarts(), 0);
    m_last_saved_time.resize(race_manager->getNumberOfKarts(), -1.0f);
}

void GameState::reset() {
    m_complete_replay = false;
    m_incorrect_replay = false;
    m_transform_events.clear();
    m_physic_info.clear();
    m_bonus_info.clear();
    m_kart_replay_event.clear();
    m_count_transforms.clear();
    m_last_saved_time.clear();
}

void GameState::update() {
    if (m_incorrect_replay || m_complete_replay) return;

    m_world = World::getWorld();

    const bool single_player = race_manager->getNumPlayers() == 1;
    unsigned int num_karts = m_world->getNumKarts();

    float time = m_world->getTime();

    for(unsigned int i=0; i<num_karts; i++)
    {
        AbstractKart *kart = m_world->getKart(i);
        // If a single player give up in game menu, stop recording
        if (kart->isEliminated() && single_player) return;

        if (kart->isGhostKart()) continue;


        if ( time - m_last_saved_time[i] < (stk_config->m_replay_dt - stk_config->ticks2Time(1)))
        {
            continue;
        }

        m_previous_steer = kart->getControls().getSteer();
        m_last_saved_time[i] = time;
        m_count_transforms[i]++;
        if (m_count_transforms[i] >= m_transform_events[i].size())
        {
            // Only print this message once.
            if (m_count_transforms[i] == m_transform_events[i].size())
            {
                char buffer[100];
                sprintf(buffer, "Can't store more events for kart %s.",
                        kart->getIdent().c_str());
                Log::warn("ReplayRecorder", buffer);
                m_incorrect_replay = single_player;
            }
            continue;
        }
        TransformEvent *p      = &(m_transform_events[i][m_count_transforms[i]-1]);
        PhysicInfo *q          = &(m_physic_info[i][m_count_transforms[i]-1]);
        BonusInfo *b           = &(m_bonus_info[i][m_count_transforms[i]-1]);
        KartReplayEvent *r     = &(m_kart_replay_event[i][m_count_transforms[i]-1]);

        p->m_time              = World::getWorld()->getTime();
        p->m_transform.setOrigin(kart->getXYZ());
        p->m_transform.setRotation(kart->getVisualRotation());

        q->m_speed             = kart->getSpeed();
        q->m_steer             = kart->getSteerPercent();
        const int num_wheels = kart->getVehicle()->getNumWheels();
        for (int j = 0; j < 4; j++)
        {
            if (j > num_wheels || num_wheels == 0)
                q->m_suspension_length[j] = 0.0f;
            else
            {
                q->m_suspension_length[j] = kart->getVehicle()
                        ->getWheelInfo(j).m_raycastInfo.m_suspensionLength;
            }
        }
        q->m_skidding_state    = kart->getSkidding()->getSkidState();

        b->m_attachment        = kart->getAttachment()->getType();
        b->m_nitro_amount      = kart->getEnergy();
        b->m_item_amount       = kart->getNumPowerup();
        b->m_item_type         = kart->getPowerup()->getType();

        //Only saves distance if recording a linear race
        if (race_manager->isLinearRaceMode())
        {
            const LinearWorld *linearworld = dynamic_cast<LinearWorld*>(World::getWorld());
            r->m_distance = linearworld->getOverallDistance(kart->getWorldKartId());
        }
        else
            r->m_distance = 0.0f;

        kart->getKartGFX()->getGFXStatus(&(r->m_nitro_usage),
                                         &(r->m_zipper_usage), &(r->m_skidding_effect), &(r->m_red_skidding));
        r->m_jumping = kart->isJumping();
    }   // for i

    if (m_world->getPhase() == World::RESULT_DISPLAY_PHASE && !m_complete_replay)
    {
        m_complete_replay = true;
    }
}   // update


void GameState::makeStateCurrentState() {
    World::setWorld(m_world);

	unsigned int num_karts = m_world->getNumKarts();

	for (unsigned int i = 0; i < num_karts; i++)
	{
		AbstractKart* kart = m_world->getKart(i);

        TransformEvent *p      = &(m_transform_events[i][m_count_transforms[i]-1]);
        PhysicInfo *q          = &(m_physic_info[i][m_count_transforms[i]-1]);
        BonusInfo *b           = &(m_bonus_info[i][m_count_transforms[i]-1]);
        KartReplayEvent *r     = &(m_kart_replay_event[i][m_count_transforms[i]-1]);

        m_world->setTime(p->m_time);

        kart->setXYZ(p->m_transform.getOrigin());
        kart->setRotation(p->m_transform.getRotation());

        kart->setSpeed(q->m_speed);
        kart->getControls().setSteer(q->m_steer);

        const int num_wheels = kart->getVehicle()->getNumWheels();
        for (int j = 0; j < 4; j++)
        {
            if (j > num_wheels || num_wheels == 0)
                q->m_suspension_length[j] = 0.0f;
            else
            {
                q->m_suspension_length[j] = kart->getVehicle()
                        ->getWheelInfo(j).m_raycastInfo.m_suspensionLength;
                kart->getVehicle()->getWheelInfo(j).m_raycastInfo.m_suspensionLength = q->m_suspension_length[j];
            }
        }

        kart->getSkidding()->setSkidState(q->m_skidding_state);

        kart->getAttachment()->set(b->m_attachment);
        kart->setEnergy(b->m_nitro_amount);
        kart->getPowerup()->set(b->m_item_type, b->m_item_amount);
	}
}

GameState GameState::copyGameState(){
    return GameState(*this);
}

int GameState::enumToCode(Attachment::AttachmentType type)
{
    int code =
            (type == Attachment::ATTACH_NOTHING)          ? 0 :
            (type == Attachment::ATTACH_PARACHUTE)        ? 1 :
            (type == Attachment::ATTACH_ANVIL)            ? 2 :
            (type == Attachment::ATTACH_BOMB)             ? 3 :
            (type == Attachment::ATTACH_SWATTER)          ? 4 :
            (type == Attachment::ATTACH_BUBBLEGUM_SHIELD) ? 5 :
            -1 ;

    return code;
} // enumToCode

int GameState::enumToCode(PowerupManager::PowerupType type)
{
    int code =
            (type == PowerupManager::POWERUP_NOTHING)    ? 0 :
            (type == PowerupManager::POWERUP_BUBBLEGUM)  ? 1 :
            (type == PowerupManager::POWERUP_CAKE)       ? 2 :
            (type == PowerupManager::POWERUP_BOWLING)    ? 3 :
            (type == PowerupManager::POWERUP_ZIPPER)     ? 4 :
            (type == PowerupManager::POWERUP_PLUNGER)    ? 5 :
            (type == PowerupManager::POWERUP_SWITCH)     ? 6 :
            (type == PowerupManager::POWERUP_SWATTER)    ? 7 :
            (type == PowerupManager::POWERUP_RUBBERBALL) ? 8 :
            (type == PowerupManager::POWERUP_PARACHUTE)  ? 9 :
            -1 ;

    return code;
} // enumToCode
