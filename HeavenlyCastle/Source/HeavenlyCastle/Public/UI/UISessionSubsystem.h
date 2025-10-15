// Local-player UI session subsystem: bridge between game code and UMG

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UISessionSubsystem.generated.h"

class UUserWidget;
class UCombatStateViewModel;
class UHealthViewModel;
class UAmmoViewModel;
class UCoverViewModel;
enum class ECombatState : uint8;

UCLASS()
class HEAVENLYCASTLE_API UUISessionSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void EnsureHUD(APlayerController* OwningPC);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void EnsureHUDWithClass(APlayerController* OwningPC, TSubclassOf<UUserWidget> InClass);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void PushCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void PushHealth(float Current, float Max);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void PushAmmo(int32 Current, int32 Max);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void PushCoverAvailability(bool bAvailable);

    UFUNCTION(BlueprintPure, Category = "UI")
    UCombatStateViewModel* GetCombatVM() const { return CombatVM; }

    UFUNCTION(BlueprintPure, Category = "UI")
    UHealthViewModel* GetHealthVM() const { return HealthVM; }

    UFUNCTION(BlueprintPure, Category = "UI")
    UAmmoViewModel* GetAmmoVM() const { return AmmoVM; }

    UFUNCTION(BlueprintPure, Category = "UI")
    UCoverViewModel* GetCoverVM() const { return CoverVM; }

public:
    /** HUD widget class (soft reference) to create for this local player */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSoftClassPtr<UUserWidget> HUDClass;

private:
    UPROPERTY()
    UUserWidget* HUDInstance = nullptr;

    UPROPERTY()
    UCombatStateViewModel* CombatVM = nullptr;

    UPROPERTY()
    UHealthViewModel* HealthVM = nullptr;

    UPROPERTY()
    UAmmoViewModel* AmmoVM = nullptr;

    UPROPERTY()
    UCoverViewModel* CoverVM = nullptr;
};
