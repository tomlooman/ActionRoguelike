#include "RogueProjectileTestTypes.h"

#include "ActionRoguelike.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/RogueGameState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "KismetCompiler.h"
#include "Misc/AutomationTest.h"
#include "Projectiles/RogueProjectilesSubsystem.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"

URogueProjectileTestAttack::URogueProjectileTestAttack()
{
	ActivationTag = FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack"));
	AttackAnimDelay = 0.1f;
	CooldownTime = 0.1f;
	HandSocketName = NAME_None;
	static ConstructorHelpers::FObjectFinder<URogueProjectileData> Config(TEXT("/Game/ActionRoguelike/Projectiles/DA_ProjectileConfigAI"));
	ProjectileConfig = Config.Object;
}


void URogueProjectileTestAttack::Configure(URogueProjectileData* Config, float Delay)
{
	ProjectileConfig = Config;
	AttackAnimDelay = Delay;
}


void URogueProjectileTestAttack::PlayActionCosmetics()
{
	++CosmeticCount;
}


ARogueProjectileTestCharacter::ARogueProjectileTestCharacter()
{
	GetCharacterMovement()->GravityScale = 0;
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void ARogueProjectileTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// The fixture drives actions directly and needs no production input/character assets.
}


void ARogueProjectileTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		ActionComp->AddAction(this, URogueProjectileTestAttack::StaticClass());
	}
}


ARogueProjectileTestGameMode::ARogueProjectileTestGameMode()
{
	GameStateClass = ARogueGameState::StaticClass();
	DefaultPawnClass = ARogueProjectileTestCharacter::StaticClass();
	PlayerControllerClass = APlayerController::StaticClass();
}


#if WITH_DEV_AUTOMATION_TESTS
namespace
{
	struct FProjectileTestWorld
	{
		UWorld* World;
		ARogueGameState* GameState;
		URogueProjectilesSubsystem* Subsystem;
		URogueProjectileData* Config;

		FProjectileTestWorld()
		{
			const UWorld::InitializationValues Values = UWorld::InitializationValues().AllowAudioPlayback(false).RequiresHitProxies(false)
				.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
			World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			GameState = World->SpawnActor<ARogueGameState>();
			World->SetGameState(GameState);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
			World->GetWorldSettings()->NotifyBeginPlay();
			Subsystem = World->GetSubsystem<URogueProjectilesSubsystem>();
			Config = NewObject<URogueProjectileData>(World);
			Config->InitialSpeed = 1000;
			Config->Lifespan = 2;
		}

		~FProjectileTestWorld()
		{
			World->EndPlay(EEndPlayReason::Quit);
			World->DestroyWorld(false);
			GEngine->DestroyWorldContext(World);
		}

		void Advance(float Time)
		{
			for (float Elapsed = 0; Elapsed < Time; Elapsed += 0.02f)
			{
				// TimerManager only ticks once per engine frame, including manually advanced test worlds.
				++GFrameCounter;
				World->Tick(LEVELTICK_All, 0.02f);
			}
		}

