#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoverCameraComponent.generated.h"

class UCoverControllerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraOffsetUpdated, FVector, Offset);

/**
 * 커버 상태에 따라 카메라 오프셋을 계산해 알림을 제공하는 보조 컴포넌트.
 */
UCLASS(ClassGroup = (Cover), meta = (BlueprintSpawnableComponent))
class COVER_API UCoverCameraComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCoverCameraComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Cover|Camera")
    FVector GetCurrentOffset() const { return CurrentOffset; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Camera")
    float OffsetInterpSpeed;

    UPROPERTY(BlueprintAssignable, Category = "Cover|Camera")
    FOnCameraOffsetUpdated OnCameraOffset;

private:
    TWeakObjectPtr<UCoverControllerComponent> CachedController;
    FVector CurrentOffset;
};
