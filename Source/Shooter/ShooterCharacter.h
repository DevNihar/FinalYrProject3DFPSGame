// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

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
	
	/**Function to look right and left at a certain Rate
	 * @param Rate  Normalized rate value i.e if rate is 1.0 that means the speed is maximum
	*/
	void TurnAtRate(float Rate);
	
	/**Function to look up and down at a certain Rate
	 * @param Rate  Normalized rate value i.e if rate is 1.0 that means the speed is maximum
	*/
	void LookUpAtRate(float Rate);
	
	//Called when the fire button is pressed
	void FireWeapon();

	// Fucction to get the end location to spawn the beam and impact particles
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation);

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
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	float BaseLookUpRate;
	
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


public:
	/**Get CameraBoom Subobject*/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; };

	/**Get FollowCamera Subobject*/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; };
};
