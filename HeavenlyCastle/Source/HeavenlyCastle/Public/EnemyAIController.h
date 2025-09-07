#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class AEnemyCharacter;
class AWaveProjectile;

UENUM()
enum class EEnemyState : uint8
{
    Idle,
    Patrol,
    Chase
};

UCLASS()
class HEAVENLYCASTLE_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

    EEnemyState GetState() const { return State; }
    UFUNCTION(BlueprintPure, Category="AI")
    float GetIdleTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category="Combat")
    float GetAttackTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category="Combat")
    float GetEffectiveAttackRange() const; // 추적 가능 거리의 절반 사용

protected:
    // Sensing/Combat params
    UPROPERTY(EditAnywhere, Category="Sensing")
    float SightRadius = 1500.f;

    UPROPERTY(EditAnywhere, Category="Sensing")
    float LoseSightRadius = 2000.f;

    UPROPERTY(EditAnywhere, Category="Sensing")
    float FOVDegrees = 90.f;

    UPROPERTY(EditAnywhere, Category="Sensing")
    bool bUseFOV = true;

    UPROPERTY(EditAnywhere, Category="Sensing")
    bool bUseLineOfSight = true;

    UPROPERTY(EditAnywhere, Category="Sensing")
    float SightHeightOffset = 50.f;

    UPROPERTY(EditAnywhere, Category="Combat")
    float AttackRange = 150.f;

    UPROPERTY(EditAnywhere, Category="Combat")
    float AttackCooldown = 1.0f;

    UPROPERTY(EditAnywhere, Category="Combat")
    float ProjectileSpawnOffset = 60.f;

    // 플레이어를 따라붙을 때 도달로 간주할 거리(가까울수록 끝까지 추격)
    UPROPERTY(EditAnywhere, Category="Chase")
    float ApproachAcceptanceRadius = 75.f;

    UPROPERTY(EditDefaultsOnly, Category="Combat")
    TSubclassOf<AWaveProjectile> AttackProjectileClass;

    UPROPERTY(EditAnywhere, Category="Patrol")
    float PatrolRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category="Patrol")
    float IdleTimeMin = 1.5f;

    UPROPERTY(EditAnywhere, Category="Patrol")
    float IdleTimeMax = 3.0f;

    UPROPERTY(EditAnywhere, Category="Debug")
    bool bDrawDebug = true;

    UPROPERTY(EditAnywhere, Category="Debug")
    float ThinkInterval = 0.2f;

    UPROPERTY(EditAnywhere, Category="Debug")
    bool bShowAttackFailReason = true;

    UPROPERTY(EditAnywhere, Category="Debug")
    float AttackDebugTextZOffset = 60.f;

private:
    TWeakObjectPtr<AEnemyCharacter> CachedEnemy;
    TWeakObjectPtr<APawn> CachedPlayer;

    EEnemyState State = EEnemyState::Idle;
    bool bInitialChasePending = true;

    FTimerHandle ThinkTimerHandle;
    FTimerHandle IdleTimerHandle;
    float LastAttackTime = -10000.f;
    FString LastAttackFailReason;

    void Think();
    void StartIdle(float Duration);
    void StartPatrol();
    void StartChase();
    void TryAttack();

    bool CanSeePlayer(bool bStrict = true) const;
    bool IsPlayerInAttackRange() const;

    void DrawFOVDebug(float Radius, const FColor& Color) const;

    void AcquirePlayer();

public:
    UFUNCTION(BlueprintPure, Category="Combat")
    FString GetLastAttackFailReason() const { return LastAttackFailReason; }

};
