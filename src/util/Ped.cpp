#include "Ped.hpp"

#include "Joaat.hpp"
#include "game/rdr/Natives.hpp" // Ensure this includes TASK and PED functions

namespace YimMenu::Peds
{
	int SpawnPed(std::string model_name, Vector3 coords, float heading, bool blockNewPedMovement, bool spawnDead, bool invincible, bool invisible, int scale, bool followPlayer, bool noFleeing)
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

		if (noFleeing)
		{
			PED::SET_PED_COMBAT_ATTRIBUTES(ped, 46, true); // Always fight
			PED::SET_PED_FLEE_ATTRIBUTES(ped, 0, false);   // Disable all fleeing
			PED::SET_PED_COMBAT_RANGE(ped, 0);             // Set combat range to near
			PED::SET_PED_COMBAT_MOVEMENT(ped, 2);          // Set combat movement to advanced
			PED::SET_PED_COMBAT_ATTRIBUTES(ped, 1, PED::IS_PED_IN_ANY_VEHICLE(ped, 0));  // Fight in vehicle
			PED::SET_PED_COMBAT_ATTRIBUTES(ped, 3, !PED::IS_PED_IN_ANY_VEHICLE(ped, 0)); // Fight on foot
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, true);                        // Block non-temporary events
			PED::SET_PED_COMBAT_ABILITY(ped, 2);                                         // High combat ability
		}

		if (spawnDead)
			PED::APPLY_DAMAGE_TO_PED(ped, std::numeric_limits<int>::max(), 1, 0, YimMenu::Self::PlayerPed);

		if (followPlayer)
		{
			// Create a group and set the player as the leader
			int groupID = PED::CREATE_GROUP(0);
			PED::SET_PED_AS_GROUP_LEADER(YimMenu::Self::PlayerPed, groupID);
			PED::SET_PED_AS_GROUP_MEMBER(ped, groupID);
			PED::SET_PED_CAN_TELEPORT_TO_GROUP_LEADER(ped, groupID, true);
			PED::SET_GROUP_SEPARATION_RANGE(groupID, 999999.9f); // Very high range to prevent separation

			// Set group formation and spacing
			PED::SET_GROUP_FORMATION(groupID, 0); // Default formation
			PED::SET_GROUP_FORMATION_SPACING(groupID, 1.0f, 1.0f, 1.0f);

			// Make the ped follow the player and engage in combat
			TASK::TASK_FOLLOW_TO_OFFSET_OF_ENTITY(ped, YimMenu::Self::PlayerPed, 0.0f, 0.0f, 0.0f, 1.0f, -1, 1.0f, true, false, false, true, false, true);
			TASK::TASK_COMBAT_HATED_TARGETS_AROUND_PED(ped, 50.0f, 0);
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, true);
		}

		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		return ped;
	}
}
