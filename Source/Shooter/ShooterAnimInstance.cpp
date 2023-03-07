// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


UShooterAnimInstance::UShooterAnimInstance() : 
    Speed(0.f),
    bIsInAir(false),
    bIsAccelerating(false),
    MovementOffsetYaw(0.f),
    LastMovementOffsetYaw(0.f),
    bAiming(false),
    TIPCharacterYaw(0.f),
    TIPCharacterYawLastFrame(0.f),
    CharacterRotation(FRotator(0.f)),
    CharacterRotationLastFrame(FRotator(0.f)),
    RootYawOffset(0.f),
    YawDelta(0.f),
    Pitch(0.f),
    bReloading(false),
    OffsetState(EOffsetState::EOS_Hip),
    bCrouching(false),
    RecoilWeight(1.f),
    bTurningInPlace(false)
{

}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if(ShooterCharacter == nullptr){
        ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());    
    }
    if(ShooterCharacter){
        bCrouching = ShooterCharacter->GetCrouching();
        bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading; 
        bEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;

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

        if(bReloading)
        {
            OffsetState = EOffsetState::EOS_Reloading;
        }
        else if(bIsInAir)
        {
            OffsetState = EOffsetState::EOS_InAir;
        }
        else
        {
            OffsetState = EOffsetState::EOS_Hip;
        }
    }

    TurnInPlace(); 
    Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
    if(ShooterCharacter == nullptr) return;

    Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

    if(Speed > 0 || bIsInAir) 
    {
        // Dont want to turn in place when character is moving 
        RootYawOffset = 0.f;
        TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        RotationCurveLastFrame = 0.f;
        RotationCurve = 0.f;
    }
    else
    {
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        const float TIPYawDelta = TIPCharacterYaw - TIPCharacterYawLastFrame;
        // Root yaw offset updated and clamped to -180 and 180
        RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

        // 1.0 if truning and 0.0 if not 
        const float Turning{ GetCurveValue(TEXT("Turning")) };
        if(Turning > 0)
        {
            bTurningInPlace = true;
            RotationCurveLastFrame = RotationCurve;
            RotationCurve = GetCurveValue(TEXT("Rotation"));
            const float DeltaRotation{RotationCurve - RotationCurveLastFrame};

            // If RootYawOffset is positive then this means we are turning left and
            // If RootYawOffset is negative then this means we are turning right
            RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

            const float ABSRootYawOffset{FMath::Abs(RootYawOffset)};
            if(ABSRootYawOffset > 90)
            {
                const float YawExcess{ ABSRootYawOffset - 90 };
                RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
            }    
            if(GEngine)
            {
                GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset));
            }
        }
        else
        {
            bTurningInPlace = false;
        }
    }

    // Set The Recoil Weight    
    if(bTurningInPlace)
        {
            if(bReloading || bEquipping)
            {
                RecoilWeight = 1.f;
            }
            else
            {
                RecoilWeight = 0.f;
            }
        }
        else  // Not Turning in place
        {
            if(bCrouching)
            {
                 if(bReloading || bEquipping)
                {
                    RecoilWeight = 1.f;
                }
                else
                {
                    RecoilWeight = 0.1f;
                }
            }
            else
            {
                if(bAiming || bReloading || bEquipping)
                {
                    RecoilWeight = 1.f;
                }
                else
                {
                    RecoilWeight = 0.5f;
                }
            }
        }
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
    if(ShooterCharacter == nullptr ) return;

    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = ShooterCharacter->GetActorRotation();

    const FRotator Delta{UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame)};

    /** The Reason here we divide by deltatime instead of multiplying is because we want to make the result of substraction 
     *  bigger with bigger changes in Delta, so we divide it by delta time which is a small number which makes the substraction 
     *  result larger.
    */
    const float Target = Delta.Yaw / DeltaTime ;

    const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };

    YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
    if(GEngine)
    {
        GEngine->AddOnScreenDebugMessage(2, -1, FColor::Blue, FString::Printf(TEXT("YawDelta: %f"), YawDelta));
    }
    if(GEngine)
    {
        GEngine->AddOnScreenDebugMessage(3, -1, FColor::Blue, FString::Printf(TEXT("Delta.Yaw: %f"), Delta.Yaw));
    }
}
