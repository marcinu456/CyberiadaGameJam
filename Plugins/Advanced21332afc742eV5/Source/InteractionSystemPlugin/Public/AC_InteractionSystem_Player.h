// Copyright 2023, zblogin, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IS_Data.h"
#include "Animation/AnimMontage.h"
#include "AC_InteractionSystem_Player.generated.h"

class UPrimitiveComponent;
class UUserWidget;
class APawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionStateChanged, E_InteractionState, InteractionState, AActor*, InteractiveActor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class INTERACTIONSYSTEMPLUGIN_API UAC_InteractionSystem_Player : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAC_InteractionSystem_Player();

	UFUNCTION(BlueprintCallable, Category = "Interaction System|Player")
	void TryInteractWithInput(const FString InputActionName, const bool bPressed);

	UFUNCTION(BlueprintCallable, Category = "Interaction System|Player")
	void ChangeDetectionState(bool DetectingAnActor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction System|Player")
	TArray<AActor*> GetOverlappingActors();

	UFUNCTION(BlueprintCallable, Category = "Interaction System|Player")
	void ChangeInteractionState(E_InteractionState State);

	UFUNCTION(BlueprintCallable, Category = "Interaction System|Player")
	void CallInteractiveObjectFunction(AActor* InteractiveActor, const TArray<FS_InteractionParameter> Parameters);

	UPROPERTY(BlueprintAssignable)
	FOnInteractionStateChanged OnInteractionStateChanged;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void ComponentInit();

	void DetectionUpdate();
	void PrintDebugMessage();
	void UpdatePreInteractiveActor(AActor* ActorRef);
	void SetInteractiveActor(AActor* ActorRef);
	void OpenPreInteractiveActorCustomDepth(bool bOpen);
	void CreateInteractiveTipWidget();
	void RemoveInteractiveTipWidget();

	UFUNCTION(Server, Reliable)
	void SR_ChangeInteractionState(AActor* InteractiveActor, const E_InteractionState State, const APawn* Interactor);

	AActor* GetLineTraceResult();
	AActor* GetCollisionOverlapResult();

	bool CheckActorValid(AActor* ActorRef);
	bool CheckCloserActor(float& CurrentCloserAngle, AActor* ActorRef);

	void CheckInputAction(const bool bPressed);
	void Bind_CheckInputKeep();
	void Bind_CheckInputMultiPress();
	void DurationPercentChanged(const float Percent);
	
	UFUNCTION(Server, Reliable)
	void SR_DurationPercentChanged(AActor* InteractiveActor, const APawn* Interactor, const float Percent);
	
	void MultiPressedAmountChanged(const int CurrentAmount, const int MaxAmount);
	
	UFUNCTION(Server, Reliable)
	void SR_MultiPressedAmountChanged(AActor* InteractiveActor, const APawn* Interactor, const int CurrentAmount, const int MaxAmount);

	void CheckPlayMontage();
	
	UFUNCTION(Server, Reliable)
	void SR_PlayMontage(const FString& MontageName);
	
	UFUNCTION(NetMulticast, Reliable)
	void MCR_PlayInteractionMontage(const FString& MontageName);
	
	void StopMontage();
	UFUNCTION(Server, Reliable)
	void SR_StopMontage(const FString& MontageName);
	
	UFUNCTION(NetMulticast, Reliable)
	void MCR_StopInteractionMontage(const FString& MontageName);

	UFUNCTION(Server, Reliable)
	void SR_CallInteractiveObjectFunction(AActor* InteractiveActor, const TArray<FS_InteractionParameter>& Parameters);
	
	UFUNCTION(Client, Reliable)
	void CR_CallInteractiveObjectFunction(AActor* InteractiveActor, const TArray<FS_InteractionParameter>& Parameters);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	E_InteractionDetectionMode DetectionMode;

	UPROPERTY(EditAnywhere, Category = "Detail")
	bool bDebug;

	//A montage of animations played during interaction.This is a Map of data. The Key
	//corresponds to the InteractMontage in BPC_InteractSystem_Object.
	//交互时播放的蒙太奇动画，这是一个Map类型数据，Key对应BPC_InteractSystem_Object中的InteractMontage。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	TMap<FString, UAnimMontage*> InteractMontage;

	UPROPERTY(EditAnywhere, Category = "Detail")
	int InteractiveWidgetZOrder;

	//Whether to detect interactions by mapping mouse screen coordinates to world coordinates. 
	//If you only want to detect by the Collision component, ignore this option.
	//是否通过将鼠标屏幕坐标映射到世界坐标来检测交互。如果你只想通过Collision组件检测，忽略这个选项。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Trace", meta = (EditCondition = "DetectionMode != E_InteractionDetectionMode::CollisionOverlap", EditConditionHides))
	bool bTraceByMousePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Trace", meta = (EditCondition = "DetectionMode != E_InteractionDetectionMode::CollisionOverlap", EditConditionHides))
	float TraceDistance;
	
	//Type of trace detection. It is highly recommended to create a new type that represents only interactive objects.
	//射线检测类型。强烈建议新建一个类型，这个类型只代表可交互物体。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail|Trace", meta = (EditCondition = "DetectionMode != E_InteractionDetectionMode::CollisionOverlap", EditConditionHides))
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail|Player Collision", meta = (EditCondition = "DetectionMode != E_InteractionDetectionMode::LineTrace", EditConditionHides))
	FName CollisionComponentTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Player Collision", meta = (EditCondition = "DetectionMode != E_InteractionDetectionMode::LineTrace", EditConditionHides))
	float FieldOfView;

private:
	APawn* PlayerPawnRef;
	APlayerController* PlayerControllerRef;
	UPrimitiveComponent* CollisionComponentRef;
	TArray<AActor*> CollisionOverlappingActors;

	AActor* InteractiveActorRef;
	UUserWidget* InteractiveTipWidgetRef;

	bool bShowAnActor;
	bool bInteractSucceed;

	FTimerHandle TimerHandle_InputKeepCheck;
	float CurrentKeepTime;

	FTimerHandle TimerHandle_InputMultiCheck;
	int CurrentInputMultiPressedAmount;
	float CurrentPressCheckTime;
};
