#include "UI/UISessionSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "UI/CombatStateViewModel.h"
#include "UI/HealthViewModel.h"
#include "CombatStateWidget.h"

void UUISessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Create view models
    CombatVM = NewObject<UCombatStateViewModel>(this);
    HealthVM = NewObject<UHealthViewModel>(this);
}

void UUISessionSubsystem::Deinitialize()
{
    HUDInstance = nullptr;
    CombatVM = nullptr;
    HealthVM = nullptr;
    Super::Deinitialize();
}

void UUISessionSubsystem::EnsureHUD(APlayerController* OwningPC)
{
    if (HUDInstance || !OwningPC)
    {
        return;
    }

    if (!OwningPC->IsLocalController())
    {
        return;
    }

    TSubclassOf<UUserWidget> ClassToUse = nullptr;
    if (!HUDClass.IsNull())
    {
        ClassToUse = HUDClass.LoadSynchronous();
    }
    if (!ClassToUse)
    {
        return;
    }

    HUDInstance = CreateWidget<UUserWidget>(OwningPC, ClassToUse);
    if (HUDInstance)
    {
        HUDInstance->AddToViewport();
        // Inject ViewModels if widget supports it
        if (UCombatStateWidget* CSW = Cast<UCombatStateWidget>(HUDInstance))
        {
            CSW->InitializeViewModels(CombatVM, HealthVM);
        }
    }
}

void UUISessionSubsystem::EnsureHUDWithClass(APlayerController* OwningPC, TSubclassOf<UUserWidget> InClass)
{
    if (HUDInstance || !OwningPC || !OwningPC->IsLocalController())
    {
        return;
    }
    TSubclassOf<UUserWidget> ClassToUse = InClass;
    if (!ClassToUse)
    {
        // Fallback to configured class if none provided
        if (!HUDClass.IsNull())
        {
            ClassToUse = HUDClass.LoadSynchronous();
        }
    }
    if (!ClassToUse)
    {
        return;
    }
    HUDInstance = CreateWidget<UUserWidget>(OwningPC, ClassToUse);
    if (HUDInstance)
    {
        HUDInstance->AddToViewport();
        if (UCombatStateWidget* CSW = Cast<UCombatStateWidget>(HUDInstance))
        {
            CSW->InitializeViewModels(CombatVM, HealthVM);
        }
    }
}

void UUISessionSubsystem::PushCombatState(ECombatState NewState)
{
    if (!CombatVM)
    {
        CombatVM = NewObject<UCombatStateViewModel>(this);
    }
    CombatVM->SetCombatState(NewState);
}

void UUISessionSubsystem::PushHealth(float Current, float Max)
{
    if (!HealthVM)
    {
        HealthVM = NewObject<UHealthViewModel>(this);
    }
    HealthVM->SetHealth(Current, Max);
}
