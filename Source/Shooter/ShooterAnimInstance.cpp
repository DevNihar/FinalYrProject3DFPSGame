// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if(ShooterCharacter == nullptr){
        ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());    
    }
    if(ShooterCharacter){
        // Getting Speed
        FVector Velocity = ShooterCharacter->GetVelocity();
        Velocity.Z = 0;
        Speed = Velocity.Size();
        //Finding out if the character is in air 
        bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
        //findinf out if the character is moving or not
        if(ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f){
            bIsAccelerating = true;
        }else{
            bIsAccelerating = false;
        }
        
        //Getting Aim rotation
        FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
        
        // Getting Movement rotation using GetRotFromX function
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

        // Getting Diffrence between the aiming yaw direction and movement yaw direction to trigger strafing animations
        MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

        if(ShooterCharacter->GetVelocity().Size() > 0.f){
            LastMovementOffsetYaw = MovementOffsetYaw;
        }

        bAiming = ShooterCharacter->GetAiming();
        
        // FString MovementOffsetYawMessage = FString::Printf(TEXT("MovementOffsetYaw: %f"), MovementOffsetYaw);
        // FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
        // FString MovementRotationMessage = FString::Printf(TEXT("Base Movement Rotation: %f"), MovementRotation.Yaw);

        // if(GEngine){
        //     GEngine->AddOnScreenDebugMessage(1,0.f,FColor::White, MovementOffsetYawMessage);
        // }

    }
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
