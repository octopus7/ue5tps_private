#include "PhasedSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APhasedSpawner::APhasedSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    Arrow->ArrowColor = FColor(0, 200, 255);
    Arrow->ArrowSize = 1.5f;
    SetRootComponent(Arrow);

    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Arrow);
    Label->SetText(FText::FromString(TEXT("Phased Spawner")));
    // 에디터에서 지정 가능한 라벨 색상 사용
    Label->SetTextRenderColor(LabelColor);
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(48.f);
    Label->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

    // 가독성을 위한 그림자 텍스트 컴포넌트
    LabelShadow = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LabelShadow"));
    LabelShadow->SetupAttachment(Arrow);
    LabelShadow->SetText(FText::FromString(TEXT("Phased Spawner")));
    LabelShadow->SetTextRenderColor(ShadowColor);
    LabelShadow->SetHorizontalAlignment(EHTA_Center);
    LabelShadow->SetWorldSize(Label->WorldSize * 1.05f);
    LabelShadow->SetRelativeLocation(Label->GetRelativeLocation() + ShadowOffset);
}

void APhasedSpawner::BeginPlay()
{
    Super::BeginPlay();

    // Double the label size at begin time
    if (Label)
    {
        Label->SetWorldSize(Label->WorldSize * 2.0f);
    }
    if (LabelShadow)
    {
        // 그림자는 라벨 대비 5% 크게 유지
        LabelShadow->SetWorldSize(Label->WorldSize * 1.05f);
        LabelShadow->SetRelativeLocation(Label->GetRelativeLocation() + ShadowOffset);
    }

    if (bAutoStart)
    {
        StartPhases();
    }
}

void APhasedSpawner::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (Label)
    {
        Label->SetHiddenInGame(!bShowLabelInGame);
        Label->SetTextRenderColor(LabelColor);
    }
    if (LabelShadow)
    {
        LabelShadow->SetHiddenInGame(!bShowLabelInGame || !bUseShadowLabel);
        LabelShadow->SetTextRenderColor(ShadowColor);
        LabelShadow->SetWorldSize(Label->WorldSize * 1.05f);
        LabelShadow->SetRelativeLocation(Label->GetRelativeLocation() + ShadowOffset);
    }

    if (bDrawDebug)
    {
        if (UWorld* World = GetWorld())
        {
            const FVector C = GetActorLocation();
            const FColor MinColor = FColor(0, 200, 255);
            const FColor MaxColor = FColor(0, 120, 255);
            DrawDebugCircle(World, C, MinDistance, 64, MinColor, false, 2.0f, 0, 2.0f, FVector(1,0,0), FVector(0,1,0), false);
            DrawDebugCircle(World, C, MaxDistance, 64, MaxColor, false, 2.0f, 0, 2.0f, FVector(1,0,0), FVector(0,1,0), false);

            // 페이즈별 스폰 포인트 마커
            static const TArray<FColor> PhaseColors = {
                FColor::Green,
                FColor::Yellow,
                FColor::Red,
                FColor::Cyan,
                FColor::Orange,
                FColor::Magenta
            };

            for (int32 PhaseIdx = 0; PhaseIdx < Phases.Num(); ++PhaseIdx)
            {
                const FSpawnPhase& Phase = Phases[PhaseIdx];
                const FColor PColor = PhaseColors[PhaseIdx % PhaseColors.Num()];
                int32 MarkerIdx = 0;

                // Actor 포인트들
                for (AActor* P : Phase.SpawnPointActors)
                {
                    if (!IsValid(P)) continue;
                    const FVector Loc = P->GetActorLocation();
                    DrawDebugSphere(World, Loc, 28.f, 12, PColor, false, 2.0f, 0, 2.0f);
                    const FString Txt = FString::Printf(TEXT("P%d A%d"), PhaseIdx + 1, ++MarkerIdx);
                    DrawDebugString(World, Loc + FVector(0,0,36.f), Txt, nullptr, PColor, 2.0f, false);
                }

                // 직접 좌표 포인트들
                for (int32 i = 0; i < Phase.SpawnLocations.Num(); ++i)
                {
                    const FVector Loc = Phase.SpawnLocations[i];
                    DrawDebugBox(World, Loc, FVector(18.f), PColor, false, 2.0f, 0, 2.0f);
                    const FString Txt = FString::Printf(TEXT("P%d L%d"), PhaseIdx + 1, i + 1);
                    DrawDebugString(World, Loc + FVector(0,0,36.f), Txt, nullptr, PColor, 2.0f, false);
                }
            }
        }
    }

    UpdateLabelText();
}

void APhasedSpawner::StartPhases()
{
    if (Phases.Num() <= 0)
    {
        CurrentPhaseIndex = INDEX_NONE;
        AliveThisPhase = 0;
        UpdateLabelText();
        return;
    }
    StartSpecificPhase(0);
}

void APhasedSpawner::StartSpecificPhase(int32 PhaseIndex)
{
    if (!ensure(PhaseIndex >= 0 && PhaseIndex < Phases.Num()))
    {
        return;
    }
    CurrentPhaseIndex = PhaseIndex;
    AliveThisPhase = 0;
    UpdateLabelText();

    OnPhaseStarted.Broadcast(CurrentPhaseIndex);
    SpawnPhase(Phases[CurrentPhaseIndex]);

    // 스폰이 0건이면 즉시 다음 페이즈로 진행
    if (AliveThisPhase <= 0)
    {
        StartNextPhaseInternal();
    }
}

