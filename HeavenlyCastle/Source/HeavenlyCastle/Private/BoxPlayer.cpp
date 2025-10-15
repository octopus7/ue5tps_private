#include "BoxPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Cover/Runtime/CoverControllerComponent.h"
#include "Cover/Runtime/CoverCameraComponent.h"

ABoxPlayer::ABoxPlayer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    DefaultCameraArmLength = 400.f;
    DefaultCameraSocketOffset = FVector(0.f, 50.f, 70.f);
    AimingCameraArmLength = 200.f;
    AimingCameraSocketOffset = FVector(0.f, 70.f, 60.f);
    CameraInterpSpeed = 20.f;
    DefaultWalkSpeed = 500.f;
    CameraAimTraceDistance = 50000.f;
    TimeBetweenShots = 0.1f;
    WeaponSocketName = FName("Weapon");
    SpawnedWeapon = nullptr;
    bIsAiming = false;
    CoverExitForwardThreshold = 0.6f;
    CoverCameraOffset = FVector::ZeroVector;

    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
        MovementComp->JumpZVelocity = 700.f;
        MovementComp->AirControl = 0.35f;
        MovementComp->MaxWalkSpeed = DefaultWalkSpeed;
        MovementComp->MinAnalogWalkSpeed = 20.f;
        MovementComp->BrakingDecelerationWalking = 2000.f;
        MovementComp->BrakingDecelerationFalling = 1500.f;
    }

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = DefaultCameraArmLength;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = DefaultCameraSocketOffset;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    CoverController = CreateDefaultSubobject<UCoverControllerComponent>(TEXT("CoverController"));
    CoverCamera = CreateDefaultSubobject<UCoverCameraComponent>(TEXT("CoverCamera"));

    ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
    ProjectileSpawnPoint->SetupAttachment(GetMesh());
}

void ABoxPlayer::BeginPlay()
{
    Super::BeginPlay();

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = DefaultCameraArmLength;
        CameraBoom->SocketOffset = DefaultCameraSocketOffset;
    }

    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = DefaultWalkSpeed;
    }

    ApplyRotationSettings();

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (DefaultMappingContext)
                {
                    Subsystem->AddMappingContext(DefaultMappingContext, 0);
                }
            }
        }
    }

    if (WeaponBlueprint && GetMesh() && GetMesh()->DoesSocketExist(WeaponSocketName))
    {
        if (UWorld* World = GetWorld())
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();

            SpawnedWeapon = World->SpawnActor<AActor>(WeaponBlueprint, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
            if (SpawnedWeapon)
            {
                FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
                SpawnedWeapon->AttachToComponent(GetMesh(), AttachmentRules, WeaponSocketName);
            }
        }
    }
    else if (WeaponBlueprint)
    {
        UE_LOG(LogTemp, Warning, TEXT("BoxPlayer: Weapon socket '%s'를 찾을 수 없습니다."), *WeaponSocketName.ToString());
    }

    if (CoverController)
    {
        CoverController->OnStateChanged.AddDynamic(this, &ABoxPlayer::OnCoverStateChanged);
        CoverController->SetAiming(bIsAiming);
    }

    if (CoverCamera)
    {
        CoverCamera->OnCameraOffset.AddDynamic(this, &ABoxPlayer::OnCoverCameraOffsetUpdated);
    }
}

void ABoxPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCamera(DeltaTime);
}

void ABoxPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABoxPlayer::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABoxPlayer::Move);
        }

        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABoxPlayer::Look);
        }

        if (AimAction)
        {
            EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABoxPlayer::AimStarted);
            EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABoxPlayer::AimStopped);
        }

        if (CoverAction)
        {
            EnhancedInputComponent->BindAction(CoverAction, ETriggerEvent::Started, this, &ABoxPlayer::HandleCoverAction);
        }

        if (FireAction)
        {
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABoxPlayer::StartFire);
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABoxPlayer::StopFire);
        }
    }
}

void ABoxPlayer::Jump()
{
    if (CoverController && IsInCover())
    {
        CoverController->ExitCover();
    }

    Super::Jump();
}

