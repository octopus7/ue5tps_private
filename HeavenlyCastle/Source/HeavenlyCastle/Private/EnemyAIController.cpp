#include "EnemyAIController.h"

#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "WaveProjectile.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

AEnemyAIController::AEnemyAIController()
{
    bAttachToPawn = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    CachedEnemy = Cast<AEnemyCharacter>(InPawn);
    CachedPlayer = UGameplayStatics::GetPlayerPawn(this, 0);

    if (UCharacterMovementComponent* MoveComp = InPawn ? InPawn->FindComponentByClass<UCharacterMovementComponent>() : nullptr)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
    }

    // Start in Chase state regardless of sight
    State = EEnemyState::Chase;
    StartChase();
    GetWorldTimerManager().SetTimer(ThinkTimerHandle, this, &AEnemyAIController::Think, ThinkInterval, true, ThinkInterval);
}

void AEnemyAIController::OnUnPossess()
{
    Super::OnUnPossess();
    // Stop all behavior when pawn is gone (e.g., death)
    GetWorldTimerManager().ClearTimer(ThinkTimerHandle);
    GetWorldTimerManager().ClearTimer(IdleTimerHandle);
    CachedEnemy = nullptr;
}

bool AEnemyAIController::CanSeePlayer(bool bStrict) const
{
    const APawn* Player = CachedPlayer.Get();
    const APawn* SelfPawn = GetPawn();
    if (!Player || !SelfPawn)
    {
        return false;
    }

    const FVector SelfLoc = SelfPawn->GetActorLocation();
    const FVector PlayerLoc = Player->GetActorLocation();
    const FVector ToPlayer = PlayerLoc - SelfLoc;
    const float Dist = ToPlayer.Size();
    if (bStrict)
    {
        if (Dist > SightRadius)
            return false;
    }
    else
    {
        if (Dist > LoseSightRadius)
            return false;
    }

    if (bUseFOV)
    {
        const FVector Forward = SelfPawn->GetActorForwardVector();
        const FVector Dir = ToPlayer.GetSafeNormal2D();
        const float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(FOVDegrees * 0.5f));
        const float Dot = FVector::DotProduct(Forward.GetSafeNormal2D(), Dir);
        if (Dot < CosHalfFOV)
        {
            return false;
        }
    }

    // Optional: visibility trace
    if (bUseLineOfSight)
    {
        FHitResult Hit;
        FCollisionQueryParams P(SCENE_QUERY_STAT(EnemySight), false, SelfPawn);
        P.AddIgnoredActor(SelfPawn);
        const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, SelfLoc + FVector(0,0,SightHeightOffset), PlayerLoc + FVector(0,0,SightHeightOffset), ECC_Visibility, P);
        if (bBlocked && Hit.GetActor() != Player)
        {
            return false;
        }
    }
    return true;
}

bool AEnemyAIController::IsPlayerInAttackRange() const
{
    const APawn* Player = CachedPlayer.Get();
    const APawn* SelfPawn = GetPawn();
    if (!Player || !SelfPawn)
    {
        return false;
    }
    const float Range = GetEffectiveAttackRange();
    return FVector::Dist2D(Player->GetActorLocation(), SelfPawn->GetActorLocation()) <= Range;
}

void AEnemyAIController::Think()
{
    // Refresh player pointer if needed
    if (!CachedPlayer.IsValid())
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(this, 0);
    }

    // Evaluate player visibility
    // bStrict=true uses SightRadius, false uses LoseSightRadius
    const bool bSee = CanSeePlayer(true);
    const bool bKeepChase = CanSeePlayer(false);

    switch (State)
    {
    case EEnemyState::Chase:
        // If player not yet valid (e.g., early possess), keep trying to acquire and chase
        if (!CachedPlayer.IsValid())
        {
            StartChase();
            break;
        }
        if (!bKeepChase)
        {
            StartIdle(FMath::FRandRange(IdleTimeMin, IdleTimeMax));
            return;
        }
        // Refresh chase target while chasing
        StartChase();
        // Attempt attack when in range/LOS with cooldown
        TryAttack();
        break;
    case EEnemyState::Idle:
    case EEnemyState::Patrol:
    default:
        if (bSee)
        {
            StartChase();
            return;
        }
        break;
    }

    if (bDrawDebug)
    {
        if (APawn* SelfPawn = GetPawn())
        {
            // 매 틱 갱신: 상태별로 부채꼴 색/반경 표시
            if (bUseFOV)
            {
                const bool bChasing = (State == EEnemyState::Chase);
                const float Radius = bChasing ? LoseSightRadius : SightRadius;
                const FColor Col = bChasing ? FColor(255, 165, 0) : FColor::Green;
                DrawFOVDebug(Radius, Col);
            }
            // 필요 시 공격 실패 이유 텍스트만 유지
            if (bShowAttackFailReason && !LastAttackFailReason.IsEmpty())
            {
                const FVector Loc = SelfPawn->GetActorLocation() + FVector(0,0,AttackDebugTextZOffset);
                DrawDebugString(GetWorld(), Loc, LastAttackFailReason, nullptr, FColor(255, 0, 180), ThinkInterval*1.1f, false);
            }
        }
    }
}

