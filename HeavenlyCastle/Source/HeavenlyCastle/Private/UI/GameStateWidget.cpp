#include "UI/GameStateWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

UGameStateWidget::UGameStateWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsCoverAvailable(false)
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
            CanvasSlot->SetPosition(FVector2D(0.f, 180.f)); // Offset slightly below center
        }
    }

    UTextBlock* TextWidget = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StateText"));
    InitializeText(TextWidget);
    if (Container)
    {
        Container->AddChildToVerticalBox(TextWidget);
    }

    CoverStatusText = TextWidget;
    RefreshText();

    return WidgetTree->RootWidget->TakeWidget();
}

void UGameStateWidget::InitializeText(UTextBlock* InText)
{
    if (!InText)
    {
        return;
    }

    InText->SetText(FText::GetEmpty());
    InText->SetJustification(ETextJustify::Center);
    InText->SetVisibility(ESlateVisibility::Collapsed);
}

void UGameStateWidget::SetCoverAvailable(bool bAvailable)
{
    if (bIsCoverAvailable == bAvailable)
    {
        return;
    }

    bIsCoverAvailable = bAvailable;
    RefreshText();
}

void UGameStateWidget::RefreshText()
{
    if (!CoverStatusText)
    {
        return;
    }

    if (bIsCoverAvailable)
    {
        CoverStatusText->SetText(FText::FromString(TEXT("커버 가능")));
        CoverStatusText->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        CoverStatusText->SetText(FText::GetEmpty());
        CoverStatusText->SetVisibility(ESlateVisibility::Collapsed);
    }
}
