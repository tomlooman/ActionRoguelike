// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RoguePlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Components/CapsuleComponent.h"
#include "SharedGameplayTags.h"
#include "ActionRoguelike.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "RoguePlayerController.h"
#include "RoguePlayerData.h"
#include "AI/RogueAICharacter.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "UI/RogueWorldUserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePlayerCharacter)


// Sets default values
ARoguePlayerCharacter::ARoguePlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);
	// We control the rotation of spring arm with pawn control rotation already, this disables a subtle side effect
	// where rotating our CapsuleComponent (eg. caused by bOrientRotationToMovement in Character Movement) will rotate our spring arm until it self corrects later in the update
	// This may cause unwanted effects when using CameraLocation during Tick as it may be slightly offset from our final camera position.
	SpringArmComp->SetUsingAbsoluteRotation(true);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	ActionComp = CreateDefaultSubobject<URogueActionComponent>(TEXT("ActionComp"));
	ActionComp->SetDefaultAttributeSet(FRogueSurvivorAttributeSet::StaticStruct());

	PerceptionStimuliComp = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliComp"));

	AttackSoundsComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AttackSoundsComp"));
	AttackSoundsComp->SetupAttachment(RootComponent);
	AttackSoundsComp->bAutoActivate = false;
	// Don't follow player unless actively playing a sound
	AttackSoundsComp->bAutoManageAttachment = true;

	UCharacterMovementComponent* CharMoveComp = GetCharacterMovement();
	CharMoveComp->bUseControllerDesiredRotation = true;

	CharMoveComp->MaxWalkSpeed = 550.0f;
	// Attempt to make the jump feel less floaty
	CharMoveComp->JumpZVelocity = 550;
	CharMoveComp->GravityScale = 1.35f;
	CharMoveComp->BrakingDecelerationFalling = 200.f;

	// Skip performing overlap queries on the Physics Asset after animation (7 queries in case of our Gideon mesh)
	GetMesh()->bUpdateOverlapsOnAnimationFinalize = false;
	
	// Enabled on mesh to react to incoming projectiles
	GetMesh()->SetGenerateOverlapEvents(true);
	// Disable on capsule collision to avoid double-dipping and receiving 2 overlaps when entering trigger zones etc.
	// Once from the mesh, and 2nd time from capsule
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);

	//TimeToHitParamName = "TimeToHit";
	HitFlash_CustomPrimitiveIndex = 0;
}


void ARoguePlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health)->OnAttributeChanged.AddUObject(this, &ThisClass::OnHealthAttributeChanged);
	
	// Cheap trick to disable until we need it in the health event
	CachedOverlayMaxDistance = GetMesh()->OverlayMaterialMaxDrawDistance;
	GetMesh()->SetOverlayMaterialMaxDrawDistance(1);
}


// Called to bind functionality to input
void ARoguePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Must be configured in the Blueprint
	check(PlayerConfig);

	const APlayerController* PC = GetController<APlayerController>();
	const ULocalPlayer* LP = PC->GetLocalPlayer();
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	// Add mappings for our game, more complex games may have multiple Contexts that are added/removed at runtime
	Subsystem->AddMappingContext(PlayerConfig->DefaultInputMapping, 0);

	// Enhanced Input
	UEnhancedInputComponent* InputComp = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// General
	InputComp->BindAction(PlayerConfig->Input_Move, ETriggerEvent::Triggered, this, &ARoguePlayerCharacter::Move);
	InputComp->BindAction(PlayerConfig->Input_Jump, ETriggerEvent::Triggered, this, &ACharacter::Jump);

	// Sprint while key is held
	InputComp->BindAction(PlayerConfig->Input_Sprint, ETriggerEvent::Started, this, &ARoguePlayerCharacter::StartActionByTag, SharedGameplayTags::Action_Sprint.GetTag());
	InputComp->BindAction(PlayerConfig->Input_Sprint, ETriggerEvent::Completed, this, &ARoguePlayerCharacter::StopActionByTag, SharedGameplayTags::Action_Sprint.GetTag());

	// MKB
	InputComp->BindAction(PlayerConfig->Input_LookMouse, ETriggerEvent::Triggered, this, &ARoguePlayerCharacter::LookMouse);
	// Gamepad
	InputComp->BindAction(PlayerConfig->Input_LookStick, ETriggerEvent::Triggered, this, &ARoguePlayerCharacter::LookStick);

	// Abilities
	InputComp->BindAction(PlayerConfig->Input_PrimaryAttack, ETriggerEvent::Triggered, this, &ARoguePlayerCharacter::StartActionByTag, SharedGameplayTags::Action_PrimaryAttack.GetTag());
	InputComp->BindAction(PlayerConfig->Input_SecondaryAttack, ETriggerEvent::Triggered, this, &ARoguePlayerCharacter::StartActionByTag, SharedGameplayTags::Action_Blackhole.GetTag());
	InputComp->BindAction(PlayerConfig->Input_Dash, ETriggerEvent::Triggered, this, &ARoguePlayerCharacter::StartActionByTag, SharedGameplayTags::Action_Dash.GetTag());
}

void ARoguePlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FindCrosshairTarget();
}


void ARoguePlayerCharacter::FindCrosshairTarget()
{
	// Ignore if not using GamePad
	ARoguePlayerController* PC = GetController<ARoguePlayerController>();

	// Only use aim assist when currently controlled and using gamepad
	// Note: you *may* always want to line trace if using this result for other things like coloring crosshair or re-using this hit data for aim adjusting during projectile attacks
	if (PC == nullptr || !PC->IsUsingGamepad())
	{
		bHasPawnTarget = false;
		return;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const float AimAssistDistance = 5000.f;
	const FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * AimAssistDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FCollisionShape Shape;
	Shape.SetSphere(50.f);

	// Called next frame when the trace has completed
	FTraceDelegate Delegate = FTraceDelegate::CreateUObject(this, &ARoguePlayerCharacter::CrosshairTraceComplete);
	
	TraceHandle = GetWorld()->AsyncSweepByChannel(EAsyncTraceType::Single, EyeLocation, TraceEnd, FQuat::Identity, ECC_Pawn, Shape, Params, FCollisionResponseParams::DefaultResponseParam, &Delegate);
}


void ARoguePlayerCharacter::CrosshairTraceComplete(const FTraceHandle& InTraceHandle, FTraceDatum& InTraceDatum)
{
	// at most expect one hit
	if (InTraceDatum.OutHits.IsValidIndex(0))
	{
		FHitResult Hit = InTraceDatum.OutHits[0];
		// Figure out if dealing with a Pawn, may want aim assist on other 'things', which requires a different check
		bHasPawnTarget = Hit.IsValidBlockingHit() && Hit.GetActor()->IsA(APawn::StaticClass());

		//UE_LOG(LogGame, Log, TEXT("has pawn target: %s"), bHasPawnTarget ? TEXT("true") : TEXT("false"));
	}
}


void ARoguePlayerCharacter::ClientOnSeenBy_Implementation(ARogueAICharacter* SeenByPawn)
{
	URogueWorldUserWidget* NewWidget = CreateWidget<URogueWorldUserWidget>(GetWorld(), SpottedWidgetClass);
	// Can be nullptr if we do not specify a class to use in Blueprint
	if (NewWidget)
	{
		NewWidget->AttachedActor = SeenByPawn;
		URogueWorldUserWidget::AddToRootCanvasPanel(NewWidget);
	}
}

void ARoguePlayerCharacter::PlayAttackSound(USoundBase* InSound)
{
	// This may interrupt previously playing sounds, so you'd want to test for this
	AttackSoundsComp->SetSound(InSound);
	AttackSoundsComp->Play();
}

void ARoguePlayerCharacter::Move(const FInputActionInstance& Instance)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	// Get value from input (combined value from WASD keys or single Gamepad stick) and convert to Vector (x,y)
	const FVector2D AxisValue = Instance.GetValue().Get<FVector2D>();

	// Move forward/back
	AddMovementInput(ControlRot.Vector(), AxisValue.Y);

	// Move Right/Left strafe
	const FVector RightVector = ControlRot.RotateVector(FVector::RightVector);
	AddMovementInput(RightVector, AxisValue.X);
}

