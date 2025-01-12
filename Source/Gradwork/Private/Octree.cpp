// Fill out your copyright notice in the Description page of Project Settings.


#include "Octree.h"
#include "Agent.h"
// Sets default values
AOctree::AOctree()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

bool AOctree::IsInsideBounds(AActor* actor)
{
	return WorldBounds.IsInside(actor->GetActorLocation());
}

// Called when the game starts or when spawned
void AOctree::BeginPlay()
{
	Super::BeginPlay();

}
void AOctree::EndPlay(EEndPlayReason::Type reason)
{
	Super::EndPlay(reason);

	if (QueryCount > 0)
	{
		double averageQueryTime = TotalQueryTime / double(QueryCount);
		UE_LOG(LogTemp, Log, TEXT("Average OCTREE Query time: %f ms over %d queries"), averageQueryTime, QueryCount);
	}
	if (InsertCount > 0)
	{
		double averageInsertTime = TotalInsertTime / double(InsertCount);
		UE_LOG(LogTemp, Log, TEXT("Average OCTREE Insert time: %f ms over %d inserts"), averageInsertTime, InsertCount);

	}
}
void AOctree::Build(const FBox& bounds)
{
	if (!bIsBuilt)
	{
		WorldBounds = bounds;
		root = MakeShared<FOctreeNode>(WorldBounds);
		bIsBuilt = true;
	}
}


void AOctree::Subdivide(TSharedPtr<FOctreeNode> node)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AOctree_Subdivide)

	FBox bounds = node->Bounds;
	FVector center = node->Bounds.GetCenter();

	//https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/introduction-to-octrees-r3529/#:~:text=Initializing%20the%20enclosing%20region,objects%20in%20the%20game%20world.
	FBox octant0 = FBox(bounds.Min, center);
	FBox octant1 = FBox(FVector(center.X, bounds.Min.Y, bounds.Min.Z), FVector(bounds.Max.X, center.Y, center.Z));
	FBox octant2 = FBox(FVector(center.X, bounds.Min.Y, center.Z), FVector(bounds.Max.X, center.Y, bounds.Max.Z));
	FBox octant3 = FBox(FVector(bounds.Min.X, bounds.Min.Y, center.Z), FVector(center.X, center.Y, bounds.Max.Z));
	FBox octant4 = FBox(FVector(bounds.Min.X, center.Y, bounds.Min.Z), FVector(center.X, bounds.Max.Y, center.Z));
	FBox octant5 = FBox(FVector(center.X, center.Y, bounds.Min.Z), FVector(bounds.Max.X, bounds.Max.Y, center.Z));
	FBox octant6 = FBox(center, bounds.Max);
	FBox octant7 = FBox(FVector(bounds.Min.X, center.Y, center.Z), FVector(center.X, bounds.Max.Y, bounds.Max.Z));
	node->Children.Empty();
	node->Children.Add(MakeShared<FOctreeNode>(octant0));
	node->Children.Add(MakeShared<FOctreeNode>(octant1));
	node->Children.Add(MakeShared<FOctreeNode>(octant2));
	node->Children.Add(MakeShared<FOctreeNode>(octant3));
	node->Children.Add(MakeShared<FOctreeNode>(octant4));
	node->Children.Add(MakeShared<FOctreeNode>(octant5));
	node->Children.Add(MakeShared<FOctreeNode>(octant6));
	node->Children.Add(MakeShared<FOctreeNode>(octant7));
	for (auto& child : node->Children)
	{
		child->Parent = node;
	}


}
void AOctree::Insert(AActor* actor)
{

	TRACE_CPUPROFILER_EVENT_SCOPE(AOctree_Insert)
	double startTime = FPlatformTime::Seconds() * 1000.f;
	InsertNode(root, actor);
	double endTime = FPlatformTime::Seconds() * 1000.f;
	TotalInsertTime += endTime - startTime;
	++InsertCount;
}
void AOctree::InsertNode(TSharedPtr<FOctreeNode>& node, AActor* actor)
{
	if (!actor || !node->Bounds.IsInside((actor->GetActorLocation())))
	{
		return;
	}
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
			if (child->Bounds.IsInside(actor->GetActorLocation()))
			{
				InsertNode(child, actor);
			}
			// actor from parent
			for (auto& parentActor : node->Actors)
			{
				if (child->Bounds.IsInside(parentActor->GetActorLocation()))
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
}

void AOctree::Query(const FVector& queryLocation, TArray<AActor*>& outActors, AActor* queryInstigator)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AOctree_Query)

	double startTime = FPlatformTime::Seconds() * 1000.f;
	QueryNode(root, queryLocation, outActors, queryInstigator);
	double endTime = FPlatformTime::Seconds() * 1000.f;

	TotalQueryTime += endTime - startTime;
	++QueryCount;
}

