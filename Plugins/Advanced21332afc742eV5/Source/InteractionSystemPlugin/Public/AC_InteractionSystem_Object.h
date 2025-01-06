// Copyright 2023, zblogin, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IS_Data.h"
#include "Blueprint/UserWidget.h"
#include "AC_InteractionSystem_Object.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class INTERACTIONSYSTEMPLUGIN_API UAC_InteractionSystem_Object : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAC_InteractionSystem_Object();

public:
	
	UPROPERTY(EditAnywhere, Category = "Detail")
	E_InteractionType InteractionType;

	//When you launch the enhanced input plugin, you can set it to the name EnhancedInput. It can also be set to the name in ProjectSettings->Input->ActionMappings.
	//如果你启用了增强输入插件，那么可以将这个参数设置为增强输入的命名。如果没有启用插件，可以设置为项目设置中的输入命名。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	FString InputActionName;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	E_InteractionInputType InputActionType;

	//Widget needed for interaction. The widget must add the BPI_InteractSystem_Widget interface.
	//交互所需的小部件。小部件必须添加BPI_InteractSystem_Widget接口。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	TSubclassOf<UUserWidget> InteractionTipWidgetClass;

	//At the beginning of the interaction, the player plays a montage of animations.
	//开始交互时，玩家蒙太奇动画类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	FString InteractMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	bool bFinishOnSuccess;

	//Interactive keystroke duration.This parameter is valid only when InputActionType is set to Press - Keep.
	//持续按键时间，InputActionType为持续按键时才需要设置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Key", meta = (EditCondition = "InputActionType == E_InteractionInputType::KeyPress_Keep", EditConditionHides))
	float KeepTime;

	//Number of interactive consecutive keystrokes. This parameter is valid only when InputActionType is set to Press-Multi.
	//连续按键次数，InputActionType为多次按键时才需要设置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Key", meta = (EditCondition = "InputActionType == E_InteractionInputType::KeyPress_Multi", EditConditionHides))
	int PressKeyAmount;

	//The detection time of each keystroke for consecutive keystrokes. Within this time, the specified key must be pressed, otherwise the interaction fails. This parameter is valid only when InputActionType is set to Press-Multi.
	//连续按键检测时间间隔，InputActionType为多次按键时才需要设置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Key", meta = (EditCondition = "InputActionType == E_InteractionInputType::KeyPress_Multi", EditConditionHides))
	float PressCheckTime;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction System|Object")
	bool GetParameterValueByKey(const TArray<FS_InteractionParameter>& Parameters, const FString Key, FString& OutValue);
};