void ABoxPlayer::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (!Controller)
    {
        return;
    }

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    const FVector WorldInput = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;

    if (CoverController && IsInCover())
    {
        const float ForwardAxis = MovementVector.Y;
        if (ForwardAxis < -CoverExitForwardThreshold)
        {
            CoverController->ExitCover();
            return;
        }

        const FVector Tangent = CoverController->GetCurrentTangent();
        const FVector Normal = CoverController->GetCurrentNormal();
        float SlideAxis = 0.f;
        if (!Tangent.IsNearlyZero())
        {
            const float InputMagnitude = FMath::Min(WorldInput.Size(), 1.f);
            const FVector NormalizedInput = WorldInput.GetSafeNormal();

            FVector SlideDirection = Tangent;
            if (Normal.IsNearlyZero())
            {
                SlideAxis = FVector::DotProduct(NormalizedInput, SlideDirection) * InputMagnitude;
            }
            else
            {
                const FVector NormalizedNormal = Normal.GetSafeNormal();
                const FVector ProjectedInput = NormalizedInput - FVector::DotProduct(NormalizedInput, NormalizedNormal) * NormalizedNormal;

                const FVector InputRight = FVector::CrossProduct(NormalizedNormal, ProjectedInput).GetSafeNormal();
                if (!InputRight.IsNearlyZero() && FVector::DotProduct(InputRight, Tangent) < 0.f)
                {
                    SlideDirection *= -1.f;
                }

                SlideAxis = FVector::DotProduct(ProjectedInput.GetSafeNormal(), SlideDirection) * InputMagnitude;
            }
        }

        CoverController->AddSlideInput(SlideAxis);
        return;
    }

    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
}

void ABoxPlayer::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ABoxPlayer::AimStarted()
{
    bIsAiming = true;
    if (CoverController)
    {
        CoverController->SetAiming(true);
        if (IsInCover())
        {
            CoverController->SetPreferRightPeek(true);
        }
    }
    ApplyRotationSettings();
}

void ABoxPlayer::AimStopped()
{
    bIsAiming = false;
    if (CoverController)
    {
        CoverController->SetPreferRightPeek(false);
        CoverController->SetAiming(false);
    }
    ApplyRotationSettings();
}

void ABoxPlayer::StartFire()
{
    if (IsInCover() && CoverController)
    {
        switch (CoverController->State)
        {
        case ESimpleCoverState::PeekSideLeft:
        case ESimpleCoverState::PeekSideRight:
        case ESimpleCoverState::PeekOver:
            break;
        default:
            return;
        }
    }

    Fire();

    if (TimeBetweenShots > 0.f)
    {
        GetWorldTimerManager().SetTimer(AutomaticFireHandle, this, &ABoxPlayer::Fire, TimeBetweenShots, true);
    }
}

void ABoxPlayer::StopFire()
{
    GetWorldTimerManager().ClearTimer(AutomaticFireHandle);
}

void ABoxPlayer::Fire()
{
    if (!ProjectileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("BoxPlayer: ProjectileClass가 설정되지 않았습니다."));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector SpawnLocation = GetActorLocation();
    FRotator SpawnRotation = GetActorRotation();

    bool bHasValidMuzzle = false;
    if (SpawnedWeapon)
    {
        if (UMeshComponent* MuzzleMesh = SpawnedWeapon->FindComponentByClass<UMeshComponent>())
        {
            static const FName MuzzleSocketName("Muzzle");
            if (MuzzleMesh->DoesSocketExist(MuzzleSocketName))
            {
                SpawnLocation = MuzzleMesh->GetSocketLocation(MuzzleSocketName);
                SpawnRotation = MuzzleMesh->GetSocketRotation(MuzzleSocketName);
                bHasValidMuzzle = true;
            }
        }
    }

    if (!bHasValidMuzzle && ProjectileSpawnPoint)
    {
        SpawnLocation = ProjectileSpawnPoint->GetComponentLocation();
        SpawnRotation = ProjectileSpawnPoint->GetComponentRotation();
    }

    FVector AimPoint = SpawnLocation + SpawnRotation.Vector() * CameraAimTraceDistance;
    const FVector AimDirection = CalculateCameraAimDirection(SpawnLocation, AimPoint);
    const FVector FinalDirection = AimDirection.IsNearlyZero() ? SpawnRotation.Vector() : AimDirection;
    const FRotator FinalRotation = FinalDirection.Rotation();

    DrawDebugLine(World, SpawnLocation, AimPoint, FColor::Cyan, false, 1.0f, 0, 1.5f);

    if (CoverController && !CoverController->IsMuzzleClear(SpawnLocation, AimPoint))
    {
        UE_LOG(LogTemp, Verbose, TEXT("BoxPlayer: Fire blocked by cover occlusion"));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    if (AActor* SpawnedProjectile = World->SpawnActor<AActor>(ProjectileClass, SpawnLocation, FinalRotation, SpawnParams))
    {
        if (FireEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, FireEffect, SpawnLocation, FinalRotation);
        }

        if (UProjectileMovementComponent* ProjectileMovement = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>())
        {
            UE_LOG(LogTemp, Verbose, TEXT("BoxPlayer: Projectile speed %f"), ProjectileMovement->InitialSpeed);
        }
    }
}

