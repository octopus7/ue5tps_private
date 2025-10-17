#pragma once

#include "CoreMinimal.h"
#include "Weapon/AmmoTypes.h"
#include "UObject/Interface.h"
#include "AmmoConsumerInterface.generated.h"

UINTERFACE(BlueprintType)
class UAmmoConsumerInterface : public UInterface
{
    GENERATED_BODY()
};

class IAmmoConsumerInterface
{
    GENERATED_BODY()

public:
    /**
     * @return Number of rounds successfully added to the consumer.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Ammo")
    int32 AddAmmoToInventory(EAmmoType AmmoType, int32 Amount);
};
