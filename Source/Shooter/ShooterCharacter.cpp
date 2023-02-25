// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter():
	// Base rates for turning/ looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	// Aiming/ looking up rates when aiming/not aiming	
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// Mouse look sensitivity scale factors
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(0.6f),
	MouseAimingLookUpRate(0.6f),
	// True when aimint the weapon
	bAiming(false),
	// Camera Field of View values
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
	// Crosshair Spread Factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	// bullet fire Timer Variables
	ShootTimeDuration(0.05f),
	bFiringbullet(false),
	// Automatic Fire Variables	
	bFireButtonPressed(false),
	bShouldFire(true),
	AutomaticFireRate(0.1f),
	// Item Trace Variables
	bShouldTraceForItems(false),
	OverlappedItemCount(0),
	//CameraInterpLocation Variables
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	// Starting Ammo Amounts
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	// Combat State variables
	CombatState(ECombatState::ECS_Unoccupied)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Create a camera stick with length TargetArmLength
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);//Attaching the Camera Stick to the character root component
	CameraBoom->TargetArmLength = 180.f;//Setting the Camera Stick length
	CameraBoom->bUsePawnControlRotation = true; // toggles wether we want the camera stick to rotate with the controller or not
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);
	
	//Creating a Camera and attaching it to the socket at the end of the camera stick
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);// Specifing the component and the socket on that component to attach the camera to
	FollowCamera->bUsePawnControlRotation = false;//  toggles wether we want the camera to rotate relative to the camera stick or not
	
	//Dont rotate the player when the controller rotates just rotate the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
	//Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;// Character moves in the direction of input ....
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);// at this Rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create Hand Scene Component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));
	}
void AShooterCharacter::MoveForward(float Value)
{
	if((Controller != nullptr) && (Value != 0)){
		//Find out the direction the controller is facing
		FRotator Rotation{Controller->GetControlRotation()};
		FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);// moving in the configured direction
	}
}
void AShooterCharacter::MoveRight(float Value)
{
	if((Controller != nullptr) && (Value != 0)){
		//Find out the direction the controller is facing
		FRotator Rotation{Controller->GetControlRotation()};
		FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);// moving in the configured direction
	}
}
void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScalingFactor;
	if(bAiming)
	{
		TurnScalingFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScalingFactor = MouseHipTurnRate;
	}	
	AddControllerYawInput(Value * TurnScalingFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookScalingFactor;
	if(bAiming)
	{
		LookScalingFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookScalingFactor = MouseHipLookUpRate;
	}	
	AddControllerPitchInput(Value * LookScalingFactor);
}

