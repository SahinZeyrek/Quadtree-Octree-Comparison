// Fill out your copyright notice in the Description page of Project Settings.


#include "QuadTree.h"
#include "Agent.h"
// Sets default values
AQuadTree::AQuadTree()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AQuadTree::BeginPlay()
{
	Super::BeginPlay();
}

void AQuadTree::EndPlay(const EEndPlayReason::Type reason)
{
	Super::EndPlay(reason);
	if (QueryCount > 0)
	{
		double averageQueryTime = TotalQueryTime / double(QueryCount);
		UE_LOG(LogTemp, Log, TEXT("Average QUADTREE Query time: %f ms over %d queries"), averageQueryTime, QueryCount);
	}
	if (InsertCount > 0)
	{
		double averageInsertTime = TotalInsertTime / double(InsertCount);
		UE_LOG(LogTemp, Log, TEXT("Average QUADTREE Insert time: %f ms over %d inserts"), averageInsertTime, InsertCount);

	}
}

void AQuadTree::Build(const FBox& bounds)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AQuadTree_Build)
	if (!bIsBuilt)
	{
		WorldBounds = bounds;
		root = MakeShared<FQuadTreeNode>(FBox2D(FVector2D(WorldBounds.Min),FVector2D(WorldBounds.Max)));
	}

}
void AQuadTree::VisualiseNode(UWorld* world, TSharedPtr<FQuadTreeNode> node, const FColor& color) const
{
	if (!node) return;
	if (!bvisualize) return;
	if (!node->IsLeaf())
	{
		for (auto& child : node->Children)
		{
			VisualiseNode(world, child);
		}
		return;
	}
	DrawDebugBox(world, FVector(node->Bounds.GetCenter(), 1 + node->Depth),
		FVector(node->Bounds.GetExtent(), 1 + node->Depth), color, false, 0.1f, node->Depth, 2.f);

}

void AQuadTree::VisualizeTree()
{
	VisualiseNode(GetWorld(), root);
}

void AQuadTree::Query(const FVector2D& queryLocation, TArray<AActor*>& outActors, AActor* queryInstigator)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AQuadTree_Query)
	double startTime = FPlatformTime::Seconds() * 1000.f;
	QueryNode(root, queryLocation, outActors, queryInstigator);
	double endTime =   FPlatformTime::Seconds() * 1000.f;
	TotalQueryTime += endTime - startTime;
	++QueryCount;

}
FColor AQuadTree::DepthToColor(int32 depth)
{

	// Clamp Depth to be within the range of [0, MaxDepth]
	depth = FMath::Clamp(depth, 0, MaxDepth);

	// Interpolation factor: 0 (base green) to 1 (full red)
	float Factor = static_cast<float>(depth) / static_cast<float>(MaxDepth);

	// Linearly interpolate between green and red
	int32 Red = FMath::Lerp(0, 255, Factor);   // From 0 to 255 as depth increases
	int32 Green = FMath::Lerp(255, 0, Factor); // From 255 to 0 as depth increases

	return FColor(Red, Green, 0); // Blue is always 0
}

void AQuadTree::QueryNode(TSharedPtr<FQuadTreeNode> node, const FVector2D& queryLocation, TArray<AActor*>& outActors, AActor* queryInstigator)
{
	//VisualiseNode(GetWorld(), node, FColor::Magenta);
	if (!node->Bounds.IsInside(queryLocation))
	{
		//VisualiseNode(GetWorld(), node, FColor::Red);
		return;
	}
	//if node has kids
	if (!node->IsLeaf())
	{
		//VisualiseNode(GetWorld(), node, FColor::Magenta);
		for (auto& child : node->Children)
		{
			QueryNode(child, queryLocation, outActors, queryInstigator);
		}
		return;
	}
	// if current node has no kids
	for (AActor* actor : node->Actors)
	{
		if (node->Bounds.IsInside(FVector2D(actor->GetActorLocation())))
		{
			if (actor != queryInstigator)
			{
				float zDistance = actor->GetActorLocation().Z - queryInstigator->GetActorLocation().Z;
				if (zDistance < zHeightTolerance)
				{
					outActors.AddUnique(actor);
					AAgent* agent = Cast<AAgent>(outActors.Top());
					agent->quadQueryResponder = node;
				}
				
			}
		}
	}
	AAgent* agent = Cast<AAgent>(queryInstigator);
	agent->quadQueryResponder = node;
	//VisualiseNode(GetWorld(), node, FColor::Blue);

	return;

}

void AQuadTree::Subdivide(TSharedPtr<FQuadTreeNode> node)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AQuadTree_Subdivide)
	if (!node)
	{
		return;
	}
	FVector2D min = node->Bounds.Min;
	FVector2D max = node->Bounds.Max;
	FVector2D center = (min + max) * 0.5f;

	FBox2D bottomLeft = FBox2D(min, center);
	FBox2D bottomRight = FBox2D(FVector2D(center.X, min.Y), FVector2D(max.X, center.Y));
	FBox2D topRight = FBox2D(center, max);
	FBox2D topLeft = FBox2D(FVector2D(min.X, center.Y), FVector2D(center.X, max.Y));

	// bottom left
	node->Children.AddUnique(MakeShared<FQuadTreeNode>(bottomLeft));
	// bottom right	
	node->Children.AddUnique(MakeShared<FQuadTreeNode>(bottomRight));
	// top right
	node->Children.AddUnique(MakeShared<FQuadTreeNode>(topRight));
	// top left
	node->Children.AddUnique(MakeShared<FQuadTreeNode>(topLeft));
	for (auto& child : node->Children)
	{
		child->Parent = node;
	}
}

