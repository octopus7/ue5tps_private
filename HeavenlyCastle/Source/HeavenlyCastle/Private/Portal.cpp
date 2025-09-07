#include "Portal.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"

APortal::APortal()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->InitBoxExtent(FVector(50.f, 50.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;

    DestinationForwardOffset = 120.f;
    DestinationZOffset = 0.f;
    bDebugDraw = true;
    DebugLineThickness = 2.f;

    TextLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextLabel"));
    TextLabel->SetupAttachment(RootComponent);
    TextLabel->SetHorizontalAlignment(EHTA_Center);
    TextLabel->SetVerticalAlignment(EVRTA_TextCenter);
    TextLabel->SetTextRenderColor(FColor::Yellow);
    LabelHeight = 120.f;
    LabelWorldSize = 30.f;
    TextLabel->SetWorldSize(LabelWorldSize);
    TextLabel->SetRelativeLocation(FVector(0.f, 0.f, LabelHeight));

    bFaceCamera = true;
    bFaceCameraYawOnly = true;
    bSmoothFaceCamera = true;
    FaceCameraInterpSpeed = 8.f;

    bUseDistanceVisibility = true;
    LabelVisibleMinDistance = 0.f;
    LabelVisibleMaxDistance = 3000.f;

    TeleportCooldownSeconds = 3.0f;

    bUseActorCooldown = true;
    bUseGlobalCooldown = false;
}

void APortal::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerBox)
    {
        // Capture original box extent (local/unscaled) and shrink the actual trigger by half for gameplay
        OriginalBoxExtent = TriggerBox->GetUnscaledBoxExtent();
        if (!OriginalBoxExtent.IsNearlyZero())
        {
            TriggerBox->SetBoxExtent(OriginalBoxExtent * 0.5f, true);
        }
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnTriggerBeginOverlap);
    }
}

void APortal::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bDebugDraw && TriggerBox)
    {
        const FVector Center = TriggerBox->GetComponentLocation();
        const FQuat Rot = TriggerBox->GetComponentQuat();
        // Draw using the original box size (before halving), with current component/world scale applied
        FVector WorldExtent;
        if (!OriginalBoxExtent.IsNearlyZero())
        {
            const FVector ScaleAbs = TriggerBox->GetComponentScale().GetAbs();
            WorldExtent = FVector(OriginalBoxExtent.X * ScaleAbs.X,
                                  OriginalBoxExtent.Y * ScaleAbs.Y,
                                  OriginalBoxExtent.Z * ScaleAbs.Z);
        }
        else
        {
            // Fallback to current box extent if original wasn't captured
            WorldExtent = TriggerBox->GetScaledBoxExtent();
        }
        DrawDebugBox(GetWorld(), Center, WorldExtent, Rot, FColor::Cyan, false, 0.f, 0, DebugLineThickness);
    }

    // Face the label toward the active player camera and control visibility by distance
    if (TextLabel)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            FVector CamLoc; FRotator CamRot;
            PC->GetPlayerViewPoint(CamLoc, CamRot);
            {
                const FVector LabelLoc = TextLabel->GetComponentLocation();
                const float CamDist = (CamLoc - LabelLoc).Size();

                // Distance-based visibility
                if (bUseDistanceVisibility)
                {
                    const bool bVisible = (CamDist >= LabelVisibleMinDistance) && (CamDist <= LabelVisibleMaxDistance);
                    TextLabel->SetHiddenInGame(!bVisible);
                }
                else
                {
                    TextLabel->SetHiddenInGame(false);
                }

                if (bFaceCamera)
                {
                    FVector Dir = CamLoc - LabelLoc;
                    if (!Dir.IsNearlyZero())
                    {
                        FRotator TargetRot;
                        bool bHaveTarget = false;
                        if (bFaceCameraYawOnly)
                        {
                            // Keep upright: only yaw towards camera on the XY plane
                            const FVector FlatDir = FVector(Dir.X, Dir.Y, 0.f).GetSafeNormal();
                            if (!FlatDir.IsNearlyZero())
                            {
                                TargetRot = FlatDir.Rotation();
                                TargetRot.Pitch = 0.f;
                                TargetRot.Roll = 0.f;
                                bHaveTarget = true;
                            }
                        }
                        else
                        {
                            TargetRot = FRotationMatrix::MakeFromX(Dir.GetSafeNormal()).Rotator();
                            bHaveTarget = true;
                        }

                        if (bHaveTarget)
                        {
                            if (bSmoothFaceCamera)
                            {
                                const FRotator Current = TextLabel->GetComponentRotation();
                                FRotator NewRot;
                                if (bFaceCameraYawOnly)
                                {
                                    const float Yaw = FMath::FixedTurn(Current.Yaw, TargetRot.Yaw, FaceCameraInterpSpeed * DeltaSeconds * 10.f);
                                    NewRot = FRotator(0.f, Yaw, 0.f);
                                }
                                else
                                {
                                    NewRot = FMath::RInterpTo(Current, TargetRot, DeltaSeconds, FaceCameraInterpSpeed);
                                }
                                TextLabel->SetWorldRotation(NewRot);
                            }
                            else
                            {
                                TextLabel->SetWorldRotation(TargetRot);
                            }
                        }
                    }
                }
            }
        }
    }
}

