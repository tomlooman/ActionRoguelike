// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAction_DropLoot.h"

#include "NavigationSystem.h"
#include "RogueActionSystemComponent.h"
#include "SharedGameplayTags.h"
#include "AI/RogueAICharacter.h"
#include "AI/RogueMonsterData.h"
#include "Pickups/RogueCoinPickupSubsystem.h"

URogueAction_DropLoot::URogueAction_DropLoot()
{
	ActionName = SharedGameplayTags::Action_DropLoot;
}

void URogueAction_DropLoot::StartAction_Implementation()
{
	Super::StartAction_Implementation();
	
	TArray<FVector> CoinLocations;
	TArray<int32> CoinAmounts;
	
	ARogueAICharacter* AIPawn = Cast<ARogueAICharacter>(GetOwningComponent()->GetOwner());
	check(AIPawn);
	
	// Assume standard value for pre-placed enemies that don't currently have a MonsterData assigned
	int32 RemainingCreditsToDrop = 10;//AIPawn->GetActionSystemComponent()->GetAttributeValue(SharedGameplayTags::Attribute_Coins);

	if (URogueMonsterData* MonsterData = AIPawn->GetMonsterData())
	{
		RemainingCreditsToDrop = MonsterData->LootCoins;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(this);
	FVector ActorLocation = GetOwningComponent()->GetOwner()->GetActorLocation();
	FRandomStream RandomStream = FRandomStream(GetWorld()->TimeSeconds);

	while (RemainingCreditsToDrop > 0)
	{		
		FNavLocation NavLocation;
		NavSystem->GetRandomPointInNavigableRadius(ActorLocation, MaxDropDistance, NavLocation);
		
		int32 CreditsPerCoin = RandomStream.RandRange(1, MaxCreditsPerCoin);
		CreditsPerCoin = FMath::Clamp(CreditsPerCoin, 1, RemainingCreditsToDrop);

		CoinLocations.Add(NavLocation.Location);
		CoinAmounts.Add(CreditsPerCoin);
		
		RemainingCreditsToDrop -= CreditsPerCoin;
	}

	URogueCoinPickupSubsystem* CoinSystem = GetWorld()->GetSubsystem<URogueCoinPickupSubsystem>();
	CoinSystem->AddCoinPickups(CoinLocations, CoinAmounts);
}
