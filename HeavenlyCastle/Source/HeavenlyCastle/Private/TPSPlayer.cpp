#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "HealthComponent.h"
#include "Components/MeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/UserWidget.h"
#include "CombatStateWidget.h"
#include "UI/UISessionSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "ThrowableGrenade.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize projectile prediction speed
	ProjectilePredictionSpeed = 3000.f;

	// Initialize automatic fire rate
	TimeBetweenShots = 0.1f;

	// Initialize aiming flag
	bIsAiming = false;

	// Initialize sprinting flag
	bIsSprinting = false;

	// --- Camera Control Defaults ---
	DefaultCameraArmLength = 400.0f;
	AimingCameraArmLength = 80.0f;
	SprintCameraArmLength = 200.0f; // Example value, adjust as needed
	DefaultCameraSocketOffset = FVector(0.f, 50.f, 70.f);
	AimingCameraSocketOffset = FVector(0.f, 70.f, 60.f);
	SprintCameraSocketOffset = FVector(0.f, 60.f, 40.f); // Example value, adjust as needed
	CameraInterpSpeed = 20.0f;

	// Initialize cover flag
	bIsCovered = false;

	// Initialize exit cover animation variables
	bIsExitingCover = false;
	ExitCoverDuration = 0.1f;

	// Initialize enter cover animation variables
	bIsEnteringCover = false;
	EnterCoverDuration = 0.1f;

	// Initialize movement speeds
	DefaultWalkSpeed = 500.f;
	SprintWalkSpeed = 800.f;

	// Initialize weapon socket names
	WeaponSocketName = FName("Weapon");
	UnarmedBackSocketName = FName("UnarmedBack");

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Configure character movement for jumping/falling
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;

	// Create a camera boom (pulls in towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a projectile spawn point
	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPoint->SetupAttachment(GetMesh()); // Attach to mesh, can be adjusted to a specific socket later

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// Combat state defaults
	CombatState = ECombatState::Armed;
	EquipMontage = nullptr;
	UnequipMontage = nullptr;
	EquipAttachDelay = 0.2f;
	UnequipAttachDelay = 0.2f;
	bHasPendingStateAfterUnequip = false;

	// Debug
	bMovementDebugEnabled = true; // default on to investigate speed issue; toggle via console if noisy

	// Throwable defaults
	HeldGrenade = nullptr;
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
    Super::BeginPlay();

	// Set initial camera boom properties
	CameraBoom->TargetArmLength = DefaultCameraArmLength;
	CameraBoom->SocketOffset = DefaultCameraSocketOffset;
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Spawn and attach the weapon
	if (WeaponBlueprint && GetMesh() && GetMesh()->DoesSocketExist(WeaponSocketName))
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			SpawnedWeapon = World->SpawnActor<AActor>(WeaponBlueprint, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (SpawnedWeapon)
			{
				FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, /*bWeldSimulatedBodies*/ false);
				SpawnedWeapon->AttachToComponent(GetMesh(), AttachmentRules, WeaponSocketName);
				// Ensure attached weapon doesn't block or overlap the character/world
				ConfigureWeaponCollision();
			}
		}
	}
	else
	{
		if (!WeaponBlueprint)
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponBlueprint is not set in TPSPlayer."));
		}
		if (!GetMesh() || !GetMesh()->DoesSocketExist(WeaponSocketName))
		{
			UE_LOG(LogTemp, Warning, TEXT("Socket '%s' does not exist on the player mesh."), *WeaponSocketName.ToString());
		}
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ATPSPlayer::OnHealthChanged);
	}

	// Ensure UI via subsystem and push initial state/health
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UUISessionSubsystem* UIS = LP->GetSubsystem<UUISessionSubsystem>())
			{
				// Create HUD via subsystem using assigned class as fallback
				TSubclassOf<UUserWidget> HUDClassToUse = CombatStateWidgetClass ? CombatStateWidgetClass : TSubclassOf<UUserWidget>(UCombatStateWidget::StaticClass());
				UIS->EnsureHUDWithClass(PC, HUDClassToUse);
				UIS->PushCombatState(CombatState);
				if (HealthComponent)
				{
					UIS->PushHealth(HealthComponent->GetHealth(), HealthComponent->GetDefaultHealth());
				}
			}
		}
	}

	UpdateCombatStateUI();

	// Quick sanity hints for grenade setup
	if (!GrenadeClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TPSPlayer: GrenadeClass is not set"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("[Grenade] GrenadeClass not set on TPSPlayer"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TPSPlayer: GrenadeClass set to %s"), *GrenadeClass->GetName());
	}

	if (!ThrowReadyAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("TPSPlayer: ThrowReadyAction is not assigned (optional)"));
	}
	if (!ThrowAction)
	{
		UE_LOG(LogTemp, Log, TEXT("TPSPlayer: ThrowAction not assigned; FireAction will throw while ThrownReady."));
	}

	if (GetMesh() && ThrowAttachSocketName != NAME_None && !GetMesh()->DoesSocketExist(ThrowAttachSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("TPSPlayer: ThrowAttachSocket '%s' does not exist on mesh"), *ThrowAttachSocketName.ToString());
	}
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	// --- Camera Interpolation ---
	float TargetArmLength;
	FVector TargetSocketOffset;

	if (bIsSprinting)
	{
		TargetArmLength = SprintCameraArmLength;
		TargetSocketOffset = SprintCameraSocketOffset;
	}
	else if (bIsAiming)
	{
		TargetArmLength = AimingCameraArmLength;
		TargetSocketOffset = AimingCameraSocketOffset;
	}
	else
	{
		TargetArmLength = DefaultCameraArmLength;
		TargetSocketOffset = DefaultCameraSocketOffset;
	}

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraInterpSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, CameraInterpSpeed);

	if (bIsCovered)
	{
		// If currently entering cover, interpolate position
		if (bIsEnteringCover)
		{
			float ElapsedTime = GetWorld()->GetTimeSeconds() - EnterCoverStartTime;
			float Alpha = FMath::Clamp(ElapsedTime / EnterCoverDuration, 0.f, 1.f);
			FVector CurrentLocation = FMath::Lerp(EnterCoverStartLocation, EnterCoverTargetLocation, Alpha);
			FRotator CurrentRotation = FMath::Lerp(EnterCoverStartRotation, EnterCoverTargetRotation, Alpha);
			SetActorLocationAndRotation(CurrentLocation, CurrentRotation);

			if (Alpha >= 1.f)
			{
				bIsEnteringCover = false;
			}
			DrawDebugString(GetWorld(), FVector(0, 0, 100), "Entering Cover", this, FColor::Cyan, 0.f);
		}
		else
		{
			// Keep the player facing away from the cover wall
			SetActorRotation((-CoverWallNormal).Rotation());
			DrawDebugString(GetWorld(), FVector(0, 0, 100), "Covered", this, FColor::Green, 0.f);
		}

		// Draw an arrow indicating the cover normal
		FVector ArrowStart = GetActorLocation() + FVector(0, 0, 50.f); // Chest height
		FVector ArrowEnd = ArrowStart + CoverWallNormal * 50.f;
		DrawDebugDirectionalArrow(GetWorld(), ArrowStart, ArrowEnd, 2.5f, FColor::Blue, false, 0.f, 0, 1.0f);
	}
	else if (bIsExitingCover)
	{
		float ElapsedTime = GetWorld()->GetTimeSeconds() - ExitCoverStartTime;
		float Alpha = FMath::Clamp(ElapsedTime / ExitCoverDuration, 0.f, 1.f);
		FVector CurrentLocation = FMath::Lerp(ExitCoverStartLocation, ExitCoverTargetLocation, Alpha);
		SetActorLocation(CurrentLocation);

		if (Alpha >= 1.f)
		{
			bIsExitingCover = false;
		}
		DrawDebugString(GetWorld(), FVector(0, 0, 100), "Exiting Cover", this, FColor::Yellow, 0.f);
	}
	else
	{
		DrawDebugString(GetWorld(), FVector(0, 0, 100), "Not Covered", this, FColor::Red, 0.f);
	}

	// Draw projectile path when armed (gun)
	if (ProjectileClass && SpawnedWeapon && CombatState == ECombatState::Armed)
	{
		FVector MuzzleLocation;
		FRotator MuzzleRotation;
			if (UMeshComponent* MuzzleMesh = SpawnedWeapon->FindComponentByClass<UMeshComponent>())
			{
				MuzzleLocation = MuzzleMesh->GetSocketLocation(FName("Muzzle"));
				MuzzleRotation = MuzzleMesh->GetSocketRotation(FName("Muzzle"));
			}
		else
		{
			MuzzleLocation = ProjectileSpawnPoint->GetComponentLocation();
			MuzzleRotation = GetActorForwardVector().Rotation();
		}

		FPredictProjectilePathParams PredictParams(20.f, MuzzleLocation, MuzzleRotation.Vector() * ProjectilePredictionSpeed, 5.f, ECC_Visibility);
		FPredictProjectilePathResult PredictResult;

		if (UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult))
		{
			for (int32 i = 0; i < PredictResult.PathData.Num() - 1; ++i)
			{
				DrawDebugLine(GetWorld(), PredictResult.PathData[i].Location, PredictResult.PathData[i+1].Location, FColor::Yellow, false, 0.f, 0, 0.5f);
    			}
    		}
    	}

	// Draw throw prediction when in thrown-ready
	if (bDrawThrowPrediction && CombatState == ECombatState::ThrownReady)
	{
		FVector Start, Vel;
		ComputeThrowParams(Start, Vel);
		FPredictProjectilePathParams PredictParams(10.f, Start, Vel, 5.f, ECC_Visibility);
		PredictParams.bTraceWithCollision = true;
		FPredictProjectilePathResult PredictResult;
		if (UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult))
		{
			for (int32 i = 0; i < PredictResult.PathData.Num() - 1; ++i)
			{
				DrawDebugLine(GetWorld(), PredictResult.PathData[i].Location, PredictResult.PathData[i+1].Location, FColor::Cyan, false, 0.f, 0, 0.75f);
			}
			DrawDebugSphere(GetWorld(), PredictResult.HitResult.ImpactPoint, 8.f, 12, FColor::Red, false, 0.f);
		}
	}

    // Removed SPD on-screen movement debug overlay
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPSPlayer::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPSPlayer::Look);

		//Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATPSPlayer::AimStarted);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATPSPlayer::AimStopped);

		//Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATPSPlayer::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATPSPlayer::StopFire);

		//Cover
		EnhancedInputComponent->BindAction(CoverAction, ETriggerEvent::Started, this, &ATPSPlayer::Cover);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATPSPlayer::SprintStarted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATPSPlayer::SprintStopped);


		// Arm/Unarm Toggle
		EnhancedInputComponent->BindAction(ArmAction, ETriggerEvent::Started, this, &ATPSPlayer::ToggleArmed);

		// Thrown-Ready toggle (optional IA)
		if (ThrowReadyAction)
		{
			EnhancedInputComponent->BindAction(ThrowReadyAction, ETriggerEvent::Started, this, &ATPSPlayer::ToggleThrownReady);
		}

		// Dedicated Throw input (optional IA). If not set, FireAction while ThrownReady will throw.
		if (ThrowAction)
		{
			EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Started, this, &ATPSPlayer::StartThrow);
		}
	}
}

