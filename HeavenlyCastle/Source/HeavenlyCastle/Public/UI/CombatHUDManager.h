#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CombatHUDManager.generated.h"

/**
 * Base manager that will orchestrate HUD fragments via Common UI.
 */
UCLASS(Abstract, Blueprintable)
class HEAVENLYCASTLE_API UCombatHUDManager : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UCombatHUDManager();

    UFUNCTION(BlueprintCallable, Category = "Combat HUD")
    UCommonActivatableWidget* EnsurePrimaryWidget();

    UFUNCTION(BlueprintPure, Category = "Combat HUD")
    UCommonActivatableWidget* GetPrimaryWidget() const { return PrimaryWidgetInstance; }

protected:
    virtual void NativeOnInitialized() override;

    // For now we only keep a single HUD fragment reference.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat HUD")
    TSubclassOf<UCommonActivatableWidget> PrimaryWidgetClass;

private:
    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> PrimaryWidgetInstance;
};
