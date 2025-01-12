// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Octree.generated.h"

USTRUCT()
struct FOctreeNode
{
	GENERATED_BODY()
	//UPROPERTY()
	FBox Bounds;
	//UPROPERTY();
	TArray<AActor*> Actors;
	//UPROPERTY();
	TArray<TSharedPtr<FOctreeNode>> Children;
	TSharedPtr<FOctreeNode> Parent;
	int32 Depth;
	FOctreeNode() = default;
	FOctreeNode(const FBox& bounds) : Bounds(bounds)
	{
		Depth = 0;
	}
	~FOctreeNode()
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Octree node destroyed"));
	}
	bool IsLeaf() const { return Children.IsEmpty(); };
};
UCLASS()
class GRADWORK_API AOctree : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOctree();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	TSharedPtr<FOctreeNode> root;
	UFUNCTION(BlueprintCallable)
	void Build(const FBox& bounds);
	UFUNCTION(BlueprintCallable)
	void Insert(AActor* actor);
	UFUNCTION(BlueprintCallable)
	void Query(const FVector& queryLocation, TArray<AActor*>& outActors, AActor* queryInstigator);
	UFUNCTION(BlueprintCallable)
	void ClearTree(bool rebuild);

	void RemoveActorFromNode(TSharedPtr<FOctreeNode> node, AActor* actor);
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Init")
	float TreeHeight = 100.f;
	UPROPERTY(EditAnywhere, Category = "Init")
	bool bvisualize = false;
	bool IsInsideBounds(AActor* actor);
	FBox GetWorldBounds() const { return WorldBounds; }
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<AActor*> allActors;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type reason) override;
private:	
	void Subdivide(TSharedPtr<FOctreeNode> node);
	void InsertNode(TSharedPtr<FOctreeNode>& node, AActor* actor);
	void QueryNode(TSharedPtr<FOctreeNode> node, const FVector& queryLocation, TArray<AActor*>& outActors, AActor* queryInstigator);
	void VisualiseNode(UWorld* world, TSharedPtr<FOctreeNode> node, const FColor& color = FColor::Green)const;
	void VisualiseTree();
	void ClearNode(TSharedPtr<FOctreeNode>& node, TSharedPtr<FOctreeNode>& previous, TArray<TSharedPtr<FOctreeNode>>& parents);
	UPROPERTY(EditAnywhere, Category = "Init")
	int32 MaxDepth = 4;
	UPROPERTY(EditAnywhere, Category = "Init")
	int32 MaxActorsPerNode = 4;

	UPROPERTY(EditAnywhere, Category = "Init")
	FBox WorldBounds;
	bool bIsBuilt = false;
	TArray<TSharedPtr<FOctreeNode>> Parents;
	int32 QueryCount;
	double TotalQueryTime;
	int32 InsertCount;
	double TotalInsertTime;
};