void ATPSPlayer::ToggleMovementDebug()
{
    bMovementDebugEnabled = !bMovementDebugEnabled;
    if (!bMovementDebugEnabled && GEngine)
    {
        // Clear the line next tick by posting empty with brief lifetime
        GEngine->AddOnScreenDebugMessage(1001, 0.01f, FColor::Green, TEXT(""), false);
    }
}

void ATPSPlayer::ConfigureWeaponCollision()
{
    if (!SpawnedWeapon)
    {
        return;
    }

    TArray<UPrimitiveComponent*> PrimComponents;
    SpawnedWeapon->GetComponents<UPrimitiveComponent>(PrimComponents, true);
    for (UPrimitiveComponent* Prim : PrimComponents)
    {
        if (!Prim) continue;
        // Ensure weapon does not simulate physics while attached
        // to avoid invalid simulate options with NoCollision.
        Prim->SetSimulatePhysics(false);
        Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Prim->SetGenerateOverlapEvents(false);
        Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }
}

void ATPSPlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (bIsCovered)
		{
			FVector MoveDirection = FVector::CrossProduct(CoverWallNormal, FVector::UpVector) * MovementVector.X;

			// Check if there is a wall in the direction of movement
			FHitResult HitResult;
			FVector Start = GetActorLocation() + MoveDirection * 50.f; // Check slightly in front of the player
			FVector End = Start - CoverWallNormal * 100.f; // Check towards the wall
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel1, QueryParams))
			{
				// Only move if there is a wall to stay in cover
				AddMovementInput(MoveDirection, 1.f);
			}
		}
		else
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
			// get right vector 
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void ATPSPlayer::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATPSPlayer::AimStarted()
{
	bIsAiming = true;
	UpdateRotationSettings();
}