void ARoguePlayerCharacter::LookMouse(const FInputActionValue& InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();
	
	AddControllerYawInput(Value.X);
	AddControllerPitchInput(Value.Y);
}


void ARoguePlayerCharacter::LookStick(const FInputActionValue& InputValue)
{
	FVector2D Value = InputValue.Get<FVector2D>();

	// Track negative as we'll lose this during the conversion
	bool XNegative = Value.X < 0.f;
	bool YNegative = Value.Y < 0.f;

	// Can further modify with 'sensitivity' settings
	const float LookYawRate = 100.0f;
	const float LookPitchRate = 50.0f;

	// non-linear to make aiming a little easier
	Value = Value * Value;

	if (XNegative)
	{
		Value.X *= -1.f;
	}
	if (YNegative)
	{
		Value.Y *= -1.f;
	}

	// Aim assist
	// todo: may need to ease this out and/or change strength based on distance to target
	float RateMultiplier = 1.0f;
	if (bHasPawnTarget)
	{
		RateMultiplier = 0.5f;
	}

	AddControllerYawInput(Value.X * (LookYawRate * RateMultiplier) * GetWorld()->GetDeltaSeconds());
	AddControllerPitchInput(Value.Y * (LookPitchRate * RateMultiplier) * GetWorld()->GetDeltaSeconds());
}


void ARoguePlayerCharacter::StartActionByTag(const FInputActionValue& Instance, const FGameplayTag InActionTag)
{
	ActionComp->StartActionByName(this, InActionTag);
}


void ARoguePlayerCharacter::StopActionByTag(const FInputActionValue& Instance, const FGameplayTag InActionTag)
{
	ActionComp->StopActionByName(this, InActionTag);
}


void ARoguePlayerCharacter::OnHealthAttributeChanged(float NewValue, const FAttributeModification& AttributeModification)
{
	// Damaged
	if (AttributeModification.Magnitude < 0.0f)
	{
		// Materials, including the mesh "OverlayMaterial" can get their data via the component
		GetMesh()->SetCustomPrimitiveDataFloat(HitFlash_CustomPrimitiveIndex, GetWorld()->TimeSeconds);

		// Activate, we can skip rendering this at a distance
		GetMesh()->SetOverlayMaterialMaxDrawDistance(CachedOverlayMaxDistance);

		// After 1.0seconds we should be finished with the hitflash (re-use the handle to reset timer if we get hit again)
		GetWorldTimerManager().SetTimer(OverlayTimerHandle, [this]()
		{
			// Cheap trick to skip rendering this all the time unless we are actively hit flashing
			GetMesh()->SetOverlayMaterialMaxDrawDistance(1);
		}, 1.0f, false);

		// Rage added equal to damage received (Abs to turn into positive rage number)
		// @TODO: change behavior of RAGE mechanic
		const float RageDelta = FMath::Abs(AttributeModification.Magnitude);
		ActionComp->ApplyAttributeChange(SharedGameplayTags::Attribute_Rage, RageDelta, this, EAttributeModifyType::AddModifier);

		UGameplayStatics::PlaySoundAtLocation(this, TakeDamageVOSound, GetActorLocation(), FRotator::ZeroRotator);
	}

	// Died
	if (NewValue <= 0.0f && AttributeModification.Magnitude < 0.0f)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathVOSound, GetActorLocation(), FRotator::ZeroRotator);

		PlayAnimMontage(DeathMontage);

		SetLifeSpan(5.0f);

		// Prevent bots from seeing us as a threat
		PerceptionStimuliComp->UnregisterFromPerceptionSystem();
		
		APlayerController* PC = GetController<ARoguePlayerController>();
		if (PC && PC->IsLocalController())
		{
			// for local player, play a "UI" sound on death
			UGameplayStatics::PlaySound2D(this, DeathUISound);

			DisableInput(PC);
		}
	}
}


FGenericTeamId ARoguePlayerCharacter::GetGenericTeamId() const
{
	// We have no team switching support during gameplay
	return FGenericTeamId(TEAM_ID_PLAYERS);
}


FVector ARoguePlayerCharacter::GetPawnViewLocation() const
{
	return CameraComp->GetComponentLocation();
}
