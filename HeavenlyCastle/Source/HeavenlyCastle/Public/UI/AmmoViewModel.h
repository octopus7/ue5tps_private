// ViewModel for ammo UI

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AmmoViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedVM, int32, Current, int32, Max);

UCLASS(BlueprintType)
class HEAVENLYCASTLE_API UAmmoViewModel : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Ammo")
    int32 Current = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Ammo")
    int32 Max = 0;

    UPROPERTY(BlueprintAssignable, Category = "Ammo")
    FOnAmmoChangedVM OnAmmoChanged;

    UFUNCTION(BlueprintCallable, Category = "Ammo")
    void SetAmmo(int32 InCurrent, int32 InMax)
    {
        if (Current != InCurrent || Max != InMax)
        {
            Current = InCurrent;
            Max = InMax;
            OnAmmoChanged.Broadcast(Current, Max);
        }
    }
};
