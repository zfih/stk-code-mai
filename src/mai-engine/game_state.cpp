//
// Created by zfih on 06/11/2019.
//

#include "game_state.hpp"

GameState::GameState(){
    //m_world = World::getWorld();
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

        // If one of the tracked kart data has significantly changed
        // for the kart, update sooner than the usual dt
        bool force_update = false;

        // Don't save directly the enum value, because any change
        // to it would break the reading of old replays
        int attachment = enumToCode(kart->getAttachment()->getType());
        int powerup_type = enumToCode(kart->getPowerup()->getType());
        int special_value = 0;

        if (attachment == -1)
        {
            Log::error("ReplayRecorder", "Unknown attachment type");
            return;
        }
        if (powerup_type == -1)
        {
            Log::error("ReplayRecorder", "Unknown powerup type");
            return;
        }

        if (m_count_transforms[i] >= 2)
        {
            BonusInfo *b_prev       = &(m_bonus_info[i][m_count_transforms[i]-1]);
            BonusInfo *b_prev2      = &(m_bonus_info[i][m_count_transforms[i]-2]);
            PhysicInfo *q_prev      = &(m_physic_info[i][m_count_transforms[i]-1]);

            // If the kart changes its steering
            if (fabsf(kart->getControls().getSteer() - m_previous_steer) >
                stk_config->m_replay_delta_steering)
                force_update = true;

            // If the kart starts or stops skidding
            if (kart->getSkidding()->getSkidState() != q_prev->m_skidding_state)
                force_update = true;
            // If the kart changes speed significantly
            float speed_change = fabsf(kart->getSpeed() - q_prev->m_speed);
            if ( speed_change > stk_config->m_replay_delta_speed )
            {
                if (speed_change > 4*stk_config->m_replay_delta_speed)
                    force_update = true;
                else if (speed_change > 2*stk_config->m_replay_delta_speed &&
                         time - m_last_saved_time[i] > (stk_config->m_replay_dt/8.0f))
                    force_update = true;
                else if (time - m_last_saved_time[i] > (stk_config->m_replay_dt/3.0f))
                    force_update = true;
            }

            // If the attachment has changed
            if (attachment != b_prev->m_attachment)
                force_update = true;

            // If the item amount has changed
            if (kart->getNumPowerup() != b_prev->m_item_amount)
                force_update = true;

            // If the item type has changed
            if (powerup_type != b_prev->m_item_type)
                force_update = true;

            // In egg-hunt mode, if an egg has been collected
            // In battle mode, if a live has been lost/gained
            if (special_value != b_prev->m_special_value)
                force_update = true;

            // If nitro starts being used or is collected
            if (kart->getEnergy() != b_prev->m_nitro_amount &&
                b_prev->m_nitro_amount == b_prev2->m_nitro_amount)
                force_update = true;

            // If nitro stops being used
            // (also generates an extra transform on collection,
            //  should be negligible and better than heavier checks)
            if (kart->getEnergy() == b_prev->m_nitro_amount &&
                b_prev->m_nitro_amount != b_prev2->m_nitro_amount)
                force_update = true;

            // If close to the end of the race, reduce the time step
            // for extra precision
            // TODO : fast updates when close to the last egg in egg hunt
            if (race_manager->isLinearRaceMode())
            {
                float full_distance;
                full_distance = race_manager->getNumLaps()
                                * Track::getCurrentTrack()->getTrackLength();

                const LinearWorld *linearworld = dynamic_cast<LinearWorld*>(World::getWorld());
                if (full_distance + DISTANCE_MAX_UPDATES >= linearworld->getOverallDistance(i) &&
                    full_distance <= linearworld->getOverallDistance(i) + DISTANCE_FAST_UPDATES)
                {
                    if (fabsf(full_distance - linearworld->getOverallDistance(i)) < DISTANCE_MAX_UPDATES)
                        force_update = true;
                    else if (time - m_last_saved_time[i] > (stk_config->m_replay_dt/5.0f))
                        force_update = true;
                }
            }
        }


        if ( time - m_last_saved_time[i] < (stk_config->m_replay_dt - stk_config->ticks2Time(1)) &&
             !force_update)
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

        b->m_attachment        = attachment;
        b->m_nitro_amount      = kart->getEnergy();
        b->m_item_amount       = kart->getNumPowerup();
        b->m_item_type         = powerup_type;
        b->m_special_value     = special_value;

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

    // TODO: Set current state to this state
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
