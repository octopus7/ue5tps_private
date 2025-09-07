// ViewModel for health UI

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HealthViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedVM, float, Current, float, Max);

UCLASS(BlueprintType)
class HEAVENLYCASTLE_API UHealthViewModel : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float Current = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float Max = 0.f;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChangedVM OnHealthChanged;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float InCurrent, float InMax)
    {
        if (!FMath::IsNearlyEqual(Current, InCurrent) || !FMath::IsNearlyEqual(Max, InMax))
        {
            Current = InCurrent;
            Max = InMax;
            OnHealthChanged.Broadcast(Current, Max);
        }
    }
};
