#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cover/CoverTypes.h"
#include "CoverControllerComponent.generated.h"

class UCoverLineComponent;
class UCoverRegistrySubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoverStateChanged, ESimpleCoverState, NewState);

/**
 * 플레이어 커버 진입/유지/피킹을 담당하는 간단한 컨트롤러 컴포넌트.
 */
UCLASS(ClassGroup = (Cover), meta = (BlueprintSpawnableComponent))
class COVER_API UCoverControllerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCoverControllerComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Cover", meta = (DisplayName = "Try Enter Cover", ToolTip = "가장 가까운 커버 라인에 진입을 시도합니다."))
    bool TryEnterCover();

    UFUNCTION(BlueprintCallable, Category = "Cover", meta = (DisplayName = "Exit Cover", ToolTip = "현재 커버에서 이탈합니다."))
    void ExitCover();

    UFUNCTION(BlueprintCallable, Category = "Cover", meta = (DisplayName = "Set Aiming", ToolTip = "커버 에이밍 상태를 갱신합니다."))
    void SetAiming(bool bAiming);

    UFUNCTION(BlueprintCallable, Category = "Cover", meta = (DisplayName = "Add Slide Input", ToolTip = "라인을 따라 좌우 이동 입력을 추가합니다."))
    void AddSlideInput(float AxisX);

    UFUNCTION(BlueprintCallable, Category = "Cover", meta = (DisplayName = "Is Muzzle Clear", ToolTip = "목표 지점을 향한 사격이 벽에 막히는지 검사합니다."))
    bool IsMuzzleClear(const FVector& Muzzle, const FVector& AimPoint) const;

    UPROPERTY(BlueprintAssignable, Category = "Cover")
    FOnCoverStateChanged OnStateChanged;

    //==== 설정 값 ====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Tuning")
    float SnapMaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Tuning")
    float SnapMaxAngleDeg;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Tuning")
    float WallMargin;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Tuning")
    float SlideSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Tuning")
    float OverPeekHeadRaise;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Tuning")
    float SidePeekNudge;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Tuning")
    float EndPeekWindow;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Camera")
    float ShoulderOffset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Camera")
    float SidePeekCamShift;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Camera")
    float OverPeekCamLift;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Camera")
    bool bRightShoulder;

    //==== 상태 ====
    UPROPERTY(BlueprintReadOnly, Category = "Cover|State")
    ECoverType CurrentType;

    UPROPERTY(BlueprintReadOnly, Category = "Cover|State")
    ESimpleCoverState State;

    UPROPERTY(BlueprintReadOnly, Category = "Cover|State")
    UCoverLineComponent* CurrentLineOwner;

    UPROPERTY(BlueprintReadOnly, Category = "Cover|State")
    FCoverLine CurrentLine;

    UPROPERTY(BlueprintReadOnly, Category = "Cover|State")
    float LineT;
    bool IsCoverSideFlipped() const;

    UPROPERTY(BlueprintReadOnly, Category = "Cover|State")
    bool bIsAiming;

    UPROPERTY(BlueprintReadOnly, Category = "Cover|State")
    float CurrentCameraXOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Cover|State")
    float CurrentCameraZOffset;

    FTransform ComputeDesiredTransform() const;

    FVector GetCurrentTangent() const;
    FVector GetCurrentNormal() const;

protected:
    bool QueryBestLine(const FVector& From, const FVector& Fwd, FCoverLine& OutLine, UCoverLineComponent*& OutOwner) const;
    void SnapToLine(const FCoverLine& Line, UCoverLineComponent* OwnerComp, float InitialT);
    void UpdateInCover(float DeltaSeconds);
    void UpdatePeeks();

private:
    void ChangeState(ESimpleCoverState NewState);
    void ClearCoverLine();
    void UpdateCameraOffsets();

    FRotator EnterCoverRotation;
    float SlideDirectionSign;
    bool bFlipCoverSides;
    float CachedSlideInput;
    float SnapElapsed;
    mutable TWeakObjectPtr<UCoverRegistrySubsystem> RegistryCached;
};
