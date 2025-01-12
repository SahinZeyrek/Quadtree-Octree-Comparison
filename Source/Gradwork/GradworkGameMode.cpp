// Copyright Epic Games, Inc. All Rights Reserved.

#include "GradworkGameMode.h"
#include "GradworkCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AGradworkGameMode::AGradworkGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AGradworkGameMode::StartPlay()
{
	Super::StartPlay();

	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), QuadTree->StaticClass(), actors);
	QuadTree = Cast<AQuadTree>(actors[0]);
	actors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), Octree->StaticClass(), actors);
	Octree = Cast<AOctree>(actors[0]);


}

AQuadTree* AGradworkGameMode::GetQuadTree()
{
	if (!QuadTree)
	{
		TArray<AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), QuadTree->StaticClass(), actors);
		QuadTree = Cast<AQuadTree>(actors[0]);
	}
	return QuadTree;
}

AOctree* AGradworkGameMode::GetOctree()
{
	if (!Octree)
	{
		TArray<AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), Octree->StaticClass(), actors);
		Octree = Cast<AOctree>(actors[0]);
	}
	return Octree;
}

ETreeType AGradworkGameMode::GetTreeType() const
{
	return treeType;
}
