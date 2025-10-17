#pragma once

#include "CoreMinimal.h"
#include "AmmoTypes.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
    Rifle   UMETA(DisplayName = "Rifle"),
    Pistol  UMETA(DisplayName = "Pistol")
};

USTRUCT(BlueprintType)
struct FAmmoTypeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo")
    EAmmoType AmmoType = EAmmoType::Rifle;

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

class FAmmoTypeState
{
public:
    void Initialize(const FAmmoTypeConfig& InConfig);

    bool CanFire() const;

    bool ConsumeRound();

    bool Reload();

    int32 AddReserveAmmo(int32 Amount);

    int32 GetAvailableReserveSpace() const;

    EAmmoType GetAmmoType() const { return Config.AmmoType; }

    int32 GetMagazineAmmo() const { return CurrentMagazineAmmo; }

    int32 GetReserveAmmo() const { return CurrentReserveAmmo; }

    int32 GetMagazineCapacity() const { return Config.MagazineCapacity; }

    int32 GetMaxReserveAmmo() const { return Config.MaxReserveAmmo; }

    bool HasInfiniteAmmo() const { return Config.bInfiniteAmmo; }

    const FAmmoTypeConfig& GetConfig() const { return Config; }

private:
    FAmmoTypeConfig Config;
    int32 CurrentMagazineAmmo = 0;
    int32 CurrentReserveAmmo = 0;
};

class FAmmoInventory
{
public:
    void Initialize(const TArray<FAmmoTypeConfig>& InConfigs);

    bool HasType(EAmmoType AmmoType) const;

    FAmmoTypeState* FindAmmoState(EAmmoType AmmoType);

    const FAmmoTypeState* FindAmmoState(EAmmoType AmmoType) const;

    bool CanFire(EAmmoType AmmoType) const;

    bool ConsumeRound(EAmmoType AmmoType);

    bool Reload(EAmmoType AmmoType);

    int32 AddReserveAmmo(EAmmoType AmmoType, int32 Amount);

    int32 GetAvailableReserveSpace(EAmmoType AmmoType) const;

    int32 GetMagazineAmmo(EAmmoType AmmoType) const;

    int32 GetReserveAmmo(EAmmoType AmmoType) const;

    int32 GetMagazineCapacity(EAmmoType AmmoType) const;

    int32 GetMaxReserveAmmo(EAmmoType AmmoType) const;

    bool HasInfiniteAmmo(EAmmoType AmmoType) const;

private:
    TMap<EAmmoType, FAmmoTypeState> AmmoStates;
};
