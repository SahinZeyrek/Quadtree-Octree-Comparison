// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuadTree.generated.h"

USTRUCT()
struct FQuadTreeNode
{
	GENERATED_BODY()
	FBox2D Bounds;
	TArray<AActor*> Actors;
	TArray<TSharedPtr<FQuadTreeNode>> Children;
	TSharedPtr<FQuadTreeNode> Parent;
	int32 Depth;
	FQuadTreeNode() = default;
	FQuadTreeNode(const FBox2D& InBounds)
		: Bounds(InBounds) 
	{
		Depth = 0;
	}
	~FQuadTreeNode() 
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Quadtree node destoryed"));
	}

	bool IsLeaf() const { return Children.IsEmpty(); }
};
UCLASS()
class GRADWORK_API AQuadTree : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AQuadTree();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	TSharedPtr<FQuadTreeNode> root;
	UFUNCTION(BlueprintCallable)
	void Build(const FBox& bounds);
	UFUNCTION(BlueprintCallable)
	void Insert(AActor* actor);
	UFUNCTION(BlueprintCallable)
	void Query(const FVector2D& queryLocation,TArray<AActor*>& outActors, AActor* queryInstigator);
	UFUNCTION(BlueprintCallable)
	FColor DepthToColor(int32 depth);

	void RemoveActorFromNode(TSharedPtr<FQuadTreeNode> node, AActor* actor);
	bool IsInsideBounds(AActor* actor);
	FBox GetWorldBounds() const { return WorldBounds; }
	UFUNCTION(BlueprintCallable)
	void ClearTree();
	UPROPERTY(EditAnywhere, Category = "Init")
	bool bvisualize = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> allActors;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float zHeightTolerance = 100.f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type reason) override;

private:	
	void Subdivide(TSharedPtr<FQuadTreeNode> node);
	void InsertNode(TSharedPtr<FQuadTreeNode>& node, AActor* actor);
	void QueryNode(TSharedPtr<FQuadTreeNode> node, const FVector2D& queryLocation, TArray<AActor*>& outActors, AActor* queryInstigator);
	void VisualiseNode(UWorld* world, TSharedPtr<FQuadTreeNode> node,const FColor& color = FColor::Green)const;
	void VisualizeTree();
	void ClearNode(TSharedPtr<FQuadTreeNode>& node, TSharedPtr<FQuadTreeNode>& previous, TArray<TSharedPtr<FQuadTreeNode>>& parents);
	UPROPERTY(EditAnywhere, Category = "Init")
	int32 MaxDepth = 4;
	UPROPERTY(EditAnywhere, Category = "Init")
	int32 MaxActorsPerNode = 4;
	UPROPERTY(EditAnywhere, Category = "Init")
	float queryRadius = 100.f;
	UPROPERTY(EditAnywhere, Category = "Init")
	FBox WorldBounds;
	bool bIsBuilt = false;
	TArray<TSharedPtr<FQuadTreeNode>> Parents;
	int32 QueryCount;
	double TotalQueryTime;
	int32 InsertCount;
	double TotalInsertTime;
};
