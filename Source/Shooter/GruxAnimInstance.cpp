// Game by Nihar & Team.


#include "GruxAnimInstance.h"
#include "Enemy.h"

void UGruxAnimInstance::UpdateAnimationProperties(float DeltTime)
{
	if (Enemy == nullptr)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	}

	if (Enemy)
	{
		FVector Velocity = Enemy->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
}