void ATPSPlayer::AimStopped()
{
	bIsAiming = false;
	UpdateRotationSettings();
}

void ATPSPlayer::StartFire()
{
	// If in thrown-ready, pressing Fire throws the grenade
	if (CombatState == ECombatState::ThrownReady)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("StartFire in ThrownReady -> StartThrow"));
		}
		UE_LOG(LogTemp, Log, TEXT("StartFire(): ThrownReady -> StartThrow"));
		StartThrow();
		return;
	}

	if (CombatState != ECombatState::Armed)
	{
		return;
	}
	UpdateRotationSettings();
	Fire(); // Fire immediately on press
	GetWorldTimerManager().SetTimer(TimerHandle_AutomaticFire, this, &ATPSPlayer::Fire, TimeBetweenShots, true);
}

void ATPSPlayer::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_AutomaticFire);
	UpdateRotationSettings();
}

void ATPSPlayer::ToggleThrownReady()
{
    if (CombatState == ECombatState::ThrownReady)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("ToggleThrownReady -> Unarmed"));
        }
        UE_LOG(LogTemp, Log, TEXT("ToggleThrownReady(): Leaving ThrownReady -> Unarmed"));
        RequestSetCombatState(ECombatState::Unarmed);
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("ToggleThrownReady -> SetThrownReady"));
        }
        UE_LOG(LogTemp, Log, TEXT("ToggleThrownReady(): Enter ThrownReady"));
        SetThrownReady();
    }
}

