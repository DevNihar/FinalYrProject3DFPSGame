// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(DisplayName = "Assualt Rifle"),
	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**Function to move forwards and backwards by a certain value*/
	void MoveForward(float Value);
	
	/**Function to move right and left by a certain value*/
	void MoveRight(float Value);
	
	/**Function to look right and left at a certain Rate for controller input
	 * @param Rate  Normalized rate value i.e if rate is 1.0 that means the speed is maximum
	*/
	void TurnAtRate(float Rate);
	
	/**Function to look up and down at a certain Rate for controller input
	 * @param Rate  Normalized rate value i.e if rate is 1.0 that means the speed is maximum
	*/
	void LookUpAtRate(float Rate);

	/**Function to look right and left at a certain Rate for mouseX input
	 * @param Value input value from mouse movement
	*/
	void Turn(float Value);

	/**Function to look up and down at a certain Rate for mouseY input
	 * @param Value input value from mouse movement
	*/
	void LookUp(float Value);
	
	//Called when the fire button is pressed
	void FireWeapon();

	// Fucction to get the end location to spawn the beam and impact particles
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation);

	void AimingButtonPressed();
	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);

	/** Set BaseTurnRate and BaseLookUpRate when aiming */
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();

	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	bool TraceUnderCrosshair(FHitResult& OutTraceHitResult,  FVector& OutHitLocation);
	
	/** Trace For items if overlappedItemCount is greater than 0*/
	void TraceForItems();

	/** Spawns a AWeapon and equips it to the mesh */
	class AWeapon* SpawnDefaultWeapon();

	/** Equips a weapon to the weapon socket */
	void EquipWeapon(AWeapon* WeaponToEquip);

	/** Detaches the equipped weapon and lets it fall to the ground*/
	void DropWeapon();

	void SelectButtonPressed();
	
	void SelectButtonReleased();

	/** Drops the currently equipped weapon with the TraceHitWeapon */
	void SwapWeapon(AWeapon* WeaponToSwap);

	/** Initialize the ammo map with ammo values*/
	void InitializeAmmoMap();

	/** Check to amke sure our weapon has ammo */
	bool WeaponHasAmmo();

	void PlayFireSound();

	void SendBullet();

	void PlayGunfireMontage();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/**A camera stick which follows the character at a set distance(pulls in when character collides with an object)*/
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	class USpringArmComponent* CameraBoom;
	
	/**Camera attached to the Camera stick which follows the player*/
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	class UCameraComponent* FollowCamera;
	
	/**base speed at which the character turns */
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	float BaseTurnRate;
	
	/**base speed at which the character looks up and down*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	float BaseLookUpRate;

	/** Turn Rate when not aiming*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	float HipTurnRate;

	/** Look Up Rate when not aiming*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	float HipLookUpRate;

	/** Turn Rate when aiming*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	float AimingTurnRate;

	/** Look Up Rate when aiming*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	float AimingLookUpRate;

	/** Scale factor for mouse sensitivity. Turn Rate when not aiming*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"), meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float MouseHipTurnRate;

	/** Scale factor for mouse sensitivity. Look Up Rate when not aiming*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"), meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float MouseHipLookUpRate;

	/** Scale factor for mouse sensitivity. Turn Rate when aiming*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"), meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float MouseAimingTurnRate;

	/** Scale factor for mouse sensitivity. Look Up Rate when aiming*/
	UPROPERTY(EditDefaultsOnly,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"), meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float MouseAimingLookUpRate;
	
	/** Randomized gun sound cue*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess="true"))
	class USoundCue* FireSound;
	
	/** Flash spawned at barrel socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess="true"))
	class UParticleSystem* MuzzleFlash;
	
	/** Flash Spawned by bullet impact*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess="true"))
	UParticleSystem* ImpactParticles;

	/** Smoke Trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess="true"))
	UParticleSystem* BeamParticles;
	
	/*Montage for firing the weapon*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess="true"))
	class UAnimMontage* HipFireMontage;

	/** True when aiming*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess="true"))
	bool bAiming;

	/** Default Camera field of view value*/
	float CameraDefaultFOV;

	/** Field of view when zoomed in*/
	float CameraZoomedFOV;

	/** camera field of view in the current frame*/
	float CameraCurrentFOV;

	/** Zooming Interpolation speed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess="true"))
	float ZoomInterpSpeed;

	/** Controls the crosshair spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess="true"))
	float CrosshairSpreadMultiplier;

	/** Velocity component for the crosshair spread*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess="true"))
	float CrosshairVelocityFactor;
	
	/** Air component for the crosshair spread*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess="true"))
	float CrosshairInAirFactor;
	
	/** Aim component for the crosshair spread*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess="true"))
	float CrosshairAimFactor;
	
	/** Shooting component for the crosshair spread*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess="true"))
	float CrosshairShootingFactor;

	/** */
	float ShootTimeDuration;

	/** Are we Firing a bullet or not */
	bool bFiringbullet;

	/** Timer to call the callable function { FinishCrosshairBulletFire }*/
	FTimerHandle CrosshairShootTimer; 

	/* Left mouse button or console right trigger pressed*/
	bool bFireButtonPressed;

	/** True when we can fire false when waiting for timer */
	bool bShouldFire;

	/** Rate of automatic gunfire*/
	float AutomaticFireRate;

	FTimerHandle AutoFireTimer;

	/** true if we should trace Every Frame for items */
	bool bShouldTraceForItems;

	/* Number Of overlapped AItems*/
	int8 OverlappedItemCount;

	/** The AItem we hit last frame*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Items, meta=(AllowPrivateAccess="true"))
	class AItem* TraceHitItemLastFrame;

	/** Currently Equipped Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat, meta=(AllowPrivateAccess="true"))
	AWeapon* EquippedWeapon;

	/** Set This in blueprints for default Weapon Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combat, meta=(AllowPrivateAccess="true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	/** The item currently hit by our trace in traceforitems function (Can be null) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat, meta=(AllowPrivateAccess="true"))
	AItem* TraceHitItem;

	/** Distance outward from the camera for the interp destination*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category=Items, meta=(AllowPrivateAccess="true"))
	float CameraInterpDistance;

	/** Distance upward from the camera for the interp destination*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category=Items, meta=(AllowPrivateAccess="true"))
	float CameraInterpElevation;

	/** Map to keep track of different ammo types */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Items, meta=(AllowPrivateAccess="true"))
	TMap<EAmmoType, int32> AmmoMap;

	/** Starting amount of 9mm ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Items, meta=(AllowPrivateAccess="true"))
	int32 Starting9mmAmmo;

	/** Starting amount of AR ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Items, meta=(AllowPrivateAccess="true"))
	int32 StartingARAmmo;

	/** Combat statue, Can only fire or reload when unoccupied */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess="true"))
	ECombatState CombatState;

public:
	/**Get CameraBoom Subobject*/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/**Get FollowCamera Subobject*/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Get Aiming variable*/
	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const ;

	/** Get OverlappedItemCount */
	FORCEINLINE int8 GetOverlappedItemsCount() const { return OverlappedItemCount; }

	/** Increments / Decrements the OverlappedItemCount by a certain Amount and sets bShouldTraceForItems */
	void IncrementOverlappedItemCount(int8 Amount);


	FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);
};