void AEnemyAIController::DrawFOVDebug(float Radius, const FColor& Color) const
{
    if (!GetPawn() || Radius <= 0.f)
    {
        return;
    }
    UWorld* World = GetWorld();
    const FVector Center = GetPawn()->GetActorLocation() + FVector(0,0,SightHeightOffset);
    const FVector Forward = GetPawn()->GetActorForwardVector();

    const float HalfFOV = FMath::Max(0.f, FOVDegrees * 0.5f);
    const int32 Segments = FMath::Clamp(FMath::CeilToInt(FMath::Max(8.f, FOVDegrees / 5.f)), 8, 90);
    const float Step = (Segments > 0) ? (FOVDegrees / Segments) : FOVDegrees;

    const FVector Up = FVector::UpVector;
    FVector StartDir = Forward.RotateAngleAxis(-HalfFOV, Up).GetSafeNormal2D();
    FVector Prev = Center + StartDir * Radius;
    for (int32 i = 1; i <= Segments; ++i)
    {
        const float Angle = -HalfFOV + Step * i;
        const FVector Dir = Forward.RotateAngleAxis(Angle, Up).GetSafeNormal2D();
        const FVector Pt = Center + Dir * Radius;
        DrawDebugLine(World, Prev, Pt, Color, false, ThinkInterval*1.1f, 0, 1.5f);
        Prev = Pt;
    }

    // Boundary rays
    const FVector LeftDir = Forward.RotateAngleAxis(-HalfFOV, Up).GetSafeNormal2D();
    const FVector RightDir = Forward.RotateAngleAxis(HalfFOV, Up).GetSafeNormal2D();
    DrawDebugLine(World, Center, Center + LeftDir * Radius, Color, false, ThinkInterval*1.1f, 0, 1.0f);
    DrawDebugLine(World, Center, Center + RightDir * Radius, Color, false, ThinkInterval*1.1f, 0, 1.0f);
}

void AEnemyAIController::StartIdle(float Duration)
{
    StopMovement();
    State = EEnemyState::Idle;
    GetWorldTimerManager().ClearTimer(IdleTimerHandle);
    GetWorldTimerManager().SetTimer(IdleTimerHandle, this, &AEnemyAIController::StartPatrol, Duration, false);

    if (bDrawDebug)
    {
        if (UWorld* World = GetWorld())
        {
            DrawDebugString(World, GetPawn()->GetActorLocation() + FVector(0,0,120), TEXT("Idle"), nullptr, FColor::White, 1.0f);
        }
    }
}

void AEnemyAIController::StartPatrol()
{
    if (!GetPawn()) return;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return;

    FNavLocation OutLoc;
    const FVector Origin = GetPawn()->GetActorLocation();
    if (NavSys->GetRandomReachablePointInRadius(Origin, PatrolRadius, OutLoc))
    {
        State = EEnemyState::Patrol;
        MoveToLocation(OutLoc.Location, /*AcceptanceRadius*/ 50.f, /*bStopOnOverlap*/ true);
        if (bDrawDebug)
        {
            DrawDebugSphere(GetWorld(), OutLoc.Location, 30.f, 12, FColor::Blue, false, 1.5f);
        }
    }
}

void AEnemyAIController::StartChase()
{
    if (!GetPawn()) return;
    APawn* Player = CachedPlayer.Get();
    if (!Player)
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(this, 0);
        Player = CachedPlayer.Get();
        if (!Player) return;
    }

    State = EEnemyState::Chase;
    // 공격 가능 거리와 무관하게 근접할 때까지 추격
    MoveToActor(Player, /*AcceptanceRadius*/ ApproachAcceptanceRadius, /*bStopOnOverlap*/ true);

    if (IsPlayerInAttackRange())
    {
        // Optional: show feedback; actual attack handled by TryAttack()
        if (bDrawDebug)
        {
            DrawDebugString(GetWorld(), GetPawn()->GetActorLocation() + FVector(0,0,120), TEXT("In Range"), nullptr, FColor::Red, 0.2f);
        }
    }
}

