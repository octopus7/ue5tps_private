#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhasedSpawner.generated.h"

class UArrowComponent;
class UTextRenderComponent;

USTRUCT(BlueprintType)
struct FPhaseSpawnEntry
{
    GENERATED_BODY()

    // 스폰할 적 클래스와 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
    TSubclassOf<AActor> EnemyClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn", meta=(ClampMin="0"))
    int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FSpawnPhase
{
    GENERATED_BODY()

    // 페이즈 이름(에디터 표기용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    FName PhaseName = NAME_None;

    // 이 페이즈에서 동시에 스폰할 묶음들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    TArray<FPhaseSpawnEntry> Entries;

    // (선택) 이 페이즈에서 사용할 고정 스폰 위치들
    // - 하나라도 지정되면 기본 랜덤 스폰 대신 이 위치들을 순환하여 사용
    // - SpawnPointActors: 레벨에 배치된 액터(예: TargetPoint)를 지정해 그 위치에서 스폰
    // - SpawnLocations: 직접 좌표를 지정 (에디터에서 핀으로 위치 지정 가능)
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Phase|SpawnPoints")
    TArray<AActor*> SpawnPointActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase|SpawnPoints", meta=(MakeEditWidget))
    TArray<FVector> SpawnLocations;
};

// 단계별 그룹 스폰: 각 페이즈의 적을 동시 스폰, 모두 처치 시 다음 페이즈 진행
UCLASS(meta=(DisplayName="단계스폰_PhasedSpawner"))
class HEAVENLYCASTLE_API APhasedSpawner : public AActor
{
    GENERATED_BODY()

public:
    APhasedSpawner();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    // 페이즈 구성
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Phases")
    TArray<FSpawnPhase> Phases;

    UPROPERTY(EditAnywhere, Category="Phases")
    bool bAutoStart = true;

    UPROPERTY(EditAnywhere, Category="Phases")
    bool bLoopPhases = false;

    UPROPERTY(EditAnywhere, Category="Phases")
    bool bDestroyOnComplete = false;

    // 범위/대상 설정 (TempRangeSpawner와 유사)
    UPROPERTY(EditAnywhere, Category="Spawn|Range")
    bool bSpawnAroundPlayer = true;

    UPROPERTY(EditAnywhere, Category="Spawn|Range", meta=(EditCondition="!bSpawnAroundPlayer"))
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, Category="Spawn|Range", meta=(ClampMin="0.0"))
    float MinDistance = 500.f;

    UPROPERTY(EditAnywhere, Category="Spawn|Range", meta=(ClampMin="0.0"))
    float MaxDistance = 1500.f;

    UPROPERTY(EditAnywhere, Category="Spawn|Range")
    bool bAlignToGround = true;

    // 디버그/표시
    UPROPERTY(EditAnywhere, Category="Debug")
    bool bDrawDebug = true;

    UPROPERTY(EditAnywhere, Category="Debug")
    bool bShowLabelInGame = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
    UArrowComponent* Arrow;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
    UTextRenderComponent* Label;

    // 가독성을 위한 그림자 텍스트(선택)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
    UTextRenderComponent* LabelShadow;

    // 라벨 색상/그림자 설정 (에디터에서 조정 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug|Label")
    FColor LabelColor = FColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug|Label")
    bool bUseShadowLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug|Label")
    FColor ShadowColor = FColor::Black;

    // 라벨 대비를 위한 그림자 위치 오프셋
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug|Label")
    FVector ShadowOffset = FVector(1.5f, 1.5f, -1.5f);

    // 진행 상태
    UPROPERTY(VisibleInstanceOnly, Category="State")
    int32 CurrentPhaseIndex = INDEX_NONE;

    UPROPERTY(VisibleInstanceOnly, Category="State")
    int32 AliveThisPhase = 0;

    // 블루프린트에서 바인딩 가능한 이벤트
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseStarted, int32, PhaseIndex);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllPhasesCompleted);

    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnPhaseStarted OnPhaseStarted;

    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnAllPhasesCompleted OnAllPhasesCompleted;

public:
    UFUNCTION(BlueprintCallable, Category="Phases")
    void StartPhases();

    UFUNCTION(BlueprintCallable, Category="Phases")
    void StartSpecificPhase(int32 PhaseIndex);

protected:
    void StartNextPhaseInternal();
    void SpawnPhase(const FSpawnPhase& Phase);
    void UpdateLabelText();
    FVector FindSpawnLocation(const FVector& Center) const;

    UFUNCTION()
    void HandleSpawnedDestroyed(AActor* DestroyedActor);
};
