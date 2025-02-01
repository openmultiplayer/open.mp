#pragma once
#include <sdk.hpp>

inline bool canWeaponBeDoubleHanded(uint8_t weapon)
{
	switch (weapon)
	{
	case PlayerWeapon_Colt45:
	case PlayerWeapon_Sawedoff:
	case PlayerWeapon_UZI:
	case PlayerWeapon_TEC9:
		return true;
	}

	return false;
}

inline bool isWeaponDoubleHanded(uint8_t weapon, int skillLevel)
{
	return canWeaponBeDoubleHanded(weapon) && skillLevel > 999;
}

inline PlayerWeaponSkill getWeaponSkillID(uint8_t weapon)
{
	static PlayerWeaponSkill skills[] = {
		PlayerWeaponSkill_Pistol, // 22
		PlayerWeaponSkill_SilencedPistol, // 23
		PlayerWeaponSkill_DesertEagle, // 24
		PlayerWeaponSkill_Shotgun, // 25
		PlayerWeaponSkill_SawnOff, // 26
		PlayerWeaponSkill_SPAS12, // 27
		PlayerWeaponSkill_Uzi, // 28
		PlayerWeaponSkill_MP5, // 29
		PlayerWeaponSkill_AK47, // 30
		PlayerWeaponSkill_M4, // 31
		PlayerWeaponSkill_Uzi, // 32
		PlayerWeaponSkill_Sniper, // 33
		PlayerWeaponSkill_Sniper // 34
	};

	if (!WeaponSlotData(weapon).shootable())
	{
		return PlayerWeaponSkill_Invalid;
	}

	return skills[weapon - 22];
}

inline int getWeaponActualClipSize(uint8_t weapon, int currentAmmo, int weaponSkillLevel, bool hasInfiniteAmmo)
{
	auto data = WeaponInfo::get(weapon);
	if (data.type != PlayerWeaponType_None)
	{
		int size = data.clipSize;
		if (isWeaponDoubleHanded(weapon, weaponSkillLevel))
		{
			size *= 2;
		}

		if (currentAmmo < size && !hasInfiniteAmmo)
		{
			size = currentAmmo;
		}

		return size;
	}
	return 0;
}
