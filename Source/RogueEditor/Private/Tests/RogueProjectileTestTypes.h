#pragma once

#include "ActionSystem/RogueAction_ProjectileAttack.h"
#include "GameFramework/GameModeBase.h"
#include "Player/RoguePlayerCharacter.h"
#include "RogueProjectileTestTypes.generated.h"

/* Test fixtures belong to the Editor module, including reflected classes used by network peers. */
UCLASS()
class URogueProjectileTestAttack : public URogueAction_ProjectileAttack
{
	GENERATED_BODY()

public:
	URogueProjectileTestAttack();
	void Configure(URogueProjectileData* Config, float Delay);
	int32 GetCosmeticCount() const { return CosmeticCount; }

protected:
	virtual void PlayActionCosmetics() override;
	int32 CosmeticCount = 0;
};

UCLASS()
class ARogueProjectileTestCharacter : public ARoguePlayerCharacter
{
	GENERATED_BODY()

public:
	ARogueProjectileTestCharacter();
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
};

UCLASS()
class ARogueProjectileTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARogueProjectileTestGameMode();
};
