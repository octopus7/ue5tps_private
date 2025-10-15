// ViewModel for cover availability UI

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CoverViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoverAvailabilityChangedVM, bool, bIsAvailable);

UCLASS(BlueprintType)
class HEAVENLYCASTLE_API UCoverViewModel : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Cover")
    bool bAvailable = false;

    UPROPERTY(BlueprintAssignable, Category = "Cover")
    FOnCoverAvailabilityChangedVM OnCoverAvailabilityChanged;

    UFUNCTION(BlueprintCallable, Category = "Cover")
    void SetAvailable(bool bInAvailable)
    {
        if (bAvailable != bInAvailable)
        {
            bAvailable = bInAvailable;
            OnCoverAvailabilityChanged.Broadcast(bAvailable);
        }
    }
};
