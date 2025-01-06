// Copyright 2023, zblogin, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IS_Data.h"
#include "IS_WidgetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIS_WidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INTERACTIONSYSTEMPLUGIN_API IIS_WidgetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Widget")
		bool SetInteractStateForWidget_BPI(const AActor* InteractiveActorRef, const E_InteractionState State);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Widget")
		bool ChangeKeepTimePercent_BPI(const AActor* InteractiveActorRef, const float Percent);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction System Interface|Widget")
		bool ChangeMultiKeyPressCount_BPI(const AActor* InteractiveActorRef, const int CurrentCount, const int MaxCount);
};
