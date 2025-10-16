#include "UI/BoxPlayerStatusWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

UBoxPlayerStatusWidget::UBoxPlayerStatusWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UBoxPlayerStatusWidget::RebuildWidget()
{
    WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));

    UTextBlock* TextWidget = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
    InitializeText(TextWidget);

    WidgetTree->RootWidget = TextWidget;
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

