#include "Ped.hpp"
#include "Joaat.hpp"
#include "game/rdr/Natives.hpp"  // Ensure this includes TASK and PED functions

namespace YimMenu::Peds
{
    // Returns 0 if it fails
    int SpawnPed(std::string model_name, Vector3 coords, float heading, bool blockNewPedMovement, bool spawnDead, bool invincible, bool invisible, int scale, bool followPlayer, bool nonFleeing, bool engageInCombat)
    {
        Hash model = Joaat(model_name.c_str());

        if (!STREAMING::IS_MODEL_IN_CDIMAGE(model) || !STREAMING::IS_MODEL_VALID(model))
        {
            Notifications::Show("Spawner", "Invalid ped model", NotificationType::Error);
            return 0;
        }

        for (int i = 0; i < 30 && !STREAMING::HAS_MODEL_LOADED(model); i++)
        {
            STREAMING::REQUEST_MODEL(model, false);
            ScriptMgr::Yield();
        }

        auto ped = PED::CREATE_PED(model, coords.x, coords.y, coords.z, heading, 1, 0, 0, 0);

        PED::_SET_RANDOM_OUTFIT_VARIATION(ped, true);
        ENTITY::PLACE_ENTITY_ON_GROUND_PROPERLY(ped, true);

        ENTITY::FREEZE_ENTITY_POSITION(ped, blockNewPedMovement);
        ENTITY::SET_ENTITY_INVINCIBLE(ped, invincible);
        ENTITY::SET_ENTITY_VISIBLE(ped, !invisible);
        PED::_SET_PED_SCALE(ped, (float)scale);

        if (spawnDead)
            PED::APPLY_DAMAGE_TO_PED(ped, std::numeric_limits<int>::max(), 1, 0, YimMenu::Self::PlayerPed);

        if (followPlayer)
        {
            // Make the ped follow the player
            TASK::TASK_FOLLOW_TO_OFFSET_OF_ENTITY(ped, YimMenu::Self::PlayerPed, 0, 0, 0, 1.0f, -1, 1.0f, true, false, false, false, false, false);
        }

        if (nonFleeing)
        {
            // Set the ped to not flee
            PED::SET_PED_COMBAT_ATTRIBUTES(ped, 46, true);  // Always fight
            PED::SET_PED_FLEE_ATTRIBUTES(ped, 0, false);    // Disable fleeing
            PED::SET_PED_COMBAT_ATTRIBUTES(ped, 5, true);   // Can fight armed peds
        }

        if (engageInCombat)
        {
            // Engage the ped in combat with nearby enemies
            PED::SET_PED_COMBAT_ATTRIBUTES(ped, 5, true);   // Can fight armed peds
            TASK::TASK_COMBAT_HATED_TARGETS_AROUND_PED(ped, 100.0f, 0);
        }

        STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
        return ped;
    }
}
