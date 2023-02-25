// Game by Nihar & Team.


#include "Weapon.h"

AWeapon::AWeapon() :
    ThrowWeaponTime(0.7f),
    bFalling(false),
    Ammo(30),
    MagazineCapacity(30),
    WeaponType(EWeaponType::EWT_SubmachineGun),
    AmmoType(EAmmoType::EAT_9mm),
    ReloadMontageSection(FName(TEXT("Reload SMG"))),
    bMovingClip(false),
    ClipBoneName(TEXT("smg_clip")) 
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Keep the weapon upright
    if(GetItemState() == EItemState::EIS_Falling && bFalling)
    {
        const FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
        GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void AWeapon::ThrowWeapon()
{
    FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

    const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
    const FVector MeshRight{ GetItemMesh()->GetRightVector()};
    // Direction in which the weapon will be thrown
    FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);
    float RandomRotation{30.f};
    ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
    ImpulseDirection *= 2000;
    GetItemMesh()->AddImpulse(ImpulseDirection);
    bFalling = true;
    GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime); 
}

void AWeapon::DecrementAmmo()
{
    if(Ammo - 1 <=0 )
    {
        Ammo = 0;
    }
    else
    {
        --Ammo;
    }
}

void AWeapon::ReloadAmmo(int32 Amount)
{
    checkf( Ammo + Amount <= MagazineCapacity, TEXT("Attempted to reload with more than magazine capacity"));
    Ammo += Amount;
}

void AWeapon::StopFalling()
{
    bFalling = false;
    SetItemState(EItemState::EIS_Pickup);
}