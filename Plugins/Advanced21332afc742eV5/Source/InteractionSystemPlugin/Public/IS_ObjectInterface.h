// Copyright 2023, zblogin, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IS_Data.h"
#include "IS_ObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIS_ObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INTERACTIONSYSTEMPLUGIN_API IIS_ObjectInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Object")
		bool CanInteract_BPI(const APawn* Interactor);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Object")
		void OnInteractionStateChanged_BPI(const E_InteractionState State, const APawn* Interactor);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Object")
		void OnKeepTimeChanged_BPI(const APawn* Interactor, const float Percent);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Object")
		void OnMultiInputPressed_BPI(const APawn* Interactor, const int CurrentCount, const int MaxCount);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Object")
		bool OpenCustomDepth_BPI(const bool Open);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Object")
		void CallFunctionByParameters_BPI(const TArray<FS_InteractionParameter>& Parameters);
};
