// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "QuadTree.h"
#include "Octree.h"
#include "GradworkGameMode.generated.h"
UENUM(BlueprintType)
enum class ETreeType : uint8 
{
	none,
	quadtree,
	octree
};
UCLASS(minimalapi)
class AGradworkGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AGradworkGameMode();
	virtual void StartPlay() override;
	AQuadTree* GetQuadTree() ;
	AOctree* GetOctree() ;
	ETreeType GetTreeType()const;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	ETreeType treeType = ETreeType::quadtree;
private:
	AQuadTree* QuadTree;
	AOctree* Octree;
};



