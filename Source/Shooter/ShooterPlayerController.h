// Game by Nihar & Team.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AShooterPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	/** Reference to overall HUD Overlay class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> HUDOverlayClass;

	/** Variable to hold HUD Overlay Widget after creating it */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Widget", meta = (AllowPrivateAccess = "true"))
	UUserWidget* HUDOverlay;

	
};
