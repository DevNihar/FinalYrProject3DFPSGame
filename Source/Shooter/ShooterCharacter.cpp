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

// Sets default values
AShooterCharacter::AShooterCharacter():
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Create a camera stick with length TargetArmLength
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);//Attaching the Camera Stick to the character root component
	CameraBoom->TargetArmLength = 300.f;//Setting the Camera Stick length
	CameraBoom->bUsePawnControlRotation = true; // toggles wether we want the camera stick to rotate with the controller or not
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);
	
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
	}
void AShooterCharacter::MoveForward(float Value)
{
	if((Controller != nullptr) && (Value != 0)){
		//Find out the direction the controller is facingw
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

void AShooterCharacter::FireWeapon(){
	UE_LOG(LogTemp, Warning, TEXT("Firing..."));
	if(FireSound){
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket){
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
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

		/*
		FHitResult FireHit;
		const FVector Start {SocketTransform.GetLocation()};
		const FQuat Rotation {SocketTransform.GetRotation()};
		const FVector RotationAxis {Rotation.GetAxisX()};
		const FVector End {Start + RotationAxis * 50'000.f };

		FVector BeamEndPoint {End};


		GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
		if(FireHit.bBlockingHit){
			// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
			// DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f);

			BeamEndPoint = FireHit.Location;

			if(ImpactParticles){
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
			} 
			
		}

		if(BeamParticles){
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if(Beam){
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
		}
		*/
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage){
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation){
	
	// Get current viewport size
	FVector2D ViewPortSize;
	if(GEngine && GEngine->GameViewport){
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	// Get Screen space location of crosshair
	FVector2D CrosshairLocation(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get World position and World direction of the crosshair
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if(bScreenToWorld)// Was Deprojection successfull?
	{
		FHitResult ScreenTraceHit;
		const FVector Start{CrosshairWorldPosition};
		const FVector End{CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f};
		
		// Set beam endpoint to line trace end point
		OutBeamEndLocation = End;

		// Trace outwards from crosshait location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		// was there a trace hit
		if(ScreenTraceHit.bBlockingHit){
			// BeamEndpoint is now Trace hit Location
			OutBeamEndLocation = ScreenTraceHit.Location;
		}
		
		// Perform a second trace this time from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{MuzzleSocketLocation};
		const FVector WeaponTraceEnd{OutBeamEndLocation};
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
		
		if(WeaponTraceHit.bBlockingHit)// Object between barrel and beam end point?
		{
			OutBeamEndLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::FireWeapon);

}

