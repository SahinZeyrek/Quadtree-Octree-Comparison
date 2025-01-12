// Fill out your copyright notice in the Description page of Project Settings.


#include "Agent.h"
#include "Kismet/GameplayStatics.h"
#include "Gradwork/GradworkGameMode.h"
// Sets default values
AAgent::AAgent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAgent::BeginPlay()
{
	Super::BeginPlay();
	auto gameMode = Cast<AGradworkGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	TreeType = gameMode->GetTreeType();
	switch (TreeType)
	{
	case ETreeType::none:
		Octree = gameMode->GetOctree();
		// the octree gets assigned here just for the world bounds and nothing else. 
		// either trees get constructed in the level anyways
		// other actor array gets filled in blueprints 
		break;
	case ETreeType::quadtree:
		QuadTree = gameMode->GetQuadTree();
		QuadTree->Insert(this);

		break;
	case ETreeType::octree:
		Octree = gameMode->GetOctree();
		Octree->Insert(this);

		break;
	default:
		break;
	}
	OtherActors.Reserve(100);

	Direction.X = FMath::Rand() % 2 ? 1 : -1;
	Direction.Y = FMath::Rand() % 2 ? 1 : -1;
	Direction.Z = FMath::Rand() % 2 ? 1 : -1;

	SteeringType = ESteeringType::seperation;
	//SteeringType = ESteeringType(FMath::RandRange(0,2));

}

// Called every frame
void AAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	QueryTree();
	// do flocking here.
	FVector flockingVector = FVector::ZeroVector;
	if (!OtherActors.IsEmpty())
	{
		switch (SteeringType)
		{
		case ESteeringType::seperation:
			for (auto& neighbour : OtherActors)
			{
				FVector toNeighbour = neighbour->GetActorLocation() - GetActorLocation();
				float distance = toNeighbour.Size();
				if (distance < seperationRange)
				{
					flockingVector -= toNeighbour / distance;
				}
			}
			break;
		case ESteeringType::allignment:
			for (auto& neighbour : OtherActors)
			{
				AAgent* agent = Cast<AAgent>(neighbour);
				flockingVector += agent->Direction * Speed;
			}

			break;
		case ESteeringType::cohesion:
			
			for (auto& neighbour : OtherActors)
			{
				flockingVector += neighbour->GetActorLocation();
			}
			flockingVector /= OtherActors.Num();
			//flockingVector.Normalize();
			break;
		default:
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Emerald, TEXT(" no valid steering enum"));
			break;
		}
	}
	FVector finalDirection = (Direction * 1.f + flockingVector * 5.f);
	finalDirection.Normalize();
	FVector newLocation = (finalDirection * Speed * DeltaTime) + GetActorLocation();
	SetActorLocation(newLocation, false);

	switch (TreeType)
	{
		// octree bounds is being used just for simplicity sake 
	case ETreeType::none:
		if (!Octree->IsInsideBounds(this))
		{
			FVector loc = FMath::RandPointInBox(Octree->GetWorldBounds());
			SetActorLocation(loc, false);

			Direction.X = FMath::Rand() % 2 ? 1 : -1;
			Direction.Y = FMath::Rand() % 2 ? 1 : -1;
			Direction.Z = FMath::Rand() % 2 ? 1 : -1;
		}
		break;
	case ETreeType::quadtree:
		if (!QuadTree->IsInsideBounds(this))
		{
			FVector loc = FMath::RandPointInBox(QuadTree->GetWorldBounds());
			SetActorLocation(loc, false);

			Direction.X = FMath::Rand() % 2 ? 1 : -1;
			Direction.Y = FMath::Rand() % 2 ? 1 : -1;
			Direction.Z = FMath::Rand() % 2 ? 1 : -1;
		}
		if (quadQueryResponder && !quadQueryResponder->Bounds.IsInside(FVector2D(this->GetActorLocation())))
		{
			QuadTree->RemoveActorFromNode(quadQueryResponder, this);
			QuadTree->Insert(this);
		}
		break;
	case ETreeType::octree:
		if (!Octree->IsInsideBounds(this))
		{
			FVector loc = FMath::RandPointInBox(Octree->GetWorldBounds());
			SetActorLocation(loc, false);

			Direction.X = FMath::Rand() % 2 ? 1 : -1;
			Direction.Y = FMath::Rand() % 2 ? 1 : -1;
			Direction.Z = FMath::Rand() % 2 ? 1 : -1;
		}
		if (octQueryResponder && !octQueryResponder->Bounds.IsInside(this->GetActorLocation()))
		{
			Octree->RemoveActorFromNode(octQueryResponder, this);
			Octree->Insert(this);
		}
		break;
	default:
		break;
	}
	
}

void AAgent::QueryTree()
{
	switch (TreeType)
	{
	case ETreeType::none:

		break;
	case ETreeType::quadtree:
		QuadTree->Query(FVector2D(this->GetActorLocation()), OtherActors,this);
		break;
	case ETreeType::octree:
		Octree->Query(GetActorLocation(), OtherActors, this);

		break;
	default:
		break;
	}
}