void ATPSPlayer::StartThrow()
{
    if (CombatState != ECombatState::ThrownReady)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("StartThrow rejected: Not in ThrownReady"));
        }
        UE_LOG(LogTemp, Warning, TEXT("StartThrow(): Rejected. CombatState=%d"), (int32)CombatState);
        return;
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("StartThrow accepted"));
    }
    UE_LOG(LogTemp, Log, TEXT("StartThrow(): Proceed"));
    ThrowGrenade();
}

void ATPSPlayer::UpdateRotationSettings()
{
	bool bShouldOrientToMovement = true;
	bool bShouldUseControllerRotationYaw = false;

	// If we are aiming OR firing, we want to face the camera direction.
	if (bIsAiming || GetWorldTimerManager().IsTimerActive(TimerHandle_AutomaticFire))
	{
		bShouldOrientToMovement = false;
		bShouldUseControllerRotationYaw = true;
	}

	GetCharacterMovement()->bOrientRotationToMovement = bShouldOrientToMovement;
	bUseControllerRotationYaw = bShouldUseControllerRotationYaw;
}

void ATPSPlayer::Fire()
{
	// Implement projectile firing logic here
	if (CombatState == ECombatState::Armed && ProjectileClass && SpawnedWeapon)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			// Get the Muzzle socket transform from the spawned weapon
			FVector SpawnLocation;
			FRotator SpawnRotation;
			if (UMeshComponent* MuzzleMesh = SpawnedWeapon->FindComponentByClass<UMeshComponent>())
			{
				SpawnLocation = MuzzleMesh->GetSocketLocation(FName("Muzzle"));
				SpawnRotation = MuzzleMesh->GetSocketRotation(FName("Muzzle"));
			}
			else
			{
				// Fallback to projectile spawn point if Muzzle socket is not found
				SpawnLocation = ProjectileSpawnPoint->GetComponentLocation();
				SpawnRotation = GetActorForwardVector().Rotation();
			}


			// Spawn the projectile
			AActor* SpawnedProjectile = World->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (SpawnedProjectile)
			{
				UE_LOG(LogTemp, Warning, TEXT("Projectile Fired!"));

				if (FireEffect)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FireEffect, SpawnLocation, SpawnRotation);
				}

				// Update the prediction speed from the spawned projectile
				UProjectileMovementComponent* ProjectileMovement = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>();
				if (ProjectileMovement)
				{
					ProjectilePredictionSpeed = ProjectileMovement->InitialSpeed;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectileClass or SpawnedWeapon not set!"));
	}
}

