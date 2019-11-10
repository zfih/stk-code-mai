//
// Created by zfih on 06/11/2019.
//

#ifndef SUPERTUXKART_GAME_STATE_HPP
#define SUPERTUXKART_GAME_STATE_HPP

#include "modes/world.hpp"
#include <modes/linear_world.hpp>
#include <modes/easter_egg_hunt.hpp>
#include "karts/abstract_kart.hpp"
#include "karts/skidding.hpp"
#include "karts/kart_gfx.hpp"
#include "physics/btKart.hpp"
#include "tracks/track.hpp"
#include "items/powerup_manager.hpp"
#include "items/powerup.hpp"
#include "items/attachment.hpp"

class GameState {
public: // everything is public, this isn't pretty, but convenient.
    /** Stores a transform event, i.e. a position and rotation of a kart
     *  at a certain time. */
    struct TransformEvent
    {
        /** Time at which this event happens. */
        float               m_time;
        /** The transform at a certain time. */
        btTransform         m_transform;
    };   // TransformEvent

    // ------------------------------------------------------------------------
    struct PhysicInfo
    {
        /** The speed at a certain time. */
        float               m_speed;
        /** The steering at a certain time. */
        float               m_steer;
        /** The suspension length of 4 wheels at a certain time. */
        float               m_suspension_length[4];
        /** The skidding state */
        Skidding::SkidState m_skidding_state;
    };   // PhysicInfo

    // ------------------------------------------------------------------------
    struct BonusInfo
    {
        /** The attachment. This is stored using a custom format
            0 = none ; 1 = parachute ; 2 = anvil ; 3 = bomb ;
            4 = swatter ; 5 = bubblegum
            This is necessary so replay files are not broken if the
            game internal attachment format/ordering is changed. */
        Attachment::AttachmentType m_attachment;
        /** The nitro amount at a certain time. */
        float               m_nitro_amount;
        /** The number of items at a certain time. */
        int                 m_item_amount;
        /** The type of item at a certain time. */
        PowerupManager::PowerupType m_item_type;
        /** Used to store mode-specific values : eggs in egg-hunt,
            number of lives in battle-mode. */
        int                 m_special_value;
    };   // StateInfo


    // ------------------------------------------------------------------------
    /** Records all other events. */
    struct KartReplayEvent
    {
        /** distance on track for the kart recorded. */
        float  m_distance;
        /** Nitro usage for the kart recorded. */
        int    m_nitro_usage;
        /** Zipper usage for the kart recorded. */
        bool   m_zipper_usage;
        /** Skidding effect for the kart recorded. */
        int    m_skidding_effect;
        /** Kart skidding showing red flame or not. */
        bool   m_red_skidding;
        /** True if the kart recorded is jumping. */
        bool        m_jumping;
    };   // KartReplayEvent

    World *m_world;

    /** A separate vector of Replay Events for all transforms. */
    std::vector< std::vector<TransformEvent> > m_transform_events;

    /** A separate vector of Replay Events for all physic info. */
    std::vector< std::vector<PhysicInfo> > m_physic_info;

    /** A separate vector of Replay Events for all item/nitro info. */
    std::vector< std::vector<BonusInfo> > m_bonus_info;

    /** A separate vector of Replay Events for all other events. */
    std::vector< std::vector<KartReplayEvent> > m_kart_replay_event;

    /** Time at which a transform was saved for the last time. */
    std::vector<float> m_last_saved_time;

    /** Counts the number of transform events for each kart. */
    std::vector<unsigned int> m_count_transforms;

    bool  m_complete_replay{};

    bool  m_incorrect_replay{};

    unsigned int m_max_frames{};

    // Stores the steering value at the previous transform.
    // Used to trigger the recording of new transforms.
    float m_previous_steer = 0.0f;

    const float DISTANCE_FAST_UPDATES = 10.0f;

    const float DISTANCE_MAX_UPDATES = 1.0f;


    // functions
    GameState();
    GameState(const GameState &state);
    ~GameState();

    void init();
    void reset();
    void update();
    void makeStateCurrentState();

    GameState copyGameState();

    int enumToCode(Attachment::AttachmentType type);

    int enumToCode(PowerupManager::PowerupType type);
};


#endif //SUPERTUXKART_GAME_STATE_HPP
