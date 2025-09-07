#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatStateWidget.generated.h"

class UProgressBar;
class UCombatStateViewModel;
class UHealthViewModel;

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
    void InitializeViewModels(UCombatStateViewModel* InCombatVM, UHealthViewModel* InHealthVM);

    // Implement in BP to bind to VM events or update initial UI.
    UFUNCTION(BlueprintImplementableEvent, Category="UI")
    void OnViewModelsReady();

private:
    UPROPERTY()
    class UTextBlock* StateTextBlock;

public:
    UPROPERTY(BlueprintReadOnly, Category="UI")
    UCombatStateViewModel* CombatVM = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="UI")
    UHealthViewModel* HealthVM = nullptr;

    // Optional: bind directly to a ProgressBar named HPBar (if present in the widget tree via UMG designer)
    UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
    UProgressBar* HPBar = nullptr;

protected:
    virtual void NativeDestruct() override;

    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);
};
