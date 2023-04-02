// Game by Nihar & Team.


#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Enemy.h"


AEnemyController::AEnemyController()
{
	BlackBoardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	check(BlackBoardComponent);

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	check(BehaviorTreeComponent);
}

void AEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (InPawn == nullptr) return;

	AEnemy* Enemy = Cast<AEnemy>(InPawn);
	if (Enemy)
	{
		if (Enemy->GetBehaviorTree())
		{
			BlackBoardComponent->InitializeBlackboard(*(Enemy->GetBehaviorTree()->BlackboardAsset));
		}
	}
}