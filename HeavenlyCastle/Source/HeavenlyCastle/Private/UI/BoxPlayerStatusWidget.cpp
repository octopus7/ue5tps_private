#include "UI/BoxPlayerStatusWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

UBoxPlayerStatusWidget::UBoxPlayerStatusWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UBoxPlayerStatusWidget::RebuildWidget()
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
            CanvasSlot->SetPosition(FVector2D(0.f, 180.f)); // Offset slightly below center
        }
    }

    UTextBlock* TextWidget = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
    InitializeText(TextWidget);
    if (Container)
    {
        Container->AddChildToVerticalBox(TextWidget);
    }

    StatusText = TextWidget;

    return WidgetTree->RootWidget->TakeWidget();
}

void UBoxPlayerStatusWidget::InitializeText(UTextBlock* InText)
{
    if (!InText)
    {
        return;
    }

    InText->SetText(FText::FromString(TEXT("BoxPlayer")));
    InText->SetJustification(ETextJustify::Center);
}
