#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Cover/CoverTypes.h"
#include "CoverLineComponent.generated.h"

class UWorld;

/**
 * 직선 커버 구간을 나타내는 컴포넌트.
 * 편집 시 길이/높이/열린 끝단/법선 등을 조정하고 런타임에 커버 라인을 구성한다.
 */
UCLASS(ClassGroup = (Cover), meta = (BlueprintSpawnableComponent))
class COVER_API UCoverLineComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UCoverLineComponent();

    /** 커버 길이(cm). */
    UPROPERTY(EditAnywhere, Category = "Cover")
    float Length;

    /** 커버 높이(cm) - 디버그 및 검증용. */
    UPROPERTY(EditAnywhere, Category = "Cover")
    float Height;

    /** 커버 유형 (Low/High). */
    UPROPERTY(EditAnywhere, Category = "Cover")
    ECoverType Type;

    /** 좌측 끝단 개방 여부. */
    UPROPERTY(EditAnywhere, Category = "Cover")
    bool bLeftOpen;

    /** 우측 끝단 개방 여부. */
    UPROPERTY(EditAnywhere, Category = "Cover")
    bool bRightOpen;

    /** 로컬 기준의 외향 법선. */
    UPROPERTY(EditAnywhere, Category = "Cover")
    FVector LocalNormal;

    /** 월드 공간 커버 라인 구조체 생성. */
    FCoverLine ToCoverLine() const;

    /** 디버그 그리기. */
    void DebugDraw(UWorld* World, const FColor& LineColor) const;

protected:
    virtual void OnRegister() override;
    virtual void OnUnregister() override;
    virtual void DestroyComponent(bool bPromoteChildren) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    void RegisterWithSubsystem();
    void UnregisterFromSubsystem();
};
