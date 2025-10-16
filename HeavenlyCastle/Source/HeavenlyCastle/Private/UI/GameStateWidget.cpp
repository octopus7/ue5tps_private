#include "UI/GameStateWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

UGameStateWidget::UGameStateWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UGameStateWidget::RebuildWidget()
{
    WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));

    UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    UVerticalBox* Container = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Container"));
    if (RootCanvas)
    {
        if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(Container))
        {
            CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        }
    }

    UTextBlock* TextWidget = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StateText"));
    InitializeText(TextWidget);
    if (Container)
    {
        Container->AddChildToVerticalBox(TextWidget);
    }

    StateText = TextWidget;

    return WidgetTree->RootWidget->TakeWidget();
}

void UGameStateWidget::InitializeText(UTextBlock* InText)
{
    if (!InText)
    {
        return;
    }

    InText->SetText(FText::FromString(TEXT("GameState")));
    InText->SetJustification(ETextJustify::Center);
}
