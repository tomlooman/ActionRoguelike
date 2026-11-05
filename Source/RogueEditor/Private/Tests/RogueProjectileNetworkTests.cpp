#include "RogueProjectileTestTypes.h"

#include "ActionRoguelike.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Core/RogueGameState.h"
#include "Editor.h"
#include "Engine/NetDriver.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "Projectiles/RogueProjectilesSubsystem.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationEditorCommon.h"

#if WITH_DEV_AUTOMATION_TESTS && USE_DATA_ORIENTED_PROJECTILES
namespace
{
	struct FProjectileNetworkScenario : public IAutomationLatentCommand
	{
		FAutomationTestBase* Test;
		bool bDedicated;
		int32 Phase = 0;
		float Started = FPlatformTime::Seconds();
		TStrongObjectPtr<ULevelEditorPlaySettings> Settings;
		TWeakObjectPtr<UWorld> Server;
		TArray<TWeakObjectPtr<UWorld>> Clients;
		TArray<FProjectilePredictionKey> Keys;
		TMap<FProjectilePredictionKey, int64> Accepted;
		TSet<uint32> ObservedPredictions;
		TSet<uint32> ObservedConfirmations;
		TMap<uint32, FVector> LastPredictedPositions;
		int64 LateJoinProjectile = 0;
		FProjectilePredictionKey CorrectionKey;
		TWeakObjectPtr<UWorld> CorrectionClient;
		TWeakObjectPtr<AActor> ClientObstacle;
		bool bObservedTentativeHit = false;
		bool bFinished = false;

		FProjectileNetworkScenario(FAutomationTestBase* InTest, bool bInDedicated)
			: Test(InTest), bDedicated(bInDedicated), Settings(NewObject<ULevelEditorPlaySettings>())
		{
			Settings->SetPlayNetMode(bDedicated ? PIE_Client : PIE_ListenServer);
			Settings->SetRunUnderOneProcess(true);
			Settings->SetPlayNumberOfClients(bDedicated ? 2 : 3);
			Settings->SetServerPort(17861);
			Settings->NetworkEmulationSettings.bIsNetworkEmulationEnabled = true;
			Settings->NetworkEmulationSettings.EmulationTarget = NetworkEmulationTarget::Any;
			Settings->NetworkEmulationSettings.CurrentProfile = TEXT("Custom");
			Settings->NetworkEmulationSettings.OutPackets.MinLatency = 100;
			Settings->NetworkEmulationSettings.OutPackets.MaxLatency = 150;
			Settings->NetworkEmulationSettings.OutPackets.PacketLossPercentage = 10;
		}

		bool Finish()
		{
			if (!bFinished)
			{
				bFinished = true;
				if (Server.IsValid())
				{
					for (FConstPlayerControllerIterator It = Server->GetPlayerControllerIterator(); It; ++It)
					{
						It->Get()->UnPossess();
					}
				}
				GEditor->RequestEndPlayMap();
			}
			return true;
		}

		bool GatherWorlds(int32 RequiredClients)
		{
			Clients.Reset();
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				if (!World || Context.WorldType != EWorldType::PIE)
				{
					continue;
				}
				if (World->GetNetMode() == NM_Client)
				{
					APlayerController* PC = World->GetFirstPlayerController();
					ARogueProjectileTestCharacter* Pawn = PC ? Cast<ARogueProjectileTestCharacter>(PC->GetPawn()) : nullptr;
					if (Pawn && World->GetGameState<ARogueGameState>() && World->GetGameState<ARogueGameState>()->HasSynchronizedServerTime() &&
						Pawn->GetActionComponent()->GetProjectileSession() != 0 &&
						Pawn->GetActionComponent()->GetAction(URogueProjectileTestAttack::StaticClass()))
					{
						Clients.Add(World);
					}
				}
				else if (World->GetNetMode() == (bDedicated ? NM_DedicatedServer : NM_ListenServer))
				{
					Server = World;
				}
			}
			return Server.IsValid() && Clients.Num() >= RequiredClients;
		}

