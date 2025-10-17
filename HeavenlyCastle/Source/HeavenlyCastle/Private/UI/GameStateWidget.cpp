#include "UI/GameStateWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"

UGameStateWidget::UGameStateWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsCoverAvailable(false)
    , CachedMagazineAmmo(0)
    , CachedReserveAmmo(0)
{
}

TSharedRef<SWidget> UGameStateWidget::RebuildWidget()
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
            OverlaySlot->SetVerticalAlignment(VAlign_Bottom);
            OverlaySlot->SetPadding(FMargin(0.f, 0.f, 0.f, 48.f));
        }
    }

    CoverStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CoverStatus"));
    InitializeCoverText(CoverStatusText);
    if (Container && CoverStatusText)
    {
        if (UVerticalBoxSlot* CoverSlot = Container->AddChildToVerticalBox(CoverStatusText))
        {
            CoverSlot->SetHorizontalAlignment(HAlign_Center);
        }
    }

    UHorizontalBox* AmmoRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("AmmoRow"));
    if (AmmoRow)
    {
        AmmoRow->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    MagazineText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MagazineText"));
    InitializeAmmoText(MagazineText, true);
    if (AmmoRow && MagazineText)
    {
        if (UHorizontalBoxSlot* MagazineSlot = AmmoRow->AddChildToHorizontalBox(MagazineText))
        {
            MagazineSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
            MagazineSlot->SetHorizontalAlignment(HAlign_Center);
            MagazineSlot->SetVerticalAlignment(VAlign_Center);
        }
    }

    ReserveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReserveText"));
    InitializeAmmoText(ReserveText, false);
    if (AmmoRow && ReserveText)
    {
        if (UHorizontalBoxSlot* ReserveSlot = AmmoRow->AddChildToHorizontalBox(ReserveText))
        {
            ReserveSlot->SetHorizontalAlignment(HAlign_Left);
            ReserveSlot->SetVerticalAlignment(VAlign_Bottom);
        }
    }

    if (Container && AmmoRow)
    {
        if (UVerticalBoxSlot* AmmoSlot = Container->AddChildToVerticalBox(AmmoRow))
        {
            AmmoSlot->SetHorizontalAlignment(HAlign_Center);
        }
    }

    RefreshText();
    RefreshAmmoTexts();

    return WidgetTree->RootWidget->TakeWidget();
}

void UGameStateWidget::InitializeCoverText(UTextBlock* InText)
{
    if (!InText)
    {
        return;
    }

    InText->SetText(FText::GetEmpty());
    InText->SetJustification(ETextJustify::Center);
    InText->SetVisibility(ESlateVisibility::Collapsed);
}

void UGameStateWidget::InitializeAmmoText(UTextBlock* InText, bool bIsMagazine)
{
    if (!InText)
    {
        return;
    }

    const int32 FontSize = bIsMagazine ? 48 : 24;
    FSlateFontInfo FontInfo = InText->Font;
    FontInfo.Size = FontSize;
    InText->SetFont(FontInfo);
    InText->SetText(FText::GetEmpty());
    InText->SetJustification(bIsMagazine ? ETextJustify::Center : ETextJustify::Left);
    InText->SetAutoWrapText(false);
    InText->SetVisibility(ESlateVisibility::HitTestInvisible);
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
        CoverStatusText->SetText(FText::FromString(TEXT("Cover Available")));
        CoverStatusText->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        CoverStatusText->SetText(FText::GetEmpty());
        CoverStatusText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UGameStateWidget::SetAmmoCounts(int32 MagazineAmmo, int32 ReserveAmmo)
{
    const int32 ClampedMagazine = FMath::Max(0, MagazineAmmo);
    const int32 ClampedReserve = FMath::Max(0, ReserveAmmo);

    if (CachedMagazineAmmo == ClampedMagazine && CachedReserveAmmo == ClampedReserve && MagazineText && ReserveText)
    {
        return;
    }

    CachedMagazineAmmo = ClampedMagazine;
    CachedReserveAmmo = ClampedReserve;
    RefreshAmmoTexts();
}

void UGameStateWidget::RefreshAmmoTexts()
{
    if (MagazineText)
    {
        MagazineText->SetText(FText::AsNumber(CachedMagazineAmmo));
    }

    if (ReserveText)
    {
        const FString ReserveString = FString::Printf(TEXT("/ %d"), CachedReserveAmmo);
        ReserveText->SetText(FText::FromString(ReserveString));
    }
}