void ATPSPlayer::Cover()
{
	if (bIsCovered)
	{
		ExitCover();
	}
	else
	{
		TryEnterCover();
	}
}

void ATPSPlayer::SprintStarted()
{
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintWalkSpeed;
}

void ATPSPlayer::SprintStopped()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void ATPSPlayer::TryEnterCover()
{
	UE_LOG(LogTemp, Warning, TEXT("TryEnterCover!"));

	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 100.f;
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

		// Use custom Cover trace channel (Configured in DefaultEngine.ini as GameTraceChannel1)
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel1, QueryParams))
		{
		// If already exiting cover, stop it
		if (bIsExitingCover)
		{
			bIsExitingCover = false;
		}

		bIsCovered = true;
		CoverWallNormal = HitResult.ImpactNormal;

		// --- Snap to cover ---
		// Calculate the new location to snap to the wall
		const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
		FVector TargetLocation = HitResult.ImpactPoint + CoverWallNormal * (CapsuleRadius + 2.0f); // Add a small buffer to avoid clipping
		TargetLocation.Z = GetActorLocation().Z; // Keep the current Z location to prevent snapping up/down

		// Set up enter cover animation
		bIsEnteringCover = true;
		EnterCoverStartTime = GetWorld()->GetTimeSeconds();
		EnterCoverStartLocation = GetActorLocation();
		EnterCoverTargetLocation = TargetLocation;
		EnterCoverStartRotation = GetActorRotation();
		EnterCoverTargetRotation = (-CoverWallNormal).Rotation();

		// Stop movement and set initial rotation
		GetCharacterMovement()->StopMovementImmediately();
		// SetActorLocationAndRotation(NewLocation, (-CoverWallNormal).Rotation()); // This will be handled by interpolation

		// Disable rotation to movement and enable controller rotation yaw
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;

		UE_LOG(LogTemp, Warning, TEXT("Entered Cover!"));
	}
}

void ATPSPlayer::ExitCover()
{
	bIsCovered = false;

	// Set up exit cover animation
	bIsExitingCover = true;
	ExitCoverStartTime = GetWorld()->GetTimeSeconds();
	ExitCoverStartLocation = GetActorLocation();
	ExitCoverTargetLocation = GetActorLocation() + CoverWallNormal * 10.f;

	// Restore normal movement and rotation settings
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	UE_LOG(LogTemp, Warning, TEXT("Exited Cover!"));
}

void ATPSPlayer::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Health <= 0.0f && !bIsDead)
    {
        OnDeath();
    }

    // Push health to UI subsystem
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UUISessionSubsystem* UIS = LP->GetSubsystem<UUISessionSubsystem>())
            {
                const float MaxHealth = HealthComponent ? HealthComponent->GetDefaultHealth() : Health;
                UIS->PushHealth(Health, MaxHealth);
            }
        }
    }
}

void ATPSPlayer::ComputeThrowParams(FVector& OutStart, FVector& OutVelocity) const
{
    // Start from character throw socket (hand) if available; else fallback to a point in front of the character
    FVector Start = GetActorLocation() + GetActorForwardVector() * 30.f + FVector(0, 0, 60.f);
    if (GetMesh())
    {
        if (ThrowAttachSocketName != NAME_None && GetMesh()->DoesSocketExist(ThrowAttachSocketName))
        {
            Start = GetMesh()->GetSocketLocation(ThrowAttachSocketName);
        }
    }

    const FRotator AimRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
    const FVector Forward = AimRot.Vector();

    OutStart = Start;
    OutVelocity = Forward * ThrowSpeed;
}

void ATPSPlayer::OnDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	// Disable input
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}

    // Drop weapon with physics so it separates cleanly from ragdoll
    DropWeaponWithPhysics();

    // Ragdoll: simulate, but do not physically affect other Pawns
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    // Prevent ragdoll from pushing characters (ignore Pawn channel)
    GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    GetMesh()->SetGenerateOverlapEvents(false);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Stop character movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	UE_LOG(LogTemp, Warning, TEXT("Player has died!"));
}

