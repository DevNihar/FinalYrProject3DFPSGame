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
#include "Components/CapsuleComponent.h"
#include "Ammo.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Shooter.h"
#include "BulletHitInterface.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

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
	// True when aiming the weapon
	bAiming(false),
	bAimingButtonpressed(false),
	// Camera Field of View values
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(25.f),
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
	CombatState(ECombatState::ECS_Unoccupied),
	// Crouching Variables
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(300.f),
	StandingCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(44.f),
	BaseGroundFriction(2.f),
	CrouchingGroundFriction(100.f),
	//Pickup sound timer properties
	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f),
	// Icon Animation property
	HighlightedSlot(-1),
	// Health properties
	Health(100.f),
	MaxHealth(100.f),
	//Stun Chance
	StunChance(.25f)
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

	// Create Interpolation Components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpComp1->SetupAttachment(GetFollowCamera());

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpComp2->SetupAttachment(GetFollowCamera());

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpComp3->SetupAttachment(GetFollowCamera());

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpComp4->SetupAttachment(GetFollowCamera());

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpComp5->SetupAttachment(GetFollowCamera());

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
	}
	float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
	{
		if (Health - DamageAmount <= 0.f)
		{
			Health = 0.f;
			Die();

			auto EnemyController = Cast<AEnemyController>(EventInstigator);
			if (EnemyController)
			{
				EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CharacterDead"), true);
			}
		}
		else
		{
			Health -= DamageAmount;
		}
		return DamageAmount;
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

		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			EquippedWeapon->StartSlideTimer();
		}
	}

}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamEndLocation;
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
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd { OutBeamEndLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	
	if(!OutHitResult.bBlockingHit)// Object between barrel and beam end point?
	{
		OutHitResult.Location = OutBeamEndLocation;
		return false;
	}
	return true;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonpressed = true;
	if(CombatState != ECombatState::ECS_Reloading 
		&& CombatState != ECombatState::ECS_Equipping 
		&& CombatState != ECombatState::ECS_Stunned)
	{
		Aim();
	}
}
void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonpressed = false;
	StopAiming();
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
	if (bAiming)
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
	if (EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
}

void AShooterCharacter::AutoFireReset()
{
	if (CombatState == ECombatState::ECS_Stunned) return;
	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon == nullptr) return;

	if(WeaponHasAmmo())
	{
		if(bFireButtonPressed && EquippedWeapon->GetAutomatic())
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
			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
			
			if (TraceHitWeapon)
			{
				if (HighlightedSlot == -1)
				{
					// Not currently highlighting a slot ; highlight one
					HighlightInventorySlot();
				}
			}
			else
			{
				// is a slot being highlighetd
				if (HighlightedSlot != -1)
				{
					//Unhighlight the slot
					UnHighlightInventorySlot();
				}
			}
			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}

			if(TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				if (Inventory.Num() >= INVENTORY_CAPACITY)
				{
					// Inventory is Full 
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					// Inventory is not Full
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}

			// We hit an AItem last Frame
			if(TraceHitItemLastFrame)
			{
				if(TraceHitItem != TraceHitItemLastFrame) // We are hitting a different Item or no Item at all
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			// Store a reference to the HitItem Last Frame
			TraceHitItemLastFrame = TraceHitItem;
		}
		
	}
	else if(TraceHitItemLastFrame) // we are no longer overlapping with the Area Sphere
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
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

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
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

		if(EquippedWeapon == nullptr)
		{
			//send -1 if no equipped weapon yet. No need to reverse the icon animation
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());	
		}
		else if(!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
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
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if(TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
}
	
void  AShooterCharacter::SelectButtonReleased()
{

}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if(Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex()) 
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
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
	if(EquippedWeapon->GetFireSound()){
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if(EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
		}

		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);

		if(bBeamEnd)
		{
			// Does hit actor implement BulletHitInterface ?
			if(BeamHitResult.GetActor()->IsValidLowLevel())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.GetActor());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
				}
				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.GetActor());
				if (HitEnemy)
				{
					int32 Damage{};
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						// HeadShot
						Damage = EquippedWeapon->GetHeadshotDamage();
						UGameplayStatics::ApplyDamage(
							BeamHitResult.GetActor(),
							Damage,
							GetController(),
							this,
							UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, true);
					}
					else
					{
						// BodyShot
						Damage = EquippedWeapon->GetDamage();
						UGameplayStatics::ApplyDamage(
							BeamHitResult.GetActor(),
							Damage,
							GetController(),
							this,
							UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, false);
					}
				}
			}
			else
			{
				// Spawn Default Particles
				// Spawn impact particles after updating the beam end point 
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
				}
			}
			

			// Spawn Beam from barrel to beam end point
			if(BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
				}
			}
		}
	}
}

