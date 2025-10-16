#include "UI/CombatHUDManager.h"

#include "Blueprint/UserWidget.h"

UCombatHUDManager::UCombatHUDManager()
{
    bIsFocusable = false;
}

void UCombatHUDManager::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    EnsurePrimaryWidget();
}

UCommonActivatableWidget* UCombatHUDManager::EnsurePrimaryWidget()
{
    if (PrimaryWidgetInstance || !PrimaryWidgetClass)
    {
        return PrimaryWidgetInstance;
    }

    if (APlayerController* OwningController = GetOwningPlayer())
    {
        PrimaryWidgetInstance = CreateWidget<UCommonActivatableWidget>(OwningController, PrimaryWidgetClass);
    }

    return PrimaryWidgetInstance;
}