void ATPSPlayer::AttachWeaponToSocket(const FName& SocketName)
{
	if (!SpawnedWeapon || !GetMesh())
	{
		return;
	}

    FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, /*bWeldSimulatedBodies*/ false);
	SpawnedWeapon->AttachToComponent(GetMesh(), AttachmentRules, SocketName);
}

void ATPSPlayer::DropWeaponWithPhysics()
{
    if (!SpawnedWeapon)
    {
        return;
    }

    // Detach first to avoid inheriting skeletal motion
    SpawnedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // Enable physics and sane collision on all primitive components
    TArray<UPrimitiveComponent*> PrimComponents;
    SpawnedWeapon->GetComponents<UPrimitiveComponent>(PrimComponents, true);
    for (UPrimitiveComponent* Prim : PrimComponents)
    {
        if (!Prim) continue;

        Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Prim->SetGenerateOverlapEvents(false);
        Prim->SetCollisionResponseToAllChannels(ECR_Block);
        // Do not interact with Pawns to avoid pushing characters
        Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        // Often you don't want to block camera
        Prim->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

        Prim->SetSimulatePhysics(true);

        // Apply a small outward + upward impulse for a natural drop
        const FVector Impulse = GetActorForwardVector() * WeaponDropForwardImpulse + FVector::UpVector * WeaponDropUpImpulse;
        Prim->AddImpulse(Impulse, NAME_None, true);
    }
}

void ATPSPlayer::HandleEquipAttach()
{
    AttachWeaponToSocket(WeaponSocketName);
    CombatState = ECombatState::Armed;
    UpdateCombatStateUI();
    ConfigureWeaponCollision();
}

void ATPSPlayer::HandleUnequipAttach()
{
    AttachWeaponToSocket(UnarmedBackSocketName);
    if (bHasPendingStateAfterUnequip)
    {
        CombatState = PendingStateAfterUnequip;
        bHasPendingStateAfterUnequip = false;
    }
    else
    {
        CombatState = ECombatState::Unarmed;
    }
    UpdateCombatStateUI();
    ConfigureWeaponCollision();

    // If we transitioned into ThrownReady, spawn/attach held grenade now
    if (CombatState == ECombatState::ThrownReady)
    {
        OnEnterThrownReady();
    }
}

void ATPSPlayer::UpdateCombatStateUI()
{
	FString Label;
	switch (CombatState)
	{
	case ECombatState::Armed: Label = TEXT("무장 (Armed)"); break;
	case ECombatState::Unarmed: Label = TEXT("비무장 (Unarmed)"); break;
	case ECombatState::ThrownReady: Label = TEXT("투척 준비 (Thrown)"); break;
	case ECombatState::Equipping: Label = TEXT("무장 전환 중 (Equipping)"); break;
	case ECombatState::Unequipping: Label = TEXT("비무장 전환 중 (Unequipping)"); break;
	}

    // Push state to UI subsystem
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UUISessionSubsystem* UIS = LP->GetSubsystem<UUISessionSubsystem>())
            {
                UIS->PushCombatState(CombatState);
            }
        }
    }

    // Back-compat: update legacy widget if still used
    if (CombatStateWidgetInstance)
    {
        static const FName FuncName = FName("UpdateStateText");
        if (CombatStateWidgetInstance->GetClass()->FindFunctionByName(FuncName))
        {
            struct FParams { FText InText; } Params{ FText::FromString(Label) };
            CombatStateWidgetInstance->ProcessEvent(CombatStateWidgetInstance->FindFunction(FuncName), &Params);
        }
    }

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage((uint64)((PTRINT)this & 0xFFFF), 2.0f, FColor::Cyan, FString::Printf(TEXT("State: %s"), *Label));
	}
}

