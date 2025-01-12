// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuadTree.h"
#include "Octree.h"
#include "Gradwork/GradworkGameMode.h"
#include "Agent.generated.h"
UENUM()
enum class ESteeringType : uint8
{
	seperation,
	allignment,
	cohesion
};
UCLASS()
class GRADWORK_API AAgent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAgent();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void QueryTree();
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<AActor*> OtherActors;
	UPROPERTY(EditAnyWhere,BlueprintReadWrite)
	float Speed;
	TSharedPtr<FOctreeNode> octQueryResponder;
	TSharedPtr<FQuadTreeNode> quadQueryResponder;
	float seperationWeight = 0.f;
	float seperationRange = 300.f;
	float allignmentWeight = 0.f;
	float cohesionWeight   = 0.f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	ETreeType TreeType;
	FVector Direction;
	AQuadTree* QuadTree;
	AOctree* Octree;
	ESteeringType SteeringType;
};
