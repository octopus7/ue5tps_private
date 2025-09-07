#include "CombatStateWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
// ViewModels
#include "UI/CombatStateViewModel.h"
#include "UI/HealthViewModel.h"
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

	StateTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Root->AddChild(StateTextBlock);

if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(StateTextBlock->Slot))
{
	// Top-right corner with some padding
	CanvasSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
	CanvasSlot->SetAlignment(FVector2D(1.f, 0.f));
	CanvasSlot->SetAutoSize(true);
	CanvasSlot->SetPosition(FVector2D(-20.f, 20.f));
}

	StateTextBlock->SetText(FText::FromString(TEXT("State")));
	StateTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.1f, 1.f)));
}

void UCombatStateWidget::UpdateStateText(FText InText)
{
    if (StateTextBlock)
    {
        StateTextBlock->SetText(InText);
    }
}

void UCombatStateWidget::InitializeViewModels(UCombatStateViewModel* InCombatVM, UHealthViewModel* InHealthVM)
{
    CombatVM = InCombatVM;
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
    OnViewModelsReady();
}

void UCombatStateWidget::NativeDestruct()
{
    if (HealthVM)
    {
        HealthVM->OnHealthChanged.RemoveDynamic(this, &UCombatStateWidget::HandleHealthChanged);
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
}
