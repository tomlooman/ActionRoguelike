// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoguePlayerData.generated.h"

class URogueAction;
class UInputAction;
class ARoguePlayerCharacter;
class UInputMappingContext;


/**
 * Data Configuration for Player including pawn class, abilities and input classes
 */
UCLASS()
class ACTIONROGUELIKE_API URoguePlayerData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	//UPROPERTY(EditDefaultsOnly, Category="CharacterClass")
	//TSubclassOf<ARoguePlayerCharacter> CharacterClass; 
	
	//UPROPERTY(EditDefaultsOnly, Category="Actions")
	//TArray<TSubclassOf<URogueAction>> DefaultActions;

	// -- Input Actions -- //

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultInputMapping;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_Move;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_LookMouse;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_LookStick;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_Jump;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_Sprint;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_Dash;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_PrimaryAttack;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_SecondaryAttack;

};
