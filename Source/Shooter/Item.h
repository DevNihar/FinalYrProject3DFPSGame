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

	/** Seta the active stars array of bools based on rarity */
	void SetActiveStars();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,  Category= "Item Properties", meta = (AllowPrivateAccess="true"))
	TArray<bool> ActiveStars;

public:

	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; };


};