		FProjectileLaunch Launch(int64 ID, FProjectilePredictionKey Key = {}) const
		{
			FProjectileLaunch Result;
			Result.ID = ID;
			Result.PredictionKey = Key;
			Result.Position = FVector(0, 0, 1000);
			Result.Velocity = FVector(1000, 0, 0);
			Result.SimulationTime = GameState->GetServerWorldTimeSeconds();
			Result.ExpirationTime = Result.SimulationTime + 2;
			Result.ConfigDataAsset = Config;
			return Result;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileReconciliationTest, "Orion.Projectiles.Reconciliation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectileReconciliationTest::RunTest(const FString& Parameters)
{
	FProjectileTestWorld Test;
	FProjectilePredictionKey A;
	A.Session = 1;
	A.Sequence = 1;
	FProjectilePredictionKey B = A;
	B.Session = 2;
	Test.Subsystem->PredictProjectile(FVector(0, 0, 1000), FVector::ForwardVector, Test.Config, nullptr, A, nullptr);
	Test.Subsystem->PredictProjectile(FVector(0, 0, 1000), FVector::ForwardVector, Test.Config, nullptr, B, nullptr);
	const FProjectileLaunch LaunchA = Test.Launch(10, A);
	const FProjectileLaunch LaunchB = Test.Launch(11, B);
	Test.Subsystem->ReceiveLaunch(LaunchB);
	Test.Subsystem->ReceiveLaunch(LaunchA);
	Test.Subsystem->ReceiveLaunch(LaunchA);
	TestEqual(TEXT("Two shooters' identical sequence numbers stay separate"), Test.Subsystem->GetActiveProjectileCount(), 2);
	TestNotNull(TEXT("First prediction promoted"), Test.Subsystem->FindProjectile(10));
	TestNotNull(TEXT("Second prediction promoted"), Test.Subsystem->FindProjectile(11));
	TestEqual(TEXT("Client handlers never insert into authoritative FastArray"), Test.GameState->ProjectileData.Items.Num(), 0);
	Test.Advance(0.2f);
	FProjectileLaunch Correction = LaunchB;
	Correction.Position.X = 500;
	Correction.SimulationTime = Test.GameState->GetServerWorldTimeSeconds() - 0.1f;
	Test.Subsystem->ReceiveLaunch(Correction);
	const FProjectileInstance* Corrected = Test.Subsystem->FindProjectile(11);
	TestTrue(TEXT("Snapshot advances from its own timestamp, not full launch age"), Corrected && FMath::IsNearlyEqual(Corrected->Position.X, 600.0f, 0.01f));
	Test.Subsystem->ReceiveLaunch(Correction);
	Corrected = Test.Subsystem->FindProjectile(11);
	TestTrue(TEXT("Repeated snapshot does not advance a second time"), Corrected && FMath::IsNearlyEqual(Corrected->Position.X, 600.0f, 0.01f));

	FProjectileUpdate Impact;
	Impact.Projectile = LaunchA;
	Impact.Outcome = EProjectileOutcome::Impacted;
	Test.Subsystem->ReceiveRemoval(LaunchA);
	Test.Subsystem->ReceiveUpdate(Impact);
	Test.Subsystem->ReceiveUpdate(Impact);
	Test.Subsystem->ReceiveLaunch(LaunchA);
	TestEqual(TEXT("Removal then duplicate impacts consume one effect"), Test.Subsystem->GetImpactPlaybackCount(), 1);
	TestNull(TEXT("Late launch cannot resurrect completed shot"), Test.Subsystem->FindProjectile(10));

	Impact.Projectile = Test.Launch(12);
	Test.Subsystem->ReceiveUpdate(Impact);
	Test.Subsystem->ReceiveLaunch(Impact.Projectile);
	TestNull(TEXT("Completion before initial replication cannot spawn a tracer"), Test.Subsystem->FindProjectile(12));
	TestEqual(TEXT("Independent impact processed"), Test.Subsystem->GetImpactPlaybackCount(), 2);
	Test.Advance(5);
	TestEqual(TEXT("Completion history expires"), Test.Subsystem->GetCompletedProjectileCount(), 0);
	Test.Subsystem->ReceiveLaunch(LaunchA);
	TestNull(TEXT("Expired launch rejected after history eviction"), Test.Subsystem->FindProjectile(10));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileConfirmationProgressTest, "Orion.Projectiles.ConfirmationPreservesProgress", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectileConfirmationProgressTest::RunTest(const FString& Parameters)
{
	FProjectileTestWorld Test;
	FProjectilePredictionKey Key;
	Key.Session = 7;
	Key.Sequence = 1;
	Test.Subsystem->PredictProjectile(FVector(0, 0, 1000), FVector::ForwardVector, Test.Config, nullptr, Key, nullptr);
	Test.Advance(0.3f);
	const FVector PredictedPosition = Test.Subsystem->FindPrediction(Key)->Position;
	const float PredictedSimulationTime = Test.Subsystem->FindPrediction(Key)->SimulationTime;

	// Authority received the release later. Its launch timestamp is newer than our prediction's.
	FProjectileLaunch Confirmation = Test.Launch(30, Key);
	Confirmation.SimulationTime = Test.GameState->GetServerWorldTimeSeconds() - 0.05f;
	Test.Subsystem->ReceiveLaunch(Confirmation);
	const FProjectileInstance* Confirmed = Test.Subsystem->FindProjectile(30);
	if (!TestNotNull(TEXT("Prediction receives its authoritative ID"), Confirmed))
	{
		return false;
	}
	TestEqual(TEXT("Confirmation preserves predicted position"), Confirmed->Position, PredictedPosition);
	TestEqual(TEXT("Confirmation preserves local simulation progress"), Confirmed->SimulationTime, PredictedSimulationTime);
	TestEqual(TEXT("Authoritative launch metadata still adopted"), Confirmed->Launch.SimulationTime, Confirmation.SimulationTime);
	TestEqual(TEXT("Confirmation does not create another instance"), Test.Subsystem->GetActiveProjectileCount(), 1);
	Test.Subsystem->ReceiveLaunch(Confirmation);
	TestEqual(TEXT("Duplicate confirmation preserves progress"), Test.Subsystem->FindProjectile(30)->Position, PredictedPosition);
	Test.Advance(0.1f);
	TestTrue(TEXT("Confirmed projectile continues forward"), Test.Subsystem->FindProjectile(30)->Position.X > PredictedPosition.X);

	// A subsequent authoritative correction is different from accepting an existing prediction.
	FProjectileLaunch Correction = Confirmation;
	Correction.Position.X = 500;
	Correction.SimulationTime = Test.GameState->GetServerWorldTimeSeconds() - 0.02f;
	Test.Subsystem->ReceiveLaunch(Correction);
	TestTrue(TEXT("Actual server corrections still apply"), FMath::IsNearlyEqual(Test.Subsystem->FindProjectile(30)->Position.X, 520.0f, 0.01f));

	FProjectileLaunch Remote = Test.Launch(31);
	Remote.SimulationTime = Test.GameState->GetServerWorldTimeSeconds() - 0.05f;
	Test.Subsystem->ReceiveLaunch(Remote);
	TestTrue(TEXT("Unpredicted launches use authoritative age"), FMath::IsNearlyEqual(Test.Subsystem->FindProjectile(31)->Position.X, 50.0f, 0.01f));

	++Key.Sequence;
	Test.Subsystem->PredictProjectile(FVector(0, 0, 1000), FVector::ForwardVector, Test.Config, nullptr, Key, nullptr);
	Test.Advance(0.2f);
	const float FlightAge = Test.GameState->GetServerWorldTimeSeconds() - Test.Subsystem->FindPrediction(Key)->Launch.SimulationTime;
	FProjectileLaunch Redirected = Test.Launch(32, Key);
	Redirected.Velocity = FVector(0, 1000, 0);
	Redirected.SimulationTime = Test.GameState->GetServerWorldTimeSeconds() - 0.05f;
	Test.Subsystem->ReceiveLaunch(Redirected);
	const FVector ExpectedPosition = Redirected.Position + Redirected.Velocity * FlightAge;
	TestTrue(TEXT("Corrected launch direction retains predicted flight age"), Test.Subsystem->FindProjectile(32)->Position.Equals(ExpectedPosition, 0.01f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileBlueprintCompatibilityTest, "Orion.Projectiles.BlueprintCompatibility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectileBlueprintCompatibilityTest::RunTest(const FString& Parameters)
{
	const TCHAR* Paths[] = {
		TEXT("/Game/ActionRoguelike/Actions/Action_MagicProjectile.Action_MagicProjectile"),
		TEXT("/Game/ActionRoguelike/Actions/Action_Dash.Action_Dash"),
		TEXT("/Game/ActionRoguelike/Actions/Action_Blackhole.Action_Blackhole"),
		TEXT("/Game/ActionRoguelike/Performance/ProjectileSpammer.ProjectileSpammer")
	};
	for (const TCHAR* Path : Paths)
	{
		UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, Path);
		if (!TestNotNull(Path, Blueprint))
		{
			continue;
		}
		FCompilerResultsLog Results;
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection, &Results);
		TestEqual(FString::Printf(TEXT("Blueprint compiles: %s"), Path), Results.NumErrors, 0);
		if (Blueprint->GeneratedClass->IsChildOf(URogueAction_ProjectileAttack::StaticClass()))
		{
			const FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(URogueAction_ProjectileAttack::StaticClass(), TEXT("ProjectileConfig"));
			UObject* Config = Property->GetObjectPropertyValue_InContainer(Blueprint->GeneratedClass->GetDefaultObject());
			AddInfo(FString::Printf(TEXT("%s projectile config: %s"), Path, *GetPathNameSafe(Config)));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileReadinessTest, "Orion.Projectiles.ReadinessAndRejection", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectileReadinessTest::RunTest(const FString& Parameters)
{
	FProjectileTestWorld Test;
	FProjectileLaunch WaitingForClock = Test.Launch(19);
	Test.GameState->SetRole(ROLE_SimulatedProxy);
	Test.Subsystem->ReceiveLaunch(WaitingForClock);
	TestEqual(TEXT("Launch waits for first server clock sample"), Test.Subsystem->GetPendingLaunchCount(), 1);
	Test.GameState->SetRole(ROLE_Authority);
	Test.Advance(0.02f);
	TestNotNull(TEXT("Clock readiness releases deferred launch"), Test.Subsystem->FindProjectile(19));
	Test.Subsystem->ReceiveRemoval(WaitingForClock);
	FProjectileLaunch Launch = Test.Launch(20);
	Launch.ConfigDataAsset = nullptr;
	Test.Subsystem->ReceiveLaunch(Launch);
	TestEqual(TEXT("Unresolved asset deferred"), Test.Subsystem->GetPendingLaunchCount(), 1);
	Test.Subsystem->ReceiveRemoval(Launch);
	Launch.ConfigDataAsset = Test.Config;
	Test.Subsystem->ReceiveLaunch(Launch);
	TestEqual(TEXT("Removal cancels deferred creation"), Test.Subsystem->GetPendingLaunchCount(), 0);
	TestEqual(TEXT("Mapping after removal does not spawn"), Test.Subsystem->GetActiveProjectileCount(), 0);
	Launch = Test.Launch(21);
	Launch.ConfigDataAsset = nullptr;
	FProjectileUpdate Impact;
	Impact.Projectile = Launch;
	Impact.Outcome = EProjectileOutcome::Impacted;
	Test.Subsystem->ReceiveUpdate(Impact);
	Launch.ConfigDataAsset = Test.Config;
	Test.Subsystem->ReceiveLaunch(Launch);
	TestEqual(TEXT("Deferred impact resolves without a live projectile"), Test.Subsystem->GetImpactPlaybackCount(), 1);

	FProjectilePredictionKey Key;
	Key.Session = 3;
	Key.Sequence = 1;
	Test.Subsystem->PredictProjectile(FVector::ZeroVector, FVector::ForwardVector, Test.Config, nullptr, Key, nullptr);
	FProjectileUpdate Rejected;
	Rejected.Projectile.PredictionKey = Key;
	Rejected.Outcome = EProjectileOutcome::Rejected;
	Test.Subsystem->ReceiveUpdate(Rejected);
	Test.Subsystem->PredictProjectile(FVector::ZeroVector, FVector::ForwardVector, Test.Config, nullptr, Key, nullptr);
	TestNull(TEXT("Rejected prediction cannot be recreated"), Test.Subsystem->FindPrediction(Key));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileAuthorityTest, "Orion.Projectiles.AuthorityAndDamageDisabled", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectileAuthorityTest::RunTest(const FString& Parameters)
{
	FProjectileTestWorld Test;
	ARogueProjectileTestCharacter* Target = Test.World->SpawnActor<ARogueProjectileTestCharacter>(FVector(200, 0, 1000), FRotator::ZeroRotator);
	UBoxComponent* Box = NewObject<UBoxComponent>(Target);
	Box->SetupAttachment(Target->GetRootComponent());
	Box->SetBoxExtent(FVector(20));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Overlap);
	Box->RegisterComponent();
	Box->SetWorldLocation(FVector(200, 0, 1000));
	const float Health = Target->GetActionComponent()->GetAttributeValue(FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health")));
	const int64 ID = Test.Subsystem->CreateProjectile(FVector(0, 0, 1000), FVector::ForwardVector, Test.Config, nullptr);
	Test.Advance(0.4f);
	TestNull(TEXT("Authority collision completes projectile"), Test.Subsystem->FindProjectile(ID));
	TestEqual(TEXT("No damage applied"), Target->GetActionComponent()->GetAttributeValue(FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health"))), Health);
	TestEqual(TEXT("Live replication record removed"), Test.GameState->ProjectileData.Items.Num(), 0);
	TestEqual(TEXT("Impact consumed once"), Test.Subsystem->GetImpactPlaybackCount(), 1);
	Test.Config->Lifespan = 0.1f;
	Test.Subsystem->CreateProjectile(FVector(0, 1000, 1000), FVector::ForwardVector, Test.Config, nullptr);
	Test.Advance(0.3f);
	TestEqual(TEXT("Expiry adds no impact"), Test.Subsystem->GetImpactPlaybackCount(), 1);
	TestEqual(TEXT("No active instances leak"), Test.Subsystem->GetActiveProjectileCount(), 0);
	return true;
}

#if USE_DATA_ORIENTED_PROJECTILES
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileAttackProtocolTest, "Orion.Projectiles.AttackProtocol", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectileAttackProtocolTest::RunTest(const FString& Parameters)
{
	FProjectileTestWorld Test;
	ARogueProjectileTestCharacter* Pawn = Test.World->SpawnActor<ARogueProjectileTestCharacter>(FVector(0, 0, 1000), FRotator::ZeroRotator);
	URogueActionComponent* Comp = Pawn->GetActionComponent();
	URogueProjectileTestAttack* Attack = CastChecked<URogueProjectileTestAttack>(Comp->GetAction(URogueProjectileTestAttack::StaticClass()));
	Attack->Configure(Test.Config, 0.1f);
	FProjectilePredictionKey Key;
	Key.Session = Comp->GetProjectileSession();
	Key.Sequence = 1;
	Comp->ServerBeginProjectileAttack(FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack")), Key);
	Comp->ServerReleaseProjectile(Key, FVector::ForwardVector);
	Comp->ServerReleaseProjectile(Key, FVector::ForwardVector);
	TestEqual(TEXT("Early aim waits for wind-up"), Test.Subsystem->GetActiveProjectileCount(), 0);
	Test.Advance(0.15f);
	TestEqual(TEXT("Exactly one permitted projectile"), Test.Subsystem->GetActiveProjectileCount(), 1);
	Comp->ServerBeginProjectileAttack(FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack")), Key);
	Comp->ServerReleaseProjectile(Key, FVector::ForwardVector);
	Test.Advance(0.15f);
	TestEqual(TEXT("Replayed attack cannot create another projectile"), Test.Subsystem->GetActiveProjectileCount(), 1);
	++Key.Sequence;
	Comp->ServerBeginProjectileAttack(FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack")), Key);
	Comp->StopAllActions();
	Comp->ServerReleaseProjectile(Key, FVector::ForwardVector);
	Test.Advance(0.15f);
	TestEqual(TEXT("Cancellation invalidates its release"), Test.Subsystem->GetActiveProjectileCount(), 1);
	++Key.Sequence;
	++Key.Session;
	Comp->ServerBeginProjectileAttack(FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack")), Key);
	Comp->ServerReleaseProjectile(Key, FVector::ForwardVector);
	Test.Advance(0.15f);
	TestEqual(TEXT("Wrong firing session rejected"), Test.Subsystem->GetActiveProjectileCount(), 1);
	--Key.Session;
	++Key.Sequence;
	Comp->ServerBeginProjectileAttack(FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack")), Key);
	Test.Advance(2.3f);
	TestFalse(TEXT("Missing release times out and stops its action"), Attack->IsRunning());
	TestEqual(TEXT("Timed-out attack spawns no shot"), Test.Subsystem->GetActiveProjectileCount(), 0);
	++Key.Sequence;
	Comp->ServerBeginProjectileAttack(FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack")), Key);
	Comp->ServerReleaseProjectile(Key, FVector::ZeroVector);
	TestFalse(TEXT("Malformed aim stops the attack"), Attack->IsRunning());
	TestEqual(TEXT("Malformed aim creates no shot"), Test.Subsystem->GetActiveProjectileCount(), 0);
	return true;
}

#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileEarlyInputTest, "Orion.Projectiles.InputBeforeActionReplication", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectileEarlyInputTest::RunTest(const FString& Parameters)
{
	FProjectileTestWorld Test;
	const FTransform Transform(FVector(0, 0, 1000));
	ARogueProjectileTestCharacter* Pawn = Test.World->SpawnActorDeferred<ARogueProjectileTestCharacter>(ARogueProjectileTestCharacter::StaticClass(), Transform);
	Pawn->SetRole(ROLE_SimulatedProxy);
	Pawn->FinishSpawning(Transform);
	TestFalse(TEXT("Input waits for initial action replication"), Pawn->GetActionComponent()->StartActionByName(Pawn,
		FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack"))));
	TestEqual(TEXT("No early prediction created"), Test.Subsystem->GetActiveProjectileCount(), 0);
	return true;
}
#endif