void ATPSPlayer::RequestSetCombatState(ECombatState NewState)
{
	if (CombatState == ECombatState::Equipping || CombatState == ECombatState::Unequipping)
	{
		return;
	}

	switch (NewState)
	{
case ECombatState::Armed:
        if (CombatState == ECombatState::ThrownReady)
        {
            OnExitThrownReady();
        }
        if (CombatState == ECombatState::Armed)
        {
            return;
        }
		// From Unarmed/ThrownReady -> Equip
		if (EquipMontage && GetMesh() && GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->Montage_Play(EquipMontage);
			CombatState = ECombatState::Equipping;
			GetWorldTimerManager().SetTimer(TimerHandle_EquipAttach, this, &ATPSPlayer::HandleEquipAttach, EquipAttachDelay, false);
			UpdateCombatStateUI();
		}
		else
		{
			HandleEquipAttach();
		}
		break;

	case ECombatState::Unarmed:
		if (CombatState == ECombatState::Unarmed)
		{
			return;
		}
		if (CombatState == ECombatState::Armed)
		{
			if (UnequipMontage && GetMesh() && GetMesh()->GetAnimInstance())
			{
				GetMesh()->GetAnimInstance()->Montage_Play(UnequipMontage);
				CombatState = ECombatState::Unequipping;
				GetWorldTimerManager().SetTimer(TimerHandle_UnequipAttach, this, &ATPSPlayer::HandleUnequipAttach, UnequipAttachDelay, false);
				UpdateCombatStateUI();
			}
			else
			{
				HandleUnequipAttach();
			}
		}
        else
        {
            // From ThrownReady -> Unarmed
            if (CombatState == ECombatState::ThrownReady)
            {
                OnExitThrownReady();
            }
            CombatState = ECombatState::Unarmed;
            UpdateCombatStateUI();
        }
        break;

	case ECombatState::ThrownReady:
		if (CombatState == ECombatState::Armed)
		{
			bHasPendingStateAfterUnequip = true;
			PendingStateAfterUnequip = ECombatState::ThrownReady;
			RequestSetCombatState(ECombatState::Unarmed);
		}
        else
        {
            CombatState = ECombatState::ThrownReady;
            UpdateCombatStateUI();
            OnEnterThrownReady();
        }
        break;

	default:
		break;
	}
}

void ATPSPlayer::ToggleArmed()
{
	if (CombatState == ECombatState::Armed)
	{
		RequestSetCombatState(ECombatState::Unarmed);
	}
	else if (CombatState == ECombatState::Unarmed || CombatState == ECombatState::ThrownReady)
	{
		RequestSetCombatState(ECombatState::Armed);
	}
}

void ATPSPlayer::SetThrownReady()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("SetThrownReady()"));
    }
    UE_LOG(LogTemp, Log, TEXT("SetThrownReady()"));
    RequestSetCombatState(ECombatState::ThrownReady);
}

void ATPSPlayer::OnEnterThrownReady()
{
    // Clean any previous held grenade
    if (HeldGrenade)
    {
        HeldGrenade->Destroy();
        HeldGrenade = nullptr;
    }

    if (!GrenadeClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnEnterThrownReady: GrenadeClass not set"));
        return;
    }

    if (!GetMesh())
    {
        return;
    }

    // Spawn grenade and attach to throw socket
    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = GetInstigator();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    FVector SpawnLoc = GetActorLocation();
    FRotator SpawnRot = GetActorRotation();
    HeldGrenade = GetWorld()->SpawnActor<AThrowableGrenade>(GrenadeClass, SpawnLoc, SpawnRot, Params);
    if (!HeldGrenade)
    {
        UE_LOG(LogTemp, Error, TEXT("OnEnterThrownReady: Failed to spawn held grenade"));
        return;
    }

    // Disable collision while held and deactivate movement
    TArray<UPrimitiveComponent*> PrimComponents;
    HeldGrenade->GetComponents<UPrimitiveComponent>(PrimComponents, true);
    for (UPrimitiveComponent* Prim : PrimComponents)
    {
        if (!Prim) continue;
        Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Prim->SetGenerateOverlapEvents(false);
    }
    if (UProjectileMovementComponent* Move = HeldGrenade->ProjectileMovement)
    {
        Move->StopMovementImmediately();
        Move->Deactivate();
    }

    const FName SocketToUse = (ThrowAttachSocketName != NAME_None && GetMesh()->DoesSocketExist(ThrowAttachSocketName))
                                ? ThrowAttachSocketName : WeaponSocketName;
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, false);
    HeldGrenade->AttachToComponent(GetMesh(), AttachRules, SocketToUse);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, FString::Printf(TEXT("Held grenade attached to %s"), *SocketToUse.ToString()));
    }
    UE_LOG(LogTemp, Log, TEXT("OnEnterThrownReady: Held grenade attached to %s"), *SocketToUse.ToString());
}