void AOctree::QueryNode(TSharedPtr<FOctreeNode> node, const FVector& queryLocation, TArray<AActor*>& outActors, AActor* queryInstigator)
{
	//VisualiseNode(GetWorld(), node, FColor::Magenta);
	if (!node->Bounds.IsInside(queryLocation))
	{
		//	VisualiseNode(GetWorld(), node, FColor::Red);
		return;
	}
	//if node has kids
	if (!node->IsLeaf())
	{
		//	VisualiseNode(GetWorld(), node, FColor::Magenta);
		for (auto& child : node->Children)
		{
			QueryNode(child, queryLocation, outActors, queryInstigator);
		}
		return;
	}
	// if current node has no kids
	for (AActor* actor : node->Actors)
	{
		if (node->Bounds.IsInside(actor->GetActorLocation()))
		{
			if (actor != queryInstigator)
			{
				outActors.AddUnique(actor);
				AAgent* agent = Cast<AAgent>(outActors.Top());
				agent->octQueryResponder = node;

			}
		}

	}
	AAgent* agent = Cast<AAgent>(queryInstigator);
	agent->octQueryResponder = node;
	//VisualiseNode(GetWorld(), node, FColor::Blue);

	return;
}

void AOctree::VisualiseNode(UWorld* world, TSharedPtr<FOctreeNode> node, const FColor& color) const
{
	if (!node) return;
	if (!bvisualize) return;
	//if (node->Children.IsEmpty()) return;
	if (!node->IsLeaf())
	{
		for (auto& child : node->Children)
		{
			VisualiseNode(world, child);
		}
		return;
	}
	//FColor color = DepthToColor(node->Depth);
	DrawDebugBox(world, node->Bounds.GetCenter(),
		node->Bounds.GetExtent(), color, false, 0.1f, node->Depth, 2.f);

}

void AOctree::VisualiseTree()
{
	VisualiseNode(GetWorld(), root);
}

void AOctree::ClearTree(bool rebuild)
{
	TSharedPtr<FOctreeNode> previous = MakeShared<FOctreeNode>(root->Bounds);
	ClearNode(root, previous, Parents);
	Parents.Empty();
	bIsBuilt = false;
	if (rebuild)
	{
		Build(WorldBounds);
	}
}

void AOctree::RemoveActorFromNode(TSharedPtr<FOctreeNode> node, AActor* actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AOctree_RemoveActorFromNode)

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
					if (!sibling->Bounds.IsInside(siblingActor->GetActorLocation()))
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
	//ClearTree(true);
	//for (auto& agent : allActors)
	//{
	//	Insert(agent);
	//}
	//if (node->IsLeaf() && node->Actors.IsEmpty())
	//{
	//	bool bClear = false;
	//	for (auto& sibling : node->Parent->Children)
	//	{
	//		if (sibling != node)
	//		{
	//			if (!sibling->IsLeaf())
	//			{
	//				
	//			}
	//			for (auto& siblingActor : sibling->Actors)
	//			{
	//				if (!sibling->Bounds.IsInside(siblingActor->GetActorLocation()))
	//				{
	//					toRemove.Add(siblingActor);
	//				}
	//			}
	//			for (auto& actor : toRemove)
	//			{
	//				sibling->Actors.Remove(actor);
	//			}
	//			if (sibling->Actors.IsEmpty())
	//			{
	//				bClear = true;
	//				continue;
	//			}
	//			bClear = false;
	//			break;
	//		}
	//		bClear = true;
	//	}
	//	if (bClear)
	//	{
	//		node->Parent->Children.Empty();
	//		return;
	//	}
	//	return;
	//}
}

void AOctree::ClearNode(TSharedPtr<FOctreeNode>& node, TSharedPtr<FOctreeNode>& previous, TArray<TSharedPtr<FOctreeNode>>& parents)
{
	// if node has kids
	if (!node->IsLeaf())
	{
		//parents.Add(node);
		for (auto& child : node->Children)
		{
			ClearNode(child, node, parents);
		}
		return;
	}
	// if it doesnt have kids, return;
	return;
}


// Called every frame
void AOctree::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//if (bIsBuilt)
	//{
	//	ClearTree(true);
	//	for (auto& agent : allActors)
	//	{
	//		Insert(agent);
	//	}
	//}
	VisualiseTree();

}





