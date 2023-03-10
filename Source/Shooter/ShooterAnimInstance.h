// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponType.h"
#include "ShooterAnimInstance.generated.h"


UENUM(BlueprintType)
enum class EOffsetState :uint8
{
	EOS_Aiming UMETA(DisplayName="Aiming"),
	EOS_Hip UMETA(DisplayName="Hip"),
	EOS_Reloading UMETA(DisplayName="Reloading"),
	EOS_InAir UMETA(DisplayName="InAir"),
	EOS_MAX UMETA(DisplayName="DefaultMAX")
}; 


/**
*
*/
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public: 
	UShooterAnimInstance();

	//Tick Function runs every frame
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);
	
	// Initialize function it initializes the class
	virtual void NativeInitializeAnimation() override;

protected:
	/** Handle turning in place variables */
	void TurnInPlace();

	/** Handle calculation for leaning when running */
	void Lean(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	class AShooterCharacter* ShooterCharacter;
	
	// Stores the speed of the player using velocity
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	float Speed;
	
	// Weather the character is in the air or not
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	bool bIsInAir;
	
	// checks if the character is moving
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess="true"))
	bool bIsAccelerating;
	
	//Offset Yaw used for strafing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess="true"))
	float MovementOffsetYaw;
	
	//Offset Yaw for the frame before we stop moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess="true"))
	float LastMovementOffsetYaw;

	// weather the character is aiming or not
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess="true"))
	bool bAiming;

	/** Yaw of character this frame used for Turn In Place Animations only updated when standing still and not in air */
	float TIPCharacterYaw;

	/** Yaw of character last frame used for Turn In Place Animations only updated when standing still  and not in air */
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn In Place", meta=(AllowPrivateAccess="true"))
	float RootYawOffset;

	/** Rotation Curve value this frame */
	float RotationCurve;

	/** Rotation Curve value last frame */
	float RotationCurveLastFrame;

	/** The pitch of the aim rotation used for aim offset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn In Place", meta=(AllowPrivateAccess="true"))
	float Pitch;

	/** True when reloading used to prevent aim offset while reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn In Place", meta=(AllowPrivateAccess="true"))
	bool bReloading;

	/** Offset state to determining which Aim offset to use */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn In Place", meta=(AllowPrivateAccess="true"))
	EOffsetState OffsetState;

	/** Rotation of character this frame */
	FRotator CharacterRotation;

	/** Rotation of character Last frame */
	FRotator CharacterRotationLastFrame;

	/** Yaw Delta used for leaning in Running BlendSpace */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Lean", meta=(AllowPrivateAccess="true"))
	float YawDelta;

	/** True when crouching */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Crouch, meta=(AllowPrivateAccess="true"))
	bool bCrouching;

	/** True when Equipping */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouch, meta = (AllowPrivateAccess = "true"))
	bool bEquipping;

	/** Change the recoil weight based on turning in place and aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess="true"))
	float RecoilWeight;

	/** True when turning in place */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess="true"))
	bool bTurningInPlace;

	/** Weapon Type for the currently equipped weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	EWeaponType EquippedWeaponType;

	/** True when not reloading or equipping */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldUseFABRIK;
};
