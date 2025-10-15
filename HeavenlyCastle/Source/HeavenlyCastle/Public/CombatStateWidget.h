#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatStateWidget.generated.h"

class UProgressBar;
class UCombatStateViewModel;
class UHealthViewModel;
class UAmmoViewModel;
class UCoverViewModel;
class UTextBlock;

UCLASS()
class HEAVENLYCASTLE_API UCombatStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable)
    void UpdateStateText(FText InText);

    // Injected by UISessionSubsystem. Widget should NOT query player/subsystem itself.
    UFUNCTION(BlueprintCallable, Category="UI")
    void InitializeViewModels(UCombatStateViewModel* InCombatVM, UHealthViewModel* InHealthVM, UAmmoViewModel* InAmmoVM = nullptr, UCoverViewModel* InCoverVM = nullptr);

    // Implement in BP to bind to VM events or update initial UI.
    UFUNCTION(BlueprintImplementableEvent, Category="UI")
    void OnViewModelsReady();

private:
    UPROPERTY()
    UTextBlock* StateTextBlock = nullptr;

    UPROPERTY()
    UTextBlock* AmmoTextBlock = nullptr;

    UPROPERTY()
    UTextBlock* CrosshairTextBlock = nullptr;

    UPROPERTY()
    UTextBlock* CoverHintTextBlock = nullptr;

    UPROPERTY()
    UTextBlock* HealthValueTextBlock = nullptr;

public:
    UPROPERTY(BlueprintReadOnly, Category="UI")
    UCombatStateViewModel* CombatVM = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="UI")
    UHealthViewModel* HealthVM = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="UI")
    UAmmoViewModel* AmmoVM = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="UI")
    UCoverViewModel* CoverVM = nullptr;

    // Optional: bind directly to a ProgressBar named HPBar (if present in the widget tree via UMG designer)
    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly)
    UProgressBar* HPBar = nullptr;

protected:
    virtual void NativeDestruct() override;

    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

    UFUNCTION()
    void HandleAmmoChanged(int32 Current, int32 Max);

    UFUNCTION()
    void HandleCoverAvailabilityChanged(bool bAvailable);
};
