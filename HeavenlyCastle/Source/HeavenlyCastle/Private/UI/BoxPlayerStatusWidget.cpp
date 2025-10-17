#include "UI/BoxPlayerStatusWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Layout/Margin.h"

UBoxPlayerStatusWidget::UBoxPlayerStatusWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UBoxPlayerStatusWidget::RebuildWidget()
{
    WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));

    UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
    WidgetTree->RootWidget = RootOverlay;

    UVerticalBox* Container = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Container"));
    if (RootOverlay)
    {
        if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(Container))
        {
            OverlaySlot->SetHorizontalAlignment(HAlign_Center);
            OverlaySlot->SetVerticalAlignment(VAlign_Center);
            OverlaySlot->SetPadding(FMargin(0.f, 180.f, 0.f, 0.f)); // Offset slightly below center
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
