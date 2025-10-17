#pragma once

#include "CoreMinimal.h"
#include "AmmoTypes.generated.h"

USTRUCT(BlueprintType)
struct FAmmoConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
    int32 MagazineCapacity = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
    int32 StartingMagazineAmmo = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
    int32 MaxReserveAmmo = 300;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
    int32 StartingReserveAmmo = 60;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo")
    bool bInfiniteAmmo = false;
};

class FAmmoState
{
public:
    void Initialize(const FAmmoConfig& InConfig);

    bool CanFire() const;

    bool ConsumeRound();

    bool Reload();

    int32 AddReserveAmmo(int32 Amount);

    int32 GetAvailableReserveSpace() const;

    int32 GetMagazineAmmo() const { return CurrentMagazineAmmo; }

    int32 GetReserveAmmo() const { return CurrentReserveAmmo; }

    int32 GetMagazineCapacity() const { return Config.MagazineCapacity; }

    int32 GetMaxReserveAmmo() const { return Config.MaxReserveAmmo; }

    bool HasInfiniteAmmo() const { return Config.bInfiniteAmmo; }

    const FAmmoConfig& GetConfig() const { return Config; }

private:
    FAmmoConfig Config;
    int32 CurrentMagazineAmmo = 0;
    int32 CurrentReserveAmmo = 0;
};