void AQuadTree::Insert(AActor* actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AQuadTree_Insert)
	double startTime = FPlatformTime::Seconds() * 1000.f;
	if (!actor || !root->Bounds.IsInside(FVector2D(actor->GetActorLocation())))
	{
		return;
	}
	InsertNode(root, actor);
	double endTime = FPlatformTime::Seconds() * 1000.f;
	TotalInsertTime += endTime - startTime;
	++InsertCount;
}
void AQuadTree::InsertNode(TSharedPtr<FQuadTreeNode>& node, AActor* actor)
{
	if (!actor || !node->Bounds.IsInside(FVector2D(actor->GetActorLocation())))
	{
		return;
	}
	// if node has children
	if (!node->IsLeaf())
	{
		for (auto& child : node->Children)
		{
			InsertNode(child, actor);
		}
		return;
	}
	// if node doesnt have children and can insert new actors
	if (node->IsLeaf() && node->Actors.Num() < MaxActorsPerNode)
	{
		node->Actors.AddUnique(actor);
		return;
	}


	// if current node has no children
	if (node->Depth < MaxDepth)
	{
		Subdivide(node);
		for (auto& child : node->Children)
		{
			child->Depth = node->Depth + 1;
			//new actor to add
			if (child->Bounds.IsInside(FVector2D(actor->GetActorLocation())))
			{
				InsertNode(child, actor);
			}
			// actor from parent
			for (auto& parentActor : node->Actors)
			{
				if (child->Bounds.IsInside(FVector2D(parentActor->GetActorLocation())))
				{
					InsertNode(child, parentActor);
				}
			}
		}
		node->Actors.Empty();
		return;
	}
	// final depth has been reached + more than max amount agents per node, adding to node as a last resort
	node->Actors.Add(actor);
	return;

	//if (node->Actors.Num() < MaxActors)
	//{
	//	node->Actors.AddUnique(actor);
	//	return;
	//}
	// if there are more actors in the node
	//if (node->Actors.Num() >= MaxActors)
	//{
	//	if (node->Depth < MaxDepth)
	//	{
	//		Subdivide(node);
	//		//DFS
	//		for (auto& child : node->Children)
	//		{
	//			child->Depth = node->Depth + 1;
	//			if (child->Bounds.IsInside(FVector2D(actor->GetActorLocation())))
	//			{
	//				InsertNode(child, actor);
	//			}
	//			for (auto& childActor : node->Actors)
	//			{
	//				if (child->Bounds.IsInside(FVector2D(childActor->GetActorLocation())))
	//				{
	//					InsertNode(child, childActor);
	//				}
	//			}
	//
	//		}
	//		node->Actors.Empty();
	//		return;
	//	}
	//	node->Actors.AddUnique(actor);
	//	return;
	//}
	// if max depth has been reached
	//node->Children.Empty();
}



void AQuadTree::RemoveActorFromNode(TSharedPtr<FQuadTreeNode> node, AActor* actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AQuadTree_RemoveActorFromNode)
	node->Actors.Remove(actor);
	TArray<AActor*> toRemove;
	if (node->IsLeaf() && node->Actors.IsEmpty())
	{
		for (auto& sibling : node->Parent->Children)
		{
			if (sibling != node)
			{
				if (!sibling->IsLeaf())
				{
					continue;
				}
				for (auto& siblingActor : sibling->Actors)
				{
					if (!sibling->Bounds.IsInside(FVector2D(siblingActor->GetActorLocation())))
					{
						toRemove.Add(siblingActor);
					}
				}
				for (auto& actor : toRemove)
				{
					sibling->Actors.Remove(actor);
				}
			}
		}
		bool bClear = true;
		for (auto& sibling : node->Parent->Children)
		{
			if (!sibling->Actors.IsEmpty())
			{
				bClear = false;
				break;
			}
			bClear = true;
		}
		if (bClear)
		{
			node->Parent->Children.Empty();
		}
	}
}

bool AQuadTree::IsInsideBounds(AActor* actor)
{
	return WorldBounds.IsInside(actor->GetActorLocation());
}

void AQuadTree::ClearTree()
{
	TSharedPtr<FQuadTreeNode> previous = MakeShared<FQuadTreeNode>(root->Bounds);
	ClearNode(root, previous,Parents);
	Parents.Empty();
	Build(WorldBounds);
}
void AQuadTree::ClearNode(TSharedPtr<FQuadTreeNode>& node, TSharedPtr<FQuadTreeNode>& previous, TArray<TSharedPtr<FQuadTreeNode>>& parents)
{
	// if node has kids

	if (!node->IsLeaf())
	{
		parents.Add(node);
		for (auto& child : node->Children)
		{
			ClearNode(child, node,parents);
		}
		return;
	}
	// if it doesnt have kids, return;
	return;
}
// Called every frame
void AQuadTree::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//if (bvisualize)
	//{
	VisualizeTree();
	//}
}


