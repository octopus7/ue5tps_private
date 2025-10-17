#include "Weapon/AmmoTypes.h"

void FAmmoState::Initialize(const FAmmoConfig& InConfig)
{
    Config = InConfig;
    Config.MagazineCapacity = FMath::Max(0, Config.MagazineCapacity);
    Config.MaxReserveAmmo = FMath::Max(0, Config.MaxReserveAmmo);
    Config.StartingMagazineAmmo = FMath::Clamp(Config.StartingMagazineAmmo, 0, Config.MagazineCapacity);
    Config.StartingReserveAmmo = FMath::Clamp(Config.StartingReserveAmmo, 0, Config.MaxReserveAmmo);

    if (Config.bInfiniteAmmo)
    {
        CurrentMagazineAmmo = Config.MagazineCapacity;
        CurrentReserveAmmo = Config.MaxReserveAmmo;
    }
    else
    {
        CurrentMagazineAmmo = Config.StartingMagazineAmmo;
        CurrentReserveAmmo = Config.StartingReserveAmmo;
    }
}

bool FAmmoState::CanFire() const
{
    return Config.bInfiniteAmmo || CurrentMagazineAmmo > 0;
}

bool FAmmoState::ConsumeRound()
{
    if (Config.bInfiniteAmmo)
    {
        if (CurrentMagazineAmmo < Config.MagazineCapacity)
        {
            CurrentMagazineAmmo = Config.MagazineCapacity;
        }

        if (CurrentReserveAmmo < Config.MaxReserveAmmo)
        {
            CurrentReserveAmmo = Config.MaxReserveAmmo;
        }

        return CurrentMagazineAmmo > 0;
    }

    if (CurrentMagazineAmmo <= 0)
    {
        return false;
    }

    --CurrentMagazineAmmo;
    return CurrentMagazineAmmo > 0;
}

bool FAmmoState::Reload()
{
    if (Config.bInfiniteAmmo)
    {
        const bool bChanged = CurrentMagazineAmmo < Config.MagazineCapacity || CurrentReserveAmmo < Config.MaxReserveAmmo;
        CurrentMagazineAmmo = Config.MagazineCapacity;
        CurrentReserveAmmo = Config.MaxReserveAmmo;
        return bChanged;
    }

    if (CurrentMagazineAmmo >= Config.MagazineCapacity || CurrentReserveAmmo <= 0)
    {
        return false;
    }

    const int32 Needed = Config.MagazineCapacity - CurrentMagazineAmmo;
    const int32 AmmoToLoad = FMath::Min(Needed, CurrentReserveAmmo);
    if (AmmoToLoad <= 0)
    {
        return false;
    }

    CurrentMagazineAmmo += AmmoToLoad;
    CurrentReserveAmmo -= AmmoToLoad;
    return true;
}
