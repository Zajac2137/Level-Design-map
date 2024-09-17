// Copyright - Unreality Bites 2023

#include "UBSplineStructureBPFL.h"
#if WITH_EDITOR
#include "ComponentReregisterContext.h"
#include "Editor.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#endif
#include "Landscape.h"

// Merge all static meshes and instanced static meshes from spline structure
UObject* UUBSplineStructureBPFL::MergeStaticMeshes(AActor* ActorToMerge, const bool UseNanite,
                                                   const bool CombineBuilding, const FString AssetPath)
{
#if WITH_EDITOR
	const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked
		<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();

	// Merge Settings
	UWorld* World = ActorToMerge->GetWorld();
	const FString AssetPrefix = (AssetPath.Len() == 0) ? TEXT("SM_") : TEXT("/SM_");
	const FString AssetSuffix = CombineBuilding ? TEXT("BUILDING_") : TEXT("STRUCTURE_");
	const FString PackageName = TEXT("/Game/") + AssetPath + AssetPrefix + AssetSuffix + ActorToMerge->GetActorLabel();
	FMeshMergingSettings MeshMergingSettings;
	MeshMergingSettings.NaniteSettings.bEnabled = UseNanite;
	constexpr float ScreenAreaSize = TNumericLimits<float>::Max();

	TArray<UPrimitiveComponent*> PrimitiveComponents;

	// Get all Static Mesh Primitives
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	ActorToMerge->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		if (StaticMeshComponent->GetStaticMesh()) // ensure static mesh exists (roof, stairs and stair walls might not)
		{
			if (CombineBuilding)
				PrimitiveComponents.Add(StaticMeshComponent);
			else
			// ignore generated elements, leaving only the main spline structure
			{
				const FString MeshName = StaticMeshComponent->GetStaticMesh().GetName();
				if (!(MeshName.StartsWith("SM_GEN"))) PrimitiveComponents.Add(StaticMeshComponent);
			}
		}

	// Get all Instanced Static Meshes (e.g. Pillars)
	TArray<UInstancedStaticMeshComponent*> InstancedStaticMeshComponents;
	ActorToMerge->GetComponents<UInstancedStaticMeshComponent>(InstancedStaticMeshComponents);
	for (UInstancedStaticMeshComponent* InstancedStaticMeshComponent :
	     InstancedStaticMeshComponents) PrimitiveComponents.Add(InstancedStaticMeshComponent);

	// If merged mesh already exists, re-register to prevent memory corruption
	if (FindObject<UObject>(nullptr, *PackageName)) FGlobalComponentReregisterContext GlobalReregister;

	if (PrimitiveComponents.Num() > 0)
	{
		TArray<UObject*> AssetsToSync;
		FVector MergedActorLocation;
		MeshUtilities.MergeComponentsToStaticMesh(PrimitiveComponents, World, MeshMergingSettings, nullptr, nullptr,
		                                          PackageName, AssetsToSync, MergedActorLocation, ScreenAreaSize, true);

		// Update Asset in Level
		if (AssetsToSync.Num())
		{
			UStaticMesh* GeneratedMesh = Cast<UStaticMesh>(AssetsToSync[0]);
			if (GeneratedMesh)
			{
				UBodySetup* BodySetup = GeneratedMesh->GetBodySetup();
				if (BodySetup) BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
			}
			FAssetRegistryModule& AssetRegistry = FModuleManager::Get().LoadModuleChecked
				<FAssetRegistryModule>("AssetRegistry");
			AssetRegistry.AssetCreated(AssetsToSync[0]);
			GEditor->BroadcastObjectReimported(AssetsToSync[0]);
			return AssetsToSync[0];
		}
		else return nullptr;
	}
#endif
	return nullptr;
}

// Used to prevent movement of structures and spline points after converting to static mesh
void UUBSplineStructureBPFL::LockActor(AActor* ActorToLock, const bool bLockStatus)
{
#if WITH_EDITOR
	ActorToLock->SetLockLocation(bLockStatus);
#endif
}

// Allows construction scripts to be re-run after Call-In-Editor functions
// *** USE CAREFULLY *** as can cause crashes due to infinite loops
void UUBSplineStructureBPFL::RerunConstructionScript(AActor* ActorToConstruct)
{
#if WITH_EDITOR
	ActorToConstruct->RerunConstructionScripts();
#endif
}

// Required so that landscape deforming structures are placed at the correct height on the landscape
float UUBSplineStructureBPFL::GetLandscapeHeightScale(ALandscape* Landscape)
{
	const ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	return (LandscapeInfo->DrawScale.Z);
}
