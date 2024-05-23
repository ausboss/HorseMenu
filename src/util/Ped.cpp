#include "Ped.hpp"

#include "Joaat.hpp"

namespace YimMenu::Peds
{
	int SpawnPed(std::string model_name, Vector3 coords, float heading, bool blockNewPedMovement, bool spawnDead, bool invincible, bool invisible, int scale, bool followPlayer, bool noFleeing, bool isArmed, bool isDrunk, bool ifBodyguard)
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

		if (isDrunk)
		{
			AUDIO::SET_PED_IS_DRUNK(ped, true);
			PED::_SET_PED_DRUNKNESS(ped, true, 1.0f);
		}


		if (isArmed)
		// give the ped the same weapon as the player
		{
			Hash weapon;
			if (WEAPON::GET_CURRENT_PED_WEAPON(YimMenu::Self::PlayerPed, &weapon, true, 0, true))
			{
				WEAPON::GIVE_WEAPON_TO_PED(ped, weapon, 1000, true, true, 0, false, 0.5f, 1.0f, 0, 0, 0.5f, 0);
			}
		}


		if (followPlayer)
		{
			// Create a group and set the player as the leader
			int groupID = PED::CREATE_GROUP(0);
			PED::SET_PED_AS_GROUP_LEADER(YimMenu::Self::PlayerPed, groupID, 0);
			PED::SET_PED_AS_GROUP_MEMBER(ped, groupID);
			PED::SET_PED_CAN_TELEPORT_TO_GROUP_LEADER(ped, groupID, true);
			PED::SET_GROUP_SEPARATION_RANGE(groupID, 999999.9f); // Very high range to prevent separation

			// Set group formation and spacing
			PED::SET_GROUP_FORMATION(groupID, 0); // Default formation
			PED::SET_GROUP_FORMATION_SPACING(groupID, 1.0f, 1.0f, 1.0f);

			// Make the ped follow the player and engage in combat
			TASK::TASK_FOLLOW_TO_OFFSET_OF_ENTITY(ped, YimMenu::Self::PlayerPed, 0.0f, 0.0f, 0.0f, 1.0f, -1, 1.0f, true, false, false, true, false, true);
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, true);
		}

		// If ifBodyguard toggle is enabled
		if (ifBodyguard)
		{
			// Give the ped the same weapon as the player
			Hash weapon;
			if (WEAPON::GET_CURRENT_PED_WEAPON(YimMenu::Self::PlayerPed, &weapon, true, 0, true))
			{
				WEAPON::GIVE_WEAPON_TO_PED(ped, weapon, 1000, true, true, 0, false, 0.5f, 1.0f, 0, 0, 0.5f, 0);
			}

			// Set combat attributes to make the ped defend the player
			PED::SET_PED_COMBAT_ATTRIBUTES(ped, 0, true);  // Enable combat attributes
			PED::SET_PED_COMBAT_ATTRIBUTES(ped, 1, true);  // Enable fighting
			PED::SET_PED_COMBAT_ATTRIBUTES(ped, 46, true); // Enable to attack threats
			PED::SET_PED_COMBAT_MOVEMENT(ped, 2);          // Offensive combat movement
			PED::SET_PED_COMBAT_RANGE(ped, 1);             // Close range
			PED::SET_PED_AS_GROUP_MEMBER(ped, groupID);
			PED::SET_PED_CONFIG_FLAG(ped, 279, true);      // Never leaves group
			PED::REGISTER_TARGET(ped, Global_35, true);    // Register player as target for defense
			PED::SET_PED_ACCURACY(ped, 60);                // Set accuracy
			PED::SET_PED_COMBAT_ATTRIBUTES(ped, 58, true); // Use cover

			// Make the ped attack anyone who attacks the player
			PED::REGISTER_TARGET(ped, YimMenu::Self::PlayerPed, true);
			PED::SET_PED_FLEE_ATTRIBUTES(ped, 0, false);   // Do not flee
			PED::SET_PED_COMBAT_ATTRIBUTES(ped, 46, true); // Always fight back

			// Additional necessary combat settings
			PED::SET_PED_RELATIONSHIP_GROUP_HASH(ped, Joaat("REL_PLAYER_ALLY"));
			int blip = MAP::BLIP_ADD_FOR_ENTITY(Joaat("BLIP_STYLE_ENEMY"), ped);
			MAP::BLIP_ADD_MODIFIER(blip, Joaat("BLIP_MODIFIER_ENEMY_DISAPPEARING"));
			PED::SET_PED_DEFENSIVE_AREA_VOLUME(ped, iLocal_56, false, false, false);
			PED::_0x815C0074A1BC0D93(ped, 0);
			PED::_0x1E017404784AA6A3(ped, Joaat("PLAYER"));
			PED::_SET_PED_COMBAT_BEHAVIOUR(ped, 2018638702);
			int iVar5 = 0;
			func_33(&iVar5, 15);
			func_33(&iVar5, 16);
			COMPANION::_0xDEB369F6AD168C58(ped, iVar5);
		}

		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		return ped;
	};
}
