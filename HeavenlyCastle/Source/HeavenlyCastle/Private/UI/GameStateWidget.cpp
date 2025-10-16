#include "UI/GameStateWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

UGameStateWidget::UGameStateWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UGameStateWidget::RebuildWidget()
{
    WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));

    UTextBlock* TextWidget = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StateText"));
    InitializeText(TextWidget);

    WidgetTree->RootWidget = TextWidget;
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

