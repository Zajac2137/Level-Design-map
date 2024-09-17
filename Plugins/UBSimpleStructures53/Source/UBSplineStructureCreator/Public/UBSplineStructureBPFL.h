// Copyright - Unreality Bites 2023

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#if WITH_EDITOR
	#include "Engine/MeshMerging.h"
#endif
#include "UBSplineStructureBPFL.generated.h"

class ALandscape;

UCLASS()
class UBSPLINESTRUCTURECREATOR_API UUBSplineStructureBPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
private:

	UFUNCTION(BlueprintCallable, Category = "UB Spline")
		static UObject* MergeStaticMeshes(AActor* ActorToMerge, bool UseNanite, bool CombineBuilding, FString AssetPath);

	UFUNCTION(BlueprintCallable, Category = "UB Spline")
		static void LockActor(AActor* ActorToLock, bool bLockStatus);

	UFUNCTION(BlueprintCallable, Category = "UB Spline")
		static void RerunConstructionScript(AActor* ActorToConstruct);

	UFUNCTION(BlueprintCallable, Category = "UB Spline")
	static float GetLandscapeHeightScale(ALandscape* Landscape);
};
