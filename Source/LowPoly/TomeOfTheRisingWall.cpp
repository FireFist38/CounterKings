#include "TomeOfTheRisingWall.h"
#include "SpellRisingWall.h"

ATomeOfTheRisingWall::ATomeOfTheRisingWall()
{
	ItemType = EItemType::Magic;
	CrosshairStyle = ECrosshairStyle::Offhand;
	ActionName = NSLOCTEXT("Skill", "RisingWall", "Rising Wall");
	TomeUseMode = ETomeUseMode::Instant;
	bUseTomeSpellObject = true;
	bRequireMagicMainHandForSpell = false;
	bUseSpellCooldown = true;
	bActivationUsesMana = true;

	USpellRisingWall* RisingWallSpell = CreateDefaultSubobject<USpellRisingWall>(TEXT("RisingWallSpell"));
	TomeSpell = RisingWallSpell;
}
