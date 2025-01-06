// Copyright 2023, zblogin, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IS_Data.generated.h"

UENUM(BlueprintType)
enum class E_InteractionInputType : uint8
{
	KeyPress_Once,
	KeyPress_Keep,
	KeyPress_Multi
};

UENUM(BlueprintType)
enum class E_InteractionDetectionMode : uint8
{
	LineTrace,
	CollisionOverlap,
	BothAndLineTraceFirst,
	BothAndCollisionOverlapFirst
};


UENUM(BlueprintType)
enum class E_InteractionState : uint8
{
	Pre,
	Start,
	Fail,
	Succeed,
	Finish
};

UENUM(BlueprintType)
enum class E_InteractionType : uint8
{
	ByLineTrace,
	ByCollision,
	Both
};

UENUM(BlueprintType)
enum class E_TrackingType : uint8
{
	Always,
	ClipToBounds
};



USTRUCT(BlueprintType)
struct FS_InteractionParameter
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		FString Key;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		FString Value;
};