#include "CombatStateWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
// ViewModels
#include "UI/CombatStateViewModel.h"
#include "UI/HealthViewModel.h"
#include "UI/AmmoViewModel.h"
#include "UI/CoverViewModel.h"

void UCombatStateWidget::NativeConstruct()
{
	Super::NativeConstruct();

    if (HealthVM)
    {
        HandleHealthChanged(HealthVM->Current, HealthVM->Max);
    }

    if (AmmoVM)
    {
        HandleAmmoChanged(AmmoVM->Current, AmmoVM->Max);
    }

    if (CoverVM)
    {
        HandleCoverAvailabilityChanged(CoverVM->bAvailable);
    }
}

void UCombatStateWidget::UpdateStateText(FText InText)
{
    if (StateTextBlock)
    {
        StateTextBlock->SetText(InText);
    }
}

void UCombatStateWidget::InitializeViewModels(UCombatStateViewModel* InCombatVM, UHealthViewModel* InHealthVM, UAmmoViewModel* InAmmoVM, UCoverViewModel* InCoverVM)
{
    if (CombatVM != InCombatVM)
    {
        CombatVM = InCombatVM;
    }

    if (HealthVM && HealthVM != InHealthVM)
    {
        HealthVM->OnHealthChanged.RemoveDynamic(this, &UCombatStateWidget::HandleHealthChanged);
    }
    HealthVM = InHealthVM;

    // Auto-bind to health changes if a ProgressBar has been bound via UMG
    if (HealthVM)
    {
        // Clean any prior binding
        HealthVM->OnHealthChanged.RemoveDynamic(this, &UCombatStateWidget::HandleHealthChanged);
        HealthVM->OnHealthChanged.AddDynamic(this, &UCombatStateWidget::HandleHealthChanged);

        // Initialize current value to the bar (if available)
        HandleHealthChanged(HealthVM->Current, HealthVM->Max);
    }

    if (AmmoVM && AmmoVM != InAmmoVM)
    {
        AmmoVM->OnAmmoChanged.RemoveDynamic(this, &UCombatStateWidget::HandleAmmoChanged);
    }
    AmmoVM = InAmmoVM;
    if (AmmoVM)
    {
        AmmoVM->OnAmmoChanged.RemoveDynamic(this, &UCombatStateWidget::HandleAmmoChanged);
        AmmoVM->OnAmmoChanged.AddDynamic(this, &UCombatStateWidget::HandleAmmoChanged);
        HandleAmmoChanged(AmmoVM->Current, AmmoVM->Max);
    }

    if (CoverVM && CoverVM != InCoverVM)
    {
        CoverVM->OnCoverAvailabilityChanged.RemoveDynamic(this, &UCombatStateWidget::HandleCoverAvailabilityChanged);
    }
    CoverVM = InCoverVM;
    if (CoverVM)
    {
        CoverVM->OnCoverAvailabilityChanged.RemoveDynamic(this, &UCombatStateWidget::HandleCoverAvailabilityChanged);
        CoverVM->OnCoverAvailabilityChanged.AddDynamic(this, &UCombatStateWidget::HandleCoverAvailabilityChanged);
        HandleCoverAvailabilityChanged(CoverVM->bAvailable);
    }
    OnViewModelsReady();
}

void UCombatStateWidget::NativeDestruct()
{
    if (HealthVM)
    {
        HealthVM->OnHealthChanged.RemoveDynamic(this, &UCombatStateWidget::HandleHealthChanged);
    }
    if (AmmoVM)
    {
        AmmoVM->OnAmmoChanged.RemoveDynamic(this, &UCombatStateWidget::HandleAmmoChanged);
    }
    if (CoverVM)
    {
        CoverVM->OnCoverAvailabilityChanged.RemoveDynamic(this, &UCombatStateWidget::HandleCoverAvailabilityChanged);
    }
    Super::NativeDestruct();
}

void UCombatStateWidget::HandleHealthChanged(float Current, float Max)
{
    if (HPBar)
    {
        const float Percent = (Max > 0.f) ? (Current / Max) : 0.f;
        HPBar->SetPercent(Percent);
    }
    if (HealthValueTextBlock)
    {
        const int32 CurrentInt = FMath::RoundToInt(Current);
        const int32 MaxInt = FMath::RoundToInt(Max);
        const FString HealthString = FString::Printf(TEXT("%d / %d"), CurrentInt, MaxInt);
        HealthValueTextBlock->SetText(FText::FromString(HealthString));
    }
}

void UCombatStateWidget::HandleAmmoChanged(int32 Current, int32 Max)
{
    if (AmmoTextBlock)
    {
        const int32 SafeCurrent = FMath::Max(0, Current);
        FString AmmoString;
        if (Max > 0)
        {
            AmmoString = FString::Printf(TEXT("%02d / %02d"), SafeCurrent, Max);
        }
        else
        {
            AmmoString = FString::Printf(TEXT("%02d / INF"), SafeCurrent);
        }
        AmmoTextBlock->SetText(FText::FromString(AmmoString));
    }
}

void UCombatStateWidget::HandleCoverAvailabilityChanged(bool bAvailable)
{
    if (CrosshairTextBlock)
    {
        const FLinearColor Color = bAvailable ? FLinearColor(0.2f, 0.8f, 0.2f, 1.f) : FLinearColor::White;
        CrosshairTextBlock->SetColorAndOpacity(FSlateColor(Color));
    }
    if (CoverHintTextBlock)
    {
        CoverHintTextBlock->SetVisibility(bAvailable ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }
}
