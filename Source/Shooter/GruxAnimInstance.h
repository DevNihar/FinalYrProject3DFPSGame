// Game by Nihar & Team.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GruxAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UGruxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltTime);

private:
	/** Lateral movement Speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	/** Lateral movement Speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class AEnemy* Enemy;
};
