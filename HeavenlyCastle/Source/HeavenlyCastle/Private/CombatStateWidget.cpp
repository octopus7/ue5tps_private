#include "CombatStateWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
// ViewModels
#include "UI/CombatStateViewModel.h"
#include "UI/HealthViewModel.h"
#include "UI/AmmoViewModel.h"
#include "UI/CoverViewModel.h"
#include "Components/ProgressBar.h"

void UCombatStateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass());
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = Root;

    //=== Top-right info (Ammo + Combat State)
    UVerticalBox* TopRightBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    Root->AddChild(TopRightBox);
    if (UCanvasPanelSlot* TopRightSlot = Cast<UCanvasPanelSlot>(TopRightBox->Slot))
    {
        TopRightSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
        TopRightSlot->SetAlignment(FVector2D(1.f, 0.f));
        TopRightSlot->SetAutoSize(true);
        TopRightSlot->SetPosition(FVector2D(-30.f, 30.f));
    }

    AmmoTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    AmmoTextBlock->SetText(FText::FromString(TEXT("00 / 00")));
    AmmoTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    AmmoTextBlock->SetShadowOffset(FVector2D(1.f, 1.f));
    AmmoTextBlock->SetJustification(ETextJustify::Right);
    if (UVerticalBoxSlot* AmmoSlot = TopRightBox->AddChildToVerticalBox(AmmoTextBlock))
    {
        AmmoSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
    }

    StateTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    StateTextBlock->SetText(FText::FromString(TEXT("State")));
    StateTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.1f, 1.f)));
    StateTextBlock->SetShadowOffset(FVector2D(1.f, 1.f));
    StateTextBlock->SetJustification(ETextJustify::Right);
    TopRightBox->AddChildToVerticalBox(StateTextBlock);

    //=== Center crosshair + cover hint
    UVerticalBox* CenterBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    Root->AddChild(CenterBox);
    if (UCanvasPanelSlot* CenterSlot = Cast<UCanvasPanelSlot>(CenterBox->Slot))
    {
        CenterSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
        CenterSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        CenterSlot->SetAutoSize(true);
    }

    CrosshairTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    CrosshairTextBlock->SetText(FText::FromString(TEXT("+")));
    {
        FSlateFontInfo FontInfo = CrosshairTextBlock->GetFont();
        FontInfo.Size = 36;
        CrosshairTextBlock->SetFont(FontInfo);
    }
    CrosshairTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    CrosshairTextBlock->SetShadowOffset(FVector2D(1.f, 1.f));
    if (UVerticalBoxSlot* CrosshairSlot = CenterBox->AddChildToVerticalBox(CrosshairTextBlock))
    {
        CrosshairSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
    }

    CoverHintTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    CoverHintTextBlock->SetText(FText::FromString(TEXT("COVER")));
    {
        FSlateFontInfo FontInfo = CoverHintTextBlock->GetFont();
        FontInfo.Size = 18;
        CoverHintTextBlock->SetFont(FontInfo);
    }
    CoverHintTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.9f, 0.2f, 1.f)));
    CoverHintTextBlock->SetShadowOffset(FVector2D(1.f, 1.f));
    CoverHintTextBlock->SetJustification(ETextJustify::Center);
    CoverHintTextBlock->SetVisibility(ESlateVisibility::Collapsed);
    if (UVerticalBoxSlot* CoverSlot = CenterBox->AddChildToVerticalBox(CoverHintTextBlock))
    {
        CoverSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
    }

    //=== Bottom-center health bar
    UVerticalBox* BottomBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    Root->AddChild(BottomBox);
    if (UCanvasPanelSlot* BottomSlot = Cast<UCanvasPanelSlot>(BottomBox->Slot))
    {
        BottomSlot->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
        BottomSlot->SetAlignment(FVector2D(0.5f, 1.f));
        BottomSlot->SetAutoSize(true);
        BottomSlot->SetPosition(FVector2D(0.f, -80.f));
    }

    UProgressBar* HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
    HealthBar->SetPercent(1.f);
    HPBar = HealthBar;
    if (UVerticalBoxSlot* HealthBarSlot = BottomBox->AddChildToVerticalBox(HealthBar))
    {
        HealthBarSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
        HealthBarSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
    }

    HealthValueTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    HealthValueTextBlock->SetText(FText::FromString(TEXT("100 / 100")));
    HealthValueTextBlock->SetJustification(ETextJustify::Center);
    HealthValueTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    HealthValueTextBlock->SetShadowOffset(FVector2D(1.f, 1.f));
    BottomBox->AddChildToVerticalBox(HealthValueTextBlock);
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