void APortal::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (TextLabel)
    {
        TextLabel->SetWorldSize(LabelWorldSize);
        TextLabel->SetRelativeLocation(FVector(0.f, 0.f, LabelHeight));
    }
    UpdateLabelText();
}

void APortal::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult& /*SweepResult*/)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Prevent teleporting while on cooldown (to stop cyclic ping-pong)
    if (bUseActorCooldown && IsActorOnCooldown(OtherActor))
    {
        return;
    }

    if (bUseGlobalCooldown && IsOnCooldown())
    {
        return;
    }

    APortal* Dest = FindDestinationPortal();
    if (!Dest)
    {
        return;
    }

	// Compute safe destination just outside the destination portal box
	const FVector DestForward = Dest->GetActorForwardVector();
	const FVector DestLocation = Dest->GetActorLocation() + DestForward * Dest->DestinationForwardOffset + FVector(0.f, 0.f, Dest->DestinationZOffset);
	const FRotator DestRotation = Dest->GetActorRotation();

    // Teleport the overlapping actor
    OtherActor->TeleportTo(DestLocation, DestRotation, false, true);

    // Start cooldown on both source and destination portals
    if (bUseActorCooldown)
    {
        StartActorCooldown(OtherActor);
        Dest->StartActorCooldown(OtherActor);
    }

    if (bUseGlobalCooldown)
    {
        StartCooldown();
        Dest->StartCooldown();
    }
}

APortal* APortal::FindDestinationPortal() const
{
    if (TargetPortalId.IsEmpty())
    {
        return nullptr;
    }

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass(), Found);
    for (AActor* Act : Found)
    {
        if (APortal* P = Cast<APortal>(Act))
        {
            if (P != this && P->PortalId.Equals(TargetPortalId, ESearchCase::IgnoreCase))
            {
                return P;
            }
        }
    }
    return nullptr;
}

void APortal::UpdateLabelText()
{
    if (!TextLabel)
    {
        return;
    }

    const FString SelfId = PortalId.IsEmpty() ? TEXT("?") : PortalId;
    const FString DestId = TargetPortalId.IsEmpty() ? TEXT("?") : TargetPortalId;
    const FString Label = FString::Printf(TEXT("%s -> %s"), *SelfId, *DestId);
    TextLabel->SetText(FText::FromString(Label));
}

bool APortal::IsOnCooldown() const
{
    if (!GetWorld())
    {
        return false;
    }
    const double Now = GetWorld()->GetTimeSeconds();
    return Now < CooldownEndTime;
}

void APortal::StartCooldown()
{
    if (!GetWorld())
    {
        return;
    }
    CooldownEndTime = GetWorld()->GetTimeSeconds() + TeleportCooldownSeconds;
}

bool APortal::IsActorOnCooldown(AActor* Actor)
{
    if (!Actor || !GetWorld())
    {
        return false;
    }

    const double Now = GetWorld()->GetTimeSeconds();
    PruneActorCooldowns();

    TWeakObjectPtr<AActor> Key(Actor);
    if (double* Expire = ActorCooldowns.Find(Key))
    {
        if (Now < *Expire)
        {
            return true;
        }
        // Expired
        ActorCooldowns.Remove(Key);
    }
    return false;
}

void APortal::StartActorCooldown(AActor* Actor)
{
    if (!Actor || !GetWorld())
    {
        return;
    }
    const double Until = GetWorld()->GetTimeSeconds() + TeleportCooldownSeconds;
    ActorCooldowns.Add(TWeakObjectPtr<AActor>(Actor), Until);
}

void APortal::PruneActorCooldowns()
{
    if (!GetWorld())
    {
        return;
    }
    const double Now = GetWorld()->GetTimeSeconds();
    for (auto It = ActorCooldowns.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid() || Now >= It.Value())
        {
            It.RemoveCurrent();
        }
    }
}