		virtual bool Update() override
		{
			if (FPlatformTime::Seconds() - Started > 65)
			{
				Test->AddError(FString::Printf(TEXT("Projectile network scenario timed out in phase %d (%d ready clients, %d accepted shots, %d predictions, %d confirmations)."),
					Phase, Clients.Num(), Accepted.Num(), ObservedPredictions.Num(), ObservedConfirmations.Num()));
				return Finish();
			}
			if (Phase == 0)
			{
				// These connection-time modules must register serializers before Iris systems are created.
				FModuleManager::Get().LoadModule(TEXT("Voice"));
				FModuleManager::Get().LoadModule(TEXT("PerfCounters"));
				UWorld* EditorWorld = FAutomationEditorCommonUtils::CreateNewMap();
				AStaticMeshActor* Wall = EditorWorld->SpawnActor<AStaticMeshActor>();
				Wall->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
				Wall->SetActorLocation(FVector(3000, 0, 0));
				Wall->SetActorScale3D(FVector(0.1f, 100, 100));
				Wall->GetStaticMeshComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
				FRequestPlaySessionParams Params;
				Params.EditorPlaySettings = Settings.Get();
				Params.GameModeOverride = ARogueProjectileTestGameMode::StaticClass();
				Params.bAllowOnlineSubsystem = false;
				GEditor->RequestPlaySession(Params);
				Phase = 1;
				return false;
			}
			if (!GatherWorlds(Phase >= 6 ? 3 : 2))
			{
				return false;
			}
			URogueProjectilesSubsystem* ServerSubsystem = Server->GetSubsystem<URogueProjectilesSubsystem>();
			ARogueGameState* GS = Server->GetGameState<ARogueGameState>();
			if (Phase == 1)
			{
				Test->TestTrue(TEXT("Real net driver uses Iris"), Server->GetNetDriver()->IsUsingIrisReplication());
				Test->TestEqual(TEXT("Configured packet loss"), Server->GetNetDriver()->PacketSimulationSettings.PktLoss, 10);
				for (const auto& Client : Clients)
				{
					APawn* Pawn = Client->GetFirstPlayerController()->GetPawn();
					URogueActionComponent* Comp = URogueActionComponent::GetActionComponent(Pawn);
					FProjectilePredictionKey Key;
					Key.Session = Comp->GetProjectileSession();
					Key.Sequence = 1;
					Keys.Add(Key);
					Test->TestTrue(TEXT("Client starts a predicted attack"), Comp->StartActionByName(Pawn, FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack"))));
				}
				Phase = 2;
			}
			if (Phase == 2)
			{
				for (const auto& Client : Clients)
				{
					URogueProjectilesSubsystem* Subsystem = Client->GetSubsystem<URogueProjectilesSubsystem>();
					const uint32 Session = URogueActionComponent::GetActionComponent(Client->GetFirstPlayerController()->GetPawn())->GetProjectileSession();
					for (FProjectilePredictionKey Key : Keys)
					{
						if (const FProjectileInstance* Instance = Subsystem->FindPrediction(Key))
						{
							if (Key.Session == Session && Instance->Launch.ID == 0)
							{
								ObservedPredictions.Add(Session);
								LastPredictedPositions.Add(Session, Instance->Position);
							}
							if (Key.Session == Session && Instance->Launch.ID > 0)
							{
								if (!ObservedConfirmations.Contains(Session))
								{
									if (const FVector* Previous = LastPredictedPositions.Find(Session))
									{
										const float Progress = FVector::DotProduct(Instance->Position - *Previous, Instance->Launch.Velocity.GetSafeNormal());
										Test->TestTrue(TEXT("Confirmation does not rewind the owner's projectile"), Progress >= -0.01f);
									}
								}
								ObservedConfirmations.Add(Session);
							}
						}
					}
				}
				for (const FProjectileItem& Item : GS->ProjectileData.Items)
				{
					if (Keys.Contains(Item.Projectile.PredictionKey))
					{
						if (const int64* Previous = Accepted.Find(Item.Projectile.PredictionKey))
						{
							Test->TestEqual(TEXT("One server ID per attack"), Item.Projectile.ID, *Previous);
						}
						Accepted.Add(Item.Projectile.PredictionKey, Item.Projectile.ID);
					}
				}
				if (Accepted.Num() == 2 && ObservedConfirmations.Num() == 2 && ServerSubsystem->GetActiveProjectileCount() == 0)
				{
					Test->TestEqual(TEXT("Both owners predicted before confirmation"), ObservedPredictions.Num(), 2);
					Phase = 3;
				}
			}
			if (Phase == 3)
			{
				for (const auto& Client : Clients)
				{
					URogueProjectilesSubsystem* Subsystem = Client->GetSubsystem<URogueProjectilesSubsystem>();
					if (Subsystem->GetActiveProjectileCount() != 0 || Subsystem->GetImpactPlaybackCount() == 0)
					{
						return false;
					}
					URogueActionComponent* Comp = URogueActionComponent::GetActionComponent(Client->GetFirstPlayerController()->GetPawn());
					const URogueProjectileTestAttack* Attack = CastChecked<URogueProjectileTestAttack>(Comp->GetAction(URogueProjectileTestAttack::StaticClass()));
					Test->TestTrue(TEXT("Owner and observer paths do not duplicate impacts"), Subsystem->GetImpactPlaybackCount() <= 2);
					Test->TestEqual(TEXT("Predicted casting cosmetics play once"), Attack->GetCosmeticCount(), 1);
				}
				CorrectionClient = Clients[0];
				AActor* Obstacle = CorrectionClient->SpawnActor<AActor>();
				UBoxComponent* Box = NewObject<UBoxComponent>(Obstacle);
				Obstacle->SetRootComponent(Box);
				Box->SetBoxExtent(FVector(20, 500, 500));
				Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				Box->SetCollisionResponseToAllChannels(ECR_Ignore);
				Box->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
				Box->RegisterComponent();
				Obstacle->SetActorLocation(FVector(300, 0, 0));
				ClientObstacle = Obstacle;
				APawn* Pawn = CorrectionClient->GetFirstPlayerController()->GetPawn();
				URogueActionComponent* Comp = URogueActionComponent::GetActionComponent(Pawn);
				CorrectionKey.Session = Comp->GetProjectileSession();
				CorrectionKey.Sequence = 2;
				Test->TestTrue(TEXT("Start shot toward a client-only obstacle"), Comp->StartActionByName(Pawn, FGameplayTag::RequestGameplayTag(TEXT("Action.PrimaryAttack"))));
				Phase = 4;
				return false;
			}
			if (Phase == 4)
			{
				if (const FProjectileInstance* Instance = CorrectionClient->GetSubsystem<URogueProjectilesSubsystem>()->FindPrediction(CorrectionKey))
				{
					bObservedTentativeHit |= Instance->bTentativeHit;
					if (Instance->bIgnorePredictedCollision && !Instance->bTentativeHit)
					{
						Test->TestTrue(TEXT("Client first observed a false collision"), bObservedTentativeHit);
						Phase = 5;
					}
				}
			}
			if (Phase == 5 && ServerSubsystem->GetActiveProjectileCount() == 0 &&
				CorrectionClient->GetSubsystem<URogueProjectilesSubsystem>()->GetActiveProjectileCount() == 0)
			{
				ClientObstacle->Destroy();
				URogueProjectileData* Config = LoadObject<URogueProjectileData>(nullptr, TEXT("/Game/ActionRoguelike/Projectiles/DA_ProjectileConfigAI.DA_ProjectileConfigAI"));
				LateJoinProjectile = ServerSubsystem->CreateProjectile(FVector(0, 10000, 1000), FVector::ForwardVector, Config, nullptr);
				GEditor->RequestLateJoin();
				Phase = 6;
				return false;
			}
			if (Phase == 6)
			{
				for (const auto& Client : Clients)
				{
					const uint32 Session = URogueActionComponent::GetActionComponent(Client->GetFirstPlayerController()->GetPawn())->GetProjectileSession();
					if (!Keys.ContainsByPredicate([Session](FProjectilePredictionKey Key) { return Key.Session == Session; }))
					{
						if (const FProjectileInstance* Instance = Client->GetSubsystem<URogueProjectilesSubsystem>()->FindProjectile(LateJoinProjectile))
						{
							Test->TestTrue(TEXT("Late join reconstructs an aged projectile"), Instance->Position.X > 0);
							const FProjectileInstance* ServerInstance = ServerSubsystem->FindProjectile(LateJoinProjectile);
							const float ErrorSeconds = ServerInstance ? FVector::Distance(Instance->Position, ServerInstance->Position) / Instance->Launch.Velocity.Size() : 1000;
							Test->AddInfo(FString::Printf(TEXT("Late join trajectory error %.3fs; client/server X %.1f/%.1f; client/server time %.3f/%.3f"),
								ErrorSeconds, Instance->Position.X, ServerInstance ? ServerInstance->Position.X : -1,
								Client->GetGameState<ARogueGameState>()->GetServerWorldTimeSeconds(), Server->GetTimeSeconds()));
							// Allow the configured 150ms delay, 100ms clock sample interval, and frame scheduling.
							Test->TestTrue(TEXT("Late join reconstructs within emulated network/clock tolerance"), ErrorSeconds < 0.4f);
							Test->TestEqual(TEXT("Late join does not replay historical impacts"), Client->GetSubsystem<URogueProjectilesSubsystem>()->GetImpactPlaybackCount(), 0);
							Phase = 7;
						}
					}
				}
			}
			if (Phase == 7 && ServerSubsystem->GetActiveProjectileCount() == 0)
			{
				for (const auto& Client : Clients)
				{
					if (Client->GetSubsystem<URogueProjectilesSubsystem>()->GetActiveProjectileCount() != 0 ||
						Client->GetGameState<ARogueGameState>()->ProjectileData.Items.Num() != 0)
					{
						return false;
					}
				}
				Test->TestEqual(TEXT("Authoritative replication array cleaned up"), GS->ProjectileData.Items.Num(), 0);
				return Finish();
			}
			return false;
		}
	};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FProjectileNetworkTest, "Orion.Projectiles.Network", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

void FProjectileNetworkTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	Names.Add(TEXT("ListenServerLagLoss"));
	Commands.Add(TEXT("Listen"));
	Names.Add(TEXT("DedicatedServerLagLoss"));
	Commands.Add(TEXT("Dedicated"));
}

bool FProjectileNetworkTest::RunTest(const FString& Parameters)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FProjectileNetworkScenario>(this, Parameters == TEXT("Dedicated")));
	return true;
}
#endif
