// Game by Nihar & Team.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),
	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	/** Called When Overlapping Area Sphere*/
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);

	/** Called When End Overlapping Area Sphere*/
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex);

	/** Sets the active stars array of bools based on rarity */
	void SetActiveStars();

	/** Sets the properties of items components based on the Item state */
	void SetItemProperties(EItemState State);

	/** Called when ItemInterpTimer is finished */
	void FinishInterping();

	/** Handles Item Interpolation when in EquipInterpolating State */
	void ItemInterp(float DeltaTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** Skeletal Mesh for the Item*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	USkeletalMeshComponent* ItemMesh;

	/** line Trace collides with the box to pop up HUD Widgets*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	class UBoxComponent* CollisionBox;

	/** Pop-up widget for when player looks at the item*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	class UWidgetComponent* PickupWidget;

	/** Enables Item Tracing when Overlapped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	class USphereComponent* AreaSphere;

	/** Name of the item which appears on the pickup widget*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	int32 ItemCount;

	/** Tells the number of stars to show on the pickup widget*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	TArray<bool> ActiveStars;

	/** Stores the state of the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	EItemState ItemState;

	/** The curve asset to use for the item's z location when interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	class UCurveFloat* ItemZCurve;

	/** Starting Location when the interping begins */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	FVector ItemInterpStartLocation;
	
	/** Target interp Location in front of camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	FVector CameraTargetLocation;

	/** true when interping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	bool bInterping;

	/** Plays when we start interping */
	FTimerHandle ItemInterpTimer;

	/** Duration of the curve and timer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	float ZCurveTime;

	/** Pointer to the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	class AShooterCharacter* Character;

	/** X and Y for the Item when Interpolating in the equipInterp state */
	float ItemInterpX;
	float ItemInterpY;

	/** Initial Yaw Offset between the camera and the interping Object */
	float InterpInitialYawOffset;

	/** Curve used to scale the Item when Interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	UCurveFloat* ItemScaleCurve;

	/** Sound played when item is picked up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	class USoundCue* PickupSound;

	/** Sound played when item is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	USoundCue* EquipSound;


public:

	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	void SetItemState(EItemState State);
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }

	/** Called From AShooterCharacter class */
	void StartItemCurve(AShooterCharacter* Char);

	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
};