void ATPSPlayer::OnExitThrownReady()
{
    if (HeldGrenade)
    {
        HeldGrenade->Destroy();
        HeldGrenade = nullptr;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("Held grenade destroyed (exit ThrownReady)"));
        }
        UE_LOG(LogTemp, Log, TEXT("OnExitThrownReady: Held grenade cleaned"));
    }
}

void ATPSPlayer::ThrowGrenade()
{
    if (!GrenadeClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("GrenadeClass not set on TPSPlayer"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, TEXT("Throw failed: GrenadeClass not set"));
        }
        // Still exit thrown-ready so player isn't stuck
        RequestSetCombatState(ECombatState::Unarmed);
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // If we have a held grenade (attached), detach and throw it
    if (HeldGrenade)
    {
        FVector Start, Vel;
        ComputeThrowParams(Start, Vel);
        HeldGrenade->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        // Re-enable collision on all primitive components
        TArray<UPrimitiveComponent*> PrimComponents;
        HeldGrenade->GetComponents<UPrimitiveComponent>(PrimComponents, true);
        for (UPrimitiveComponent* Prim : PrimComponents)
        {
            if (!Prim) continue;
            Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Prim->SetGenerateOverlapEvents(true);
        }

        if (UProjectileMovementComponent* Move = HeldGrenade->ProjectileMovement)
        {
            Move->Activate(true);
            Move->Velocity = Vel;
        }
        HeldGrenade->FuseTime = GrenadeFuseTime;
        HeldGrenade->ActivateFuse();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Held grenade thrown"));
        }
        UE_LOG(LogTemp, Log, TEXT("ThrowGrenade(): Held grenade thrown"));

        HeldGrenade = nullptr;

        // Return to unarmed after throw; weapon stays on back
        RequestSetCombatState(ECombatState::Unarmed);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("State -> Unarmed after throw"));
        }
        UE_LOG(LogTemp, Log, TEXT("ThrowGrenade(): State -> Unarmed"));
        return;
    }

    FVector Start, Vel;
    ComputeThrowParams(Start, Vel);
    if (GEngine)
    {
        const FString Msg = FString::Printf(TEXT("Throw params: Start=%s Speed=%.1f"), *Start.ToCompactString(), Vel.Length());
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, Msg);
    }
    UE_LOG(LogTemp, Log, TEXT("ThrowGrenade(): Start=(%s) Vel=%.1f"), *Start.ToString(), Vel.Length());
    // Visualize start and direction
    DrawDebugSphere(GetWorld(), Start, 8.f, 12, FColor::White, false, 2.0f);
    DrawDebugLine(GetWorld(), Start, Start + Vel.GetSafeNormal() * 200.f, FColor::Green, false, 2.0f, 0, 1.5f);

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = GetInstigator();
    AThrowableGrenade* Grenade = World->SpawnActor<AThrowableGrenade>(GrenadeClass, Start, Vel.Rotation(), Params);
    if (Grenade)
    {
        if (UProjectileMovementComponent* Move = Grenade->ProjectileMovement)
        {
            Move->Velocity = Vel;
        }
        Grenade->FuseTime = GrenadeFuseTime;
        Grenade->ActivateFuse();
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Grenade spawned + fuse started"));
        }
        UE_LOG(LogTemp, Log, TEXT("ThrowGrenade(): Spawn success -> Fuse=%.2fs"), GrenadeFuseTime);
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Grenade spawn FAILED (check collision/class)"));
        }
        UE_LOG(LogTemp, Error, TEXT("ThrowGrenade(): Spawn FAILED at Start=%s"), *Start.ToString());
    }
    
    // Return to unarmed after throw; weapon stays on back
    RequestSetCombatState(ECombatState::Unarmed);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("State -> Unarmed after throw"));
    }
    UE_LOG(LogTemp, Log, TEXT("ThrowGrenade(): State -> Unarmed"));
}
