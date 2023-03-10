// Game by Nihar & Team.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
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

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),
	EIT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;
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
	virtual void SetItemProperties(EItemState State);

	/** Called when ItemInterpTimer is finished */
	void FinishInterping();

	/** Handles Item Interpolation when in EquipInterpolating State */
	void ItemInterp(float DeltaTime);

	/** Get InterpLocation based on Item type */
	FVector GetInterpLocation();

	void PlayPickupSound(bool bForcePlaySound = false);

	virtual void InitializeCustomDepth();

	/** C++ version of Construction script This script runs before running the game */
	virtual void OnConstruction(const FTransform& Transform) override;	

	void EnableGlowMaterial();

	void UpdatePulse();

	void ResetPulseTimer();

	void StartPulseTimer();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called in AshooterCharacter::GetPickupItem
	void PlayEquipSound(bool bForcePlaySound = false);

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Rarity, meta = (AllowPrivateAccess="true"))
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

	/** Enum for the type of item this item is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	EItemType ItemType;

	/** index of the Interp location this item is Interping to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	int32  InterpLocIndex;

	/** Index of the material we'd like to change at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	int32 MaterialIndex;

	/** Dynamic Instance that we can change at run time */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	/** material Instance used with the dynamic material instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	UMaterialInstance* MaterialInstance;

	bool bCanChangeCustomDepth;

	/** Curve to drive the dynamic material parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	class UCurveVector* PulseCurve;

	FTimerHandle PulseTimer;

	/** Time for the pulse timer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	float GlowAmount;

	UPROPERTY(VisibleAnywhere, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	float FresnelExponent;

	UPROPERTY(VisibleAnywhere, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	float FresnelReflectFraction;

	/** Curve to drive the dynamic material parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	UCurveVector* InterpPulseCurve;

	/** The Icon for this item in the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Inventory, meta = (AllowPrivateAccess="true"))
	UTexture2D* IconItem;

	/** The AmmoIcon for this item in the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Inventory, meta = (AllowPrivateAccess="true"))
	UTexture2D* AmmoIcon;

	/** Slot in the inventory array */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Inventory, meta = (AllowPrivateAccess="true"))
	int32 SlotIndex;

	/** True when Character Inventory is full */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Inventory, meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=DataTable, meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDatatable;

	/** Color in the glow material*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	/** Color in the glow material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	/*Color in the glow material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	/** Number Of stars in the pickup widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	/** Background Icon for the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;

public:

	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	void SetItemState(EItemState State);
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE void SetCharacter(AShooterCharacter* Char) { Character = Char; }

	/** Called From AShooterCharacter class */
	void StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound = false);

	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE void SetPickupSound(USoundCue* Sound) { PickupSound = Sound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE void SetEquipSound(USoundCue* Sound) { EquipSound = Sound; }

	FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }
	
	// Set Item Icon for the inventory 
	FORCEINLINE void SetIconItem(UTexture2D* Icon) { IconItem = Icon; }
	// Set AmmoIcon for the pickup widget
	FORCEINLINE void SetAmmoIcon(UTexture2D* Icon) { AmmoIcon = Icon; }

	FORCEINLINE int32 GetItemCount() const { return ItemCount; }

	virtual void EnableCustomDepth();

	virtual void DisableCustomDepth();

	void DisableGlowMaterial();

	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	FORCEINLINE void SetCharacterInventoryFull(bool bFull) { bCharacterInventoryFull = bFull; }
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance) { MaterialInstance = Instance; }
	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }

	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* Instance) { DynamicMaterialInstance = Instance; }
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }

	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }
	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }
	FORCEINLINE void SetMaterialIndex(int32 Index) { MaterialIndex = Index; }

};
