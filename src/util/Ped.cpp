#include "Ped.hpp"

#include "Joaat.hpp"

namespace YimMenu::Peds
{
	void MarkPedAsCompanion(Ped ped)
	{
		Hash playerGroup = GAMEPLAY::GET_HASH_KEY((char*)"PLAYER");
		Hash companionGroup;

		PED::ADD_RELATIONSHIP_GROUP((char*)"_CHAOS_COMPANION", &companionGroup);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(0, companionGroup, playerGroup);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(0, playerGroup, companionGroup);

		PED::SET_PED_RELATIONSHIP_GROUP_HASH(ped, companionGroup);

		PED::SET_PED_AS_GROUP_MEMBER(ped, PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID()));

		/** BF_CanFightArmedPedsWhenNotArmed */
		PED::SET_PED_COMBAT_ATTRIBUTES(ped, 5, true);
		/** BF_AlwaysFight */
		PED::SET_PED_COMBAT_ATTRIBUTES(ped, 46, true);

		static Hash blipHash = GET_HASH("BLIP_STYLE_FRIENDLY");

		/** BLIP_ADD_FOR_ENTITY */
		Blip blip = RADAR::_0x23F74C2FDA6E7C61(blipHash, ped);

		static Hash blipModifier = GET_HASH("BLIP_MODIFIER_MP_COLOR_1");

		/** BLIP_ADD_MODIFIER */
		RADAR::_0x662D364ABF16DE2F(blip, blipModifier);

		/** Allow talk */
		PED::SET_PED_CONFIG_FLAG(ped, 130, 0);
	}

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
		{
			Hash weapon;
			if (WEAPON::GET_CURRENT_PED_WEAPON(YimMenu::Self::PlayerPed, &weapon, true, 0, true))
			{
				WEAPON::GIVE_WEAPON_TO_PED(ped, weapon, 1000, true, true, 0, false, 0.5f, 1.0f, 0, 0, 0.5f, 0);
			}
		}

		if (followPlayer)
		{
			int groupID = PED::CREATE_GROUP(0);
			PED::SET_PED_AS_GROUP_LEADER(YimMenu::Self::PlayerPed, groupID, 0);
			PED::SET_PED_AS_GROUP_MEMBER(ped, groupID);
			PED::SET_PED_CAN_TELEPORT_TO_GROUP_LEADER(ped, groupID, true);
			PED::SET_GROUP_SEPARATION_RANGE(groupID, 999999.9f);

			PED::SET_GROUP_FORMATION(groupID, 0);
			PED::SET_GROUP_FORMATION_SPACING(groupID, 1.0f, 1.0f, 1.0f);

			TASK::TASK_FOLLOW_TO_OFFSET_OF_ENTITY(ped, YimMenu::Self::PlayerPed, 0.0f, 0.0f, 0.0f, 1.0f, -1, 1.0f, true, false, false, true, false, true);
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, true);
		}

		if (ifBodyguard)
		{
			MarkPedAsCompanion(ped);
		}

		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		return ped;
	}
}
