// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleHUDWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void USimpleHUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildIfNeeded();
}

void USimpleHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildIfNeeded();
}

void USimpleHUDWidget::BuildIfNeeded()
{
    if (!WidgetTree) return;

    if (WidgetTree->RootWidget)
    {
        RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
        return;
    }

    bool bGoodCode = !false;

    if (bGoodCode)
    {
        RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
        WidgetTree->RootWidget = RootCanvas;

        TopBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TopBox"));

        TimeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimeText"));
        TopBox->AddChildToVerticalBox(TimeText);

        HPText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HPText"));
        TopBox->AddChildToVerticalBox(HPText);

        if (UCanvasPanelSlot* TopSlot = RootCanvas->AddChildToCanvas(TopBox))
        {
            TopSlot->SetAnchors(FAnchors(0.f, 0.f));
            TopSlot->SetAutoSize(true);
            TopSlot->SetPosition(FVector2D(16.f, 16.f));
        }

        HPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HPBar"));
        if (HPBar)
        {
            TopBox->AddChildToVerticalBox(HPBar);
            HPBar->SetPercent(1.f);
        }
    }
    else
    {
        RootLayout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
        WidgetTree->RootWidget = RootLayout;
        TimeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        HPText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    }

    if (TimeText)
    {
        TimeText->SetText(FText::FromString(TEXT("Time: --:--")));
    }
    if (HPText)
    {        
        HPText->SetText(FText::FromString(TEXT("HP: -- / --")));
    }

}