FVector ABoxPlayer::CalculateCameraAimDirection(const FVector& MuzzleLocation, FVector& OutAimPoint)
{
    OutAimPoint = MuzzleLocation;

    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }

    const float TraceDistance = CameraAimTraceDistance > 0.f ? CameraAimTraceDistance : 10000.f;

    FVector ViewLocation = MuzzleLocation;
    FRotator ViewRotation = GetActorRotation();

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
    }
    else if (FollowCamera)
    {
        ViewLocation = FollowCamera->GetComponentLocation();
        ViewRotation = FollowCamera->GetComponentRotation();
    }

    const FVector TraceStart = ViewLocation;
    const FVector TraceEnd = TraceStart + ViewRotation.Vector() * TraceDistance;

    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(BoxPlayerFireTrace), true, this);
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);
    if (SpawnedWeapon)
    {
        TraceParams.AddIgnoredActor(SpawnedWeapon);
    }

    FHitResult Hit;
    if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
    {
        OutAimPoint = Hit.ImpactPoint;
        return (Hit.ImpactPoint - MuzzleLocation).GetSafeNormal();
    }

    OutAimPoint = TraceEnd;
    return (TraceEnd - MuzzleLocation).GetSafeNormal();
}

void ABoxPlayer::UpdateCamera(float DeltaTime)
{
    if (!CameraBoom)
    {
        return;
    }

    const float DesiredArmLength = bIsAiming ? AimingCameraArmLength : DefaultCameraArmLength;
    FVector DesiredSocketOffset = bIsAiming ? AimingCameraSocketOffset : DefaultCameraSocketOffset;
    DesiredSocketOffset += CoverCameraOffset;

    const float EffectiveInterpSpeed = FMath::Max(CameraInterpSpeed, 0.f);

    if (EffectiveInterpSpeed > 0.f)
    {
        CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, DesiredArmLength, DeltaTime, EffectiveInterpSpeed);
        CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, DesiredSocketOffset, DeltaTime, EffectiveInterpSpeed);
    }
    else
    {
        CameraBoom->TargetArmLength = DesiredArmLength;
        CameraBoom->SocketOffset = DesiredSocketOffset;
    }
}

void ABoxPlayer::ApplyRotationSettings()
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        if (bIsAiming)
        {
            MovementComp->bOrientRotationToMovement = false;
            bUseControllerRotationYaw = true;
        }
        else
        {
            MovementComp->bOrientRotationToMovement = true;
            bUseControllerRotationYaw = false;
        }
    }
}

void ABoxPlayer::HandleCoverAction()
{
    if (!CoverController)
    {
        return;
    }

    if (IsInCover())
    {
        CoverController->ExitCover();
        return;
    }

    if (CoverController->TryEnterCover())
    {
        CoverController->SetAiming(bIsAiming);
    }
}

bool ABoxPlayer::IsInCover() const
{
    if (!CoverController)
    {
        return false;
    }

    switch (CoverController->State)
    {
    case ESimpleCoverState::Free:
    case ESimpleCoverState::Exit:
        return false;
    default:
        return true;
    }
}

void ABoxPlayer::OnCoverStateChanged(ESimpleCoverState NewState)
{
    ApplyRotationSettings();
}

void ABoxPlayer::OnCoverCameraOffsetUpdated(FVector Offset)
{
    CoverCameraOffset = Offset;
}