void APhasedSpawner::StartNextPhaseInternal()
{
    int32 NextIndex = (CurrentPhaseIndex == INDEX_NONE) ? 0 : (CurrentPhaseIndex + 1);
    const bool bDone = (Phases.Num() <= 0) || (NextIndex >= Phases.Num());
    if (bDone)
    {
        if (bLoopPhases && Phases.Num() > 0)
        {
            StartSpecificPhase(0);
        }
        else
        {
            OnAllPhasesCompleted.Broadcast();
            if (bDestroyOnComplete)
            {
                Destroy();
            }
            else
            {
                // 완료 상태 표시만 갱신
                CurrentPhaseIndex = INDEX_NONE;
                AliveThisPhase = 0;
                UpdateLabelText();
            }
        }
        return;
    }
    StartSpecificPhase(NextIndex);
}

void APhasedSpawner::SpawnPhase(const FSpawnPhase& Phase)
{
    // 기준 위치 결정 (플레이어/타겟/자기자신)
    AActor* CenterActor = nullptr;
    if (bSpawnAroundPlayer)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            CenterActor = PC->GetPawn();
        }
    }
    else
    {
        CenterActor = TargetActor ? TargetActor : this;
    }
    const FVector CenterLoc = CenterActor ? CenterActor->GetActorLocation() : GetActorLocation();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // 페이즈별 고정 스폰 포인트가 있는지 모아서 확인
    TArray<FVector> FixedPoints;
    FixedPoints.Reserve(Phase.SpawnPointActors.Num() + Phase.SpawnLocations.Num());
    for (AActor* P : Phase.SpawnPointActors)
    {
        if (IsValid(P))
        {
            FixedPoints.Add(P->GetActorLocation());
        }
    }
    for (const FVector& L : Phase.SpawnLocations)
    {
        FixedPoints.Add(L);
    }

    for (const FPhaseSpawnEntry& Entry : Phase.Entries)
    {
        if (!Entry.EnemyClass || Entry.Count <= 0)
        {
            continue;
        }

        for (int32 i = 0; i < Entry.Count; ++i)
        {
            FVector SpawnLoc;
            if (FixedPoints.Num() > 0)
            {
                const int32 Index = (i % FixedPoints.Num());
                SpawnLoc = FixedPoints[Index];
                // 지정 포인트도 지면에 맞추고 싶다면 전역 옵션을 그대로 적용
                if (bAlignToGround)
                {
                    // FindSpawnLocation의 지면 정렬 부분을 재사용하기 위해 Center는 무시하고 수직 트레이스만 수행
                    FVector Start = SpawnLoc + FVector(0, 0, 1000.f);
                    FVector End = SpawnLoc - FVector(0, 0, 2000.f);
                    FHitResult Hit;
                    FCollisionQueryParams Params(SCENE_QUERY_STAT(PhasedSpawnerTrace), false, this);
                    if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
                    {
                        SpawnLoc = Hit.ImpactPoint + FVector(0, 0, 2.f);
                    }
                }
            }
            else
            {
                SpawnLoc = FindSpawnLocation(CenterLoc);
            }

            FRotator SpawnRot = FRotator::ZeroRotator;
            if (CenterActor)
            {
                SpawnRot = (CenterLoc - SpawnLoc).Rotation();
                SpawnRot.Pitch = 0.f;
                SpawnRot.Roll = 0.f;
            }

            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            if (bDrawDebug)
            {
                DrawDebugSphere(World, SpawnLoc, 24.f, 16, FColor::Cyan, false, 2.0f, 0, 2.0f);
            }

            if (AActor* Spawned = World->SpawnActor<AActor>(Entry.EnemyClass, SpawnLoc, SpawnRot, Params))
            {
                ++AliveThisPhase;
                // 파괴 시 카운트 감소
                Spawned->OnDestroyed.AddDynamic(this, &APhasedSpawner::HandleSpawnedDestroyed);
            }
        }
    }

    UpdateLabelText();
}

void APhasedSpawner::HandleSpawnedDestroyed(AActor* DestroyedActor)
{
    AliveThisPhase = FMath::Max(0, AliveThisPhase - 1);
    UpdateLabelText();

    if (AliveThisPhase <= 0)
    {
        StartNextPhaseInternal();
    }
}

FVector APhasedSpawner::FindSpawnLocation(const FVector& Center) const
{
    const float R = FMath::FRandRange(MinDistance, MaxDistance);
    const float Theta = FMath::FRandRange(0.f, 2 * PI);
    FVector Offset(FMath::Cos(Theta) * R, FMath::Sin(Theta) * R, 0.f);

    FVector Location = Center + Offset;

    if (bAlignToGround)
    {
        if (UWorld* World = GetWorld())
        {
            FVector Start = Location + FVector(0, 0, 1000.f);
            FVector End = Location - FVector(0, 0, 2000.f);
            FHitResult Hit;
            FCollisionQueryParams Params(SCENE_QUERY_STAT(PhasedSpawnerTrace), false, this);
            if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
            {
                Location = Hit.ImpactPoint + FVector(0, 0, 2.f);
            }
        }
    }

    return Location;
}

void APhasedSpawner::UpdateLabelText()
{
    if (!Label)
    {
        return;
    }

    if (CurrentPhaseIndex == INDEX_NONE)
    {
        const FText IdleTxt = FText::FromString(TEXT("Phased Spawner (Idle/Completed)"));
        Label->SetText(IdleTxt);
        if (LabelShadow)
        {
            LabelShadow->SetText(IdleTxt);
        }
        return;
    }
    const int32 TotalPhases = Phases.Num();
    const FString Txt = FString::Printf(TEXT("Phase %d/%d | Alive %d"), CurrentPhaseIndex + 1, TotalPhases, AliveThisPhase);
    const FText TxtText = FText::FromString(Txt);
    Label->SetText(TxtText);
    if (LabelShadow)
    {
        LabelShadow->SetText(TxtText);
    }
}