void AEnemyAIController::TryAttack()
{
    APawn* SelfPawn = GetPawn();
    APawn* Player = CachedPlayer.Get();
    if (!SelfPawn || !Player)
    {
        LastAttackFailReason = TEXT("No pawn/player");
        return;
    }

    // Dead/ragdoll gate
    if (ACharacter* SelfChar = Cast<ACharacter>(SelfPawn))
    {
        if (USkeletalMeshComponent* Skel = SelfChar->GetMesh())
        {
            if (Skel->IsSimulatingPhysics())
            {
                LastAttackFailReason = TEXT("Dead");
                return;
            }
        }
    }

    // Cooldown gate
    UWorld* World = GetWorld();
    const float Now = World ? World->TimeSeconds : 0.f;
    if (Now - LastAttackTime < AttackCooldown)
    {
        const float Rem = FMath::Max(0.f, AttackCooldown - (Now - LastAttackTime));
        LastAttackFailReason = FString::Printf(TEXT("Cooldown %.1fs"), Rem);
        return;
    }

    // Require range and LOS
    const bool bInRange = IsPlayerInAttackRange();
    const bool bLOS = CanSeePlayer(true);
    if (!bInRange || !bLOS)
    {
        if (!bInRange)
        {
            const float Dist = FVector::Dist2D(Player->GetActorLocation(), SelfPawn->GetActorLocation());
            const float Range = GetEffectiveAttackRange();
            LastAttackFailReason = FString::Printf(TEXT("Out of range %.0f/%.0f"), Dist, Range);
        }
        else if (!bLOS)
        {
            LastAttackFailReason = TEXT("No LOS/FOV");
        }
        return;
    }

    const FVector SelfLoc = SelfPawn->GetActorLocation();
    FVector Dir = (Player->GetActorLocation() - SelfLoc);
    Dir.Z = 0.f;
    if (!Dir.Normalize())
    {
        LastAttackFailReason = TEXT("Bad dir");
        return;
    }

    const FVector SpawnXY = SelfLoc + Dir * ProjectileSpawnOffset;
    const float SpawnZ = Player->GetActorLocation().Z + 40.f; // 플레이어 높이에 맞춰 발사 Z 정렬
    const FVector SpawnLoc(SpawnXY.X, SpawnXY.Y, SpawnZ);
    const FRotator SpawnRot = Dir.Rotation();

    FActorSpawnParameters Params;
    Params.Owner = SelfPawn;
    Params.Instigator = SelfPawn;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (!AttackProjectileClass)
    {
        AttackProjectileClass = AWaveProjectile::StaticClass();
    }

    if (World && AttackProjectileClass)
    {
        // do not attack me
        bool enableAttack = !true;
        if(!enableAttack)
        {
            return;
        }

        if (AWaveProjectile* P = World->SpawnActor<AWaveProjectile>(AttackProjectileClass, SpawnLoc, SpawnRot, Params))
        {
            P->InitVelocity(Dir);
            LastAttackTime = Now;
            LastAttackFailReason.Empty();
        }
    }
}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    if (State == EEnemyState::Patrol)
    {
        // After reaching a patrol point, idle a bit then pick another
        StartIdle(FMath::FRandRange(IdleTimeMin, IdleTimeMax));
    }
    else if (State == EEnemyState::Chase)
    {
        // Keep thinking; Think() will refresh chase target or fall back to idle
    }
}

float AEnemyAIController::GetIdleTimeRemaining() const
{
    if (State != EEnemyState::Idle)
    {
        return 0.f;
    }
    return const_cast<AEnemyAIController*>(this)->GetWorldTimerManager().GetTimerRemaining(const_cast<AEnemyAIController*>(this)->IdleTimerHandle);
}

float AEnemyAIController::GetAttackTimeRemaining() const
{
    const UWorld* World = GetWorld();
    const float Now = World ? World->TimeSeconds : 0.f;
    const float Elapsed = Now - LastAttackTime;
    const float Remaining = AttackCooldown - Elapsed;
    return Remaining > 0.f ? Remaining : 0.f;
}

float AEnemyAIController::GetEffectiveAttackRange() const
{
    // 추적 유지 거리(LoseSightRadius)의 절반을 공격 사거리로 사용
    return LoseSightRadius * 0.5f;
}