void AShooterCharacter::FireWeapon(){
	// UE_LOG(LogTemp, Warning, TEXT("Firing..."));
	if(EquippedWeapon == nullptr) return;
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if(WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunfireMontage();

		// Start Bullet Fire timer for crosshair
		StartCrosshairBulletFire();

		// Substract one from the weapons ammo
		EquippedWeapon->DecrementAmmo();

		// Start Fire timer For Firing the weapon
		StartFireTimer();
	}

}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation)
{
	// Check for Crosshair Trace Hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshair(CrosshairHitResult, OutBeamEndLocation);
	if(bCrosshairHit)
	{
		/** Tentative beam end location - still need to trace from the
		 gun barrel to check whether there is something between the barrel and crosshair */
		OutBeamEndLocation = CrosshairHitResult.Location;
	}
	else // No Crosshair Trace Hit
	{
		// Out Beam Location is the End Location For the Line Trace
	}
	// Perform a second trace this time from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd { OutBeamEndLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	
	if(WeaponTraceHit.bBlockingHit)// Object between barrel and beam end point?
	{
		OutBeamEndLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}
void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	// Setting Current Field of View
	if(bAiming)
	{
		// Interpolating to the zoomed Field of view
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		// Interpolating to the default Field of view
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	 
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if(bAiming)
	{
		BaseLookUpRate = AimingLookUpRate;
		BaseTurnRate = AimingTurnRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}	
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f, 600.f};
	FVector2D VelocityMultiplierRange{0.f, 1.f};
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	if(GetMovementComponent()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if(bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	// True 0.05 seconds after Firing
	if(bFiringbullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
	
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringbullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringbullet = false;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
	// if(WeaponHasAmmo())
	// {		
	// 	StartFireTimer();
	// }
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if(WeaponHasAmmo())
	{
		if(bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		// Reload Weapon
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshair(FHitResult& OutTraceHitResult, FVector& OutHitLocation)
{
	// Getting View Port Size
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f , ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get World Position And Direction of Crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0), 
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if(bScreenToWorld)
	{
		// Trace From Crosshair World Position outwards
		const FVector Start{ CrosshairWorldPosition };
		const FVector End {Start + CrosshairWorldDirection * 50'000.f};

		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(OutTraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if(OutTraceHitResult.bBlockingHit)
		{
			OutHitLocation = OutTraceHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if(OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

void AShooterCharacter::TraceForItems()
{
	// UE_LOG(LogTemp, Warning, TEXT("%d"), bShouldTraceForItems);
	if(bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshair(ItemTraceResult, HitLocation);
		
		if(ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if(TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				bool visibility = TraceHitItem->GetPickupWidget()->IsVisible();
			}

			// We hit an AItem last Frame
			if(TraceHitItemLastFrame)
			{
				if(TraceHitItem != TraceHitItemLastFrame) // We are hitting a different Item or no Item at all
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			// Store a reference to the HitItem Last Frame
			TraceHitItemLastFrame = TraceHitItem;
		}
		
	}
	else if(TraceHitItemLastFrame) // we are no longer overlapping with the Area Sphere
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check the validity of TSubclassOf variable
	if(DefaultWeaponClass)
	{
		// Spawn the weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
		
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(WeaponToEquip)
	{
		// Get The hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		// Check the validity of HandSocket variable
		if(HandSocket)
		{
			// Attaching the weapon to hand Socket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		// Set the equipped weapon to newly spawned weapon
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if(EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void  AShooterCharacter::SelectButtonPressed()
{
	if(TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);
		if(TraceHitItem->GetPickupSound())
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickupSound());
		}
	}
}
	
void  AShooterCharacter::SelectButtonReleased()
{

}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
    if(EquippedWeapon == nullptr) return false;
	
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if(FireSound){
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket){
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if(MuzzleFlash){
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if(bBeamEnd){
			// Spawn impact particles after updating the beam end point 
			if(ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			// Spawn Beam from barrel to beam end point
			if(BeamParticles){
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if(Beam){
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}

void AShooterCharacter::PlayGunfireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage){
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if(EquippedWeapon == nullptr) return;

	// Do we have amo of the correct type
	if(CarryingAmmo())
	{
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	} 
}

void AShooterCharacter::FinishReloading()
{
	// Update the Combat state
	CombatState = ECombatState::ECS_Unoccupied;

	if(EquippedWeapon == nullptr) return;

	const auto AmmoType = EquippedWeapon->GetAmmoType();

	// Update the AmmoMap
	if(AmmoMap.Contains(AmmoType))
	{
		// Amount of ammo the character is carrying of the equipped weapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];

		// Space left in the magazine of equipped weapon
		int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if(MagEmptySpace > CarriedAmmo)
		{
			// Reloads the magazine wilth all the carried ammo
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		} 
		else
		{
			// fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}

}

bool AShooterCharacter::CarryingAmmo()
{
    if(EquippedWeapon == nullptr) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if(AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}
	return false;
}

void AShooterCharacter::GrabClip()
{
	if(EquippedWeapon == nullptr) return;
	if(HandSceneComponent == nullptr) return;
	// Index for the clip bone on the equipped weapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	// Store the transform of the clip
	ClipTransform =  EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation()};
	const FVector CameraForwardVector{FollowCamera->GetForwardVector()};

	// Distance = CameraWorldLocation + CameraForwardVector * A + CameraUpwardVector * B
	return CameraWorldLocation + CameraForwardVector * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	if(Item->GetEquipSound())
	{
		UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
	}
	auto Weapon = Cast<AWeapon>(Item);
	if(Weapon)
	{
		SwapWeapon(Weapon);
	}
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if(FollowCamera){
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// Spawn the Default Weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());
	InitializeAmmoMap();
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Handle Interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);
	// change look sensitivity based on aiming
	SetLookRates();
	// Calculate Crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);
	// Check OverlappedItemCount then Trace For Items
	TraceForItems();
	
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnAtRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpAtRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
}