void AShooterCharacter::PlayGunfireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
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
	if(CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{
		if(bAiming)
		{
			StopAiming();
		}
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
	if (CombatState == ECombatState::ECS_Stunned) return;
	// Update the Combat state
	CombatState = ECombatState::ECS_Unoccupied;

	if(bAimingButtonpressed)
	{
		Aim();
	}

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

void AShooterCharacter::FinishEquipping()
{
	if (CombatState == ECombatState::ECS_Stunned) return;
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonpressed)
	{
		Aim();
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

void AShooterCharacter::CrouchButtonPressed()
{
	if(!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	if(bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AShooterCharacter::Jump()
{
	if(bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{};
	if(bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}

	const float InterpHalfHeight{FMath::FInterpTo(
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		 TargetCapsuleHalfHeight,
		  DeltaTime,
		   20.f)};

	
	// Negative when Standing -> Crouching and Positive when Crouching -> Standing 
	const float DeltaCapsuleHalfHeight{ InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()} ; 
	const FVector MeshOffset{0.f, 0.f, -DeltaCapsuleHalfHeight};
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if(!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}	
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	// Check to see if ammo map contains ammo's ammotype 
	if(AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount{AmmoMap[Ammo->GetAmmoType()]};
		AmmoCount += Ammo->GetItemCount();
		// Set the amount of ammo in the map for its type
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}
	if(EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		// Check to see if the gun is empty
		if(EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}
	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{WeaponInterpComp, 0};
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{InterpComp1, 0};
	InterpLocations.Add(InterpLoc1); 

	FInterpLocation InterpLoc2{InterpComp2, 0};
	InterpLocations.Add(InterpLoc2);

	FInterpLocation InterpLoc3{InterpComp3, 0};
	InterpLocations.Add(InterpLoc3);

	FInterpLocation InterpLoc4{InterpComp4, 0};
	InterpLocations.Add(InterpLoc4);

	FInterpLocation InterpLoc5{InterpComp5, 0};
	InterpLocations.Add(InterpLoc5);

	FInterpLocation InterpLoc6{InterpComp6, 0};
	InterpLocations.Add(InterpLoc6);
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	bool bCanExchangeItems = 
		(CurrentItemIndex != NewItemIndex) &&
		(NewItemIndex < Inventory.Num()) &&
		(CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping);
	if (bCanExchangeItems)
	{
		if (bAiming)
		{
			StopAiming();
		}
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}

		NewWeapon->PlayEquipSound(true);
	}
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr) 
		{
			return i;
		}
	}
	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}
	return -1; // Inventory is full 
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot{ GetEmptyInventorySlot() };
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start{ GetActorLocation() };
	const FVector End{ Start + FVector(0.f, 0.f, -400.f) };
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonpressed)
	{
		Aim();
	}
}

void AShooterCharacter::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		DisableInput(PC);
	}
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

void AShooterCharacter::Stun()
{
	if (Health <= 0.f) return;
	CombatState = ECombatState::ECS_Stunned;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
    int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for(int32 i=1; i<InterpLocations.Num(); i++)
	{
		if(InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount =  InterpLocations[i].ItemCount;
		}
	}
	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if(Amount < -1 || Amount > 1) return;
	if(InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}

// No Longet needed; AItem has GetInterpLocation()
// FVector AShooterCharacter::GetCameraInterpLocation()
// {
// 	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation()};
// 	const FVector CameraForwardVector{FollowCamera->GetForwardVector()};

// 	// Distance = CameraWorldLocation + CameraForwardVector * A + CameraUpwardVector * B
// 	return CameraWorldLocation + CameraForwardVector * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
// }

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();
	auto Weapon = Cast<AWeapon>(Item);
	if(Weapon)
	{
		if(Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else // Inventory is full swap with equipped weapon
		{
			SwapWeapon(Weapon);
		}
	}
	auto Ammo = Cast<AAmmo>(Item);
	if(Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if(Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
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
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);


	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	// Create InterpLocation Structs for each Interp Location and add to array
	InitializeInterpLocations();
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
	// Interpolate the capsule half height based on crouching/standing
	InterpCapsuleHalfHeight(DeltaTime);
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

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
	
	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);

}

