// Copyright 2023, zblogin, All Rights Reserved.


#include "AC_InteractionSystem_Player.h"
#include "Components/PrimitiveComponent.h"
#include "Blueprint/UserWidget.h"
#include "AC_InteractionSystem_Object.h"
#include "IS_ObjectInterface.h"
#include "IS_WidgetInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/HitResult.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"




UAC_InteractionSystem_Player::UAC_InteractionSystem_Player()
{
	TraceDistance = 1000.0f;
	CollisionComponentTag = TEXT("InteractCollision");
	FieldOfView = 180.0f;
	bShowAnActor = true;

	PrimaryComponentTick.bCanEverTick = true;
}


void UAC_InteractionSystem_Player::TryInteractWithInput(const FString InputActionName, const bool bPressed)
{
	if(InteractiveActorRef == nullptr)
		return;

	UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(InteractiveActorRef->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
	if (comp != nullptr)
	{
		if(comp->InputActionName == InputActionName && !bInteractSucceed)
		{
			CheckInputAction(bPressed);
		}
	}
}

void UAC_InteractionSystem_Player::ChangeDetectionState(bool DetectingAnActor)
{
	bShowAnActor = DetectingAnActor;
	SetInteractiveActor(nullptr);
}

TArray<AActor*> UAC_InteractionSystem_Player::GetOverlappingActors()
{
	return CollisionOverlappingActors;
}

void UAC_InteractionSystem_Player::CheckInputAction(const bool bPressed)
{
	if(InteractiveActorRef == nullptr)
		return;

	UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(InteractiveActorRef->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
	if (comp != nullptr)
	{
		switch (comp->InputActionType)
		{
		case E_InteractionInputType::KeyPress_Once:
		{
			if(bPressed)
			{
				CheckPlayMontage();
				ChangeInteractionState(E_InteractionState::Start);
				ChangeInteractionState(E_InteractionState::Succeed);
			}
			break;
		}
		case E_InteractionInputType::KeyPress_Keep:
		{
			if (bPressed)
			{
				if(!TimerHandle_InputKeepCheck.IsValid())
				{
					CheckPlayMontage();
					ChangeInteractionState(E_InteractionState::Start);
					CurrentKeepTime = 0.0f;
					const float LoopTime = (comp->KeepTime > 0.0f) ? 0.02f : 10.0f;
					GetWorld()->GetTimerManager().SetTimer(TimerHandle_InputKeepCheck, this, &UAC_InteractionSystem_Player::Bind_CheckInputKeep, LoopTime, true);

					//PrimaryComponentTick.SetTickFunctionEnable(false);
				}
			}
			else
			{
				if (TimerHandle_InputKeepCheck.IsValid())
				{
					if(comp->KeepTime > 0.0f)
					{
						if(CurrentKeepTime < comp->KeepTime)
						{
							GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InputKeepCheck);
							ChangeInteractionState(E_InteractionState::Fail);
						}
					}
					else
					{
						GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InputKeepCheck);
						ChangeInteractionState(E_InteractionState::Succeed);
					}
				}
				//PrimaryComponentTick.SetTickFunctionEnable(true);
			}
			break;
		}
		case E_InteractionInputType::KeyPress_Multi:
		{
			if (bPressed)
			{
				if (TimerHandle_InputMultiCheck.IsValid())
				{
					CurrentInputMultiPressedAmount++;
					MultiPressedAmountChanged(CurrentInputMultiPressedAmount, comp->PressKeyAmount);
					if(CurrentInputMultiPressedAmount >= comp->PressKeyAmount)
					{
						GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InputMultiCheck);
						ChangeInteractionState(E_InteractionState::Succeed);
					}
					else
					{
						CurrentPressCheckTime = comp->PressCheckTime;
					}
				}
				else
				{
					CurrentInputMultiPressedAmount = 1;
					CheckPlayMontage();
					CurrentPressCheckTime = comp->PressCheckTime;
					ChangeInteractionState(E_InteractionState::Start);
					GetWorld()->GetTimerManager().SetTimer(TimerHandle_InputMultiCheck, this, &UAC_InteractionSystem_Player::Bind_CheckInputMultiPress, 0.02f, true);
					MultiPressedAmountChanged(CurrentInputMultiPressedAmount, comp->PressKeyAmount);
				}
			}

			break;
		}
		}
	}
}

void UAC_InteractionSystem_Player::CheckPlayMontage()
{
	if (InteractiveActorRef == nullptr)
		return;
	UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(InteractiveActorRef->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
	if (comp != nullptr && !comp->InteractMontage.IsEmpty())
	{
		SR_PlayMontage(comp->InteractMontage);
	}
}

void UAC_InteractionSystem_Player::SR_PlayMontage_Implementation(const FString& MontageName)
{
	MCR_PlayInteractionMontage(MontageName);
}

void UAC_InteractionSystem_Player::MCR_PlayInteractionMontage_Implementation(const FString& MontageName)
{
	UAnimMontage** MapValue = InteractMontage.Find(MontageName);
	if(MapValue == nullptr)
		return;

	UAnimMontage* MontageRef = *MapValue;
	if(MontageRef != nullptr)
	{
		USkeletalMeshComponent* SK_Comp = Cast<USkeletalMeshComponent>(PlayerPawnRef->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
		if(SK_Comp != nullptr)
		{
			SK_Comp->GetAnimInstance()->Montage_Play(MontageRef);
		}
	}

}

void UAC_InteractionSystem_Player::StopMontage()
{
	if(InteractiveActorRef == nullptr)
		return;
	UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(InteractiveActorRef->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
	if (comp != nullptr && !comp->InteractMontage.IsEmpty())
	{
		SR_StopMontage(comp->InteractMontage);
	}
}

void UAC_InteractionSystem_Player::SR_StopMontage_Implementation(const FString& MontageName)
{
	MCR_StopInteractionMontage(MontageName);
}

void UAC_InteractionSystem_Player::MCR_StopInteractionMontage_Implementation(const FString& MontageName)
{
	UAnimMontage** MapValue = InteractMontage.Find(MontageName);
	if (MapValue == nullptr)
		return;

	UAnimMontage* MontageRef = *MapValue;
	if (MontageRef != nullptr)
	{
		USkeletalMeshComponent* SK_Comp = Cast<USkeletalMeshComponent>(PlayerPawnRef->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
		if (SK_Comp != nullptr)
		{
			UAnimInstance* AnimInstance = SK_Comp->GetAnimInstance();
			if(AnimInstance == nullptr)
				return;
			if(AnimInstance->Montage_IsActive(MontageRef))
			{
				AnimInstance->Montage_Stop(MontageRef->GetDefaultBlendOutTime(), MontageRef);
			}
		}
	}
}



void UAC_InteractionSystem_Player::Bind_CheckInputKeep()
{
	if(InteractiveActorRef == nullptr)
	{
		CurrentKeepTime = 0.0f;
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InputKeepCheck);
		//PrimaryComponentTick.SetTickFunctionEnable(true);
		return;
	}

	UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(InteractiveActorRef->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
	if (comp != nullptr)
	{
		if(comp->KeepTime > 0.0f)
		{
			CurrentKeepTime += 0.02;
			DurationPercentChanged(CurrentKeepTime / comp->KeepTime);

			if(CurrentKeepTime >= comp->KeepTime)
			{
				CurrentKeepTime = 0.0f;
				GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InputKeepCheck);
				ChangeInteractionState(E_InteractionState::Succeed);

				//PrimaryComponentTick.SetTickFunctionEnable(true);
			}
		}
	}
}

void UAC_InteractionSystem_Player::Bind_CheckInputMultiPress()
{
	if (InteractiveActorRef == nullptr)
	{
		CurrentPressCheckTime = 0.0f;
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InputMultiCheck);
		CurrentInputMultiPressedAmount = 0;

		//PrimaryComponentTick.SetTickFunctionEnable(true);
		return;
	}

	UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(InteractiveActorRef->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
	if (comp != nullptr)
	{
		CurrentPressCheckTime -= 0.02f;
		if(CurrentPressCheckTime <= 0.0f)
		{
			CurrentPressCheckTime = comp->PressCheckTime;
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InputMultiCheck);
			ChangeInteractionState(E_InteractionState::Fail);

			//PrimaryComponentTick.SetTickFunctionEnable(true);
		}
	}
}

void UAC_InteractionSystem_Player::DurationPercentChanged(const float Percent)
{
	if(InteractiveActorRef == nullptr)
		return;
	if(InteractiveActorRef->GetIsReplicated())
		SR_DurationPercentChanged(InteractiveActorRef, PlayerPawnRef, Percent);
	else
		IIS_ObjectInterface::Execute_OnKeepTimeChanged_BPI(InteractiveActorRef, PlayerPawnRef, Percent);

	if(InteractiveTipWidgetRef != nullptr && InteractiveTipWidgetRef->Implements<UIS_WidgetInterface>())
	{
		IIS_WidgetInterface::Execute_ChangeKeepTimePercent_BPI(InteractiveTipWidgetRef, InteractiveActorRef, Percent);
	}
}

void UAC_InteractionSystem_Player::SR_DurationPercentChanged_Implementation(AActor* InteractiveActor, const APawn* Interactor, const float Percent)
{
	if(InteractiveActor != nullptr && InteractiveActor->Implements<UIS_ObjectInterface>())
		IIS_ObjectInterface::Execute_OnKeepTimeChanged_BPI(InteractiveActor, Interactor, Percent);
}

void UAC_InteractionSystem_Player::MultiPressedAmountChanged(const int CurrentAmount, const int MaxAmount)
{
	if (InteractiveActorRef == nullptr)
		return;
	if (InteractiveActorRef->GetIsReplicated())
		SR_MultiPressedAmountChanged(InteractiveActorRef, PlayerPawnRef, CurrentAmount, MaxAmount);
	else
		IIS_ObjectInterface::Execute_OnMultiInputPressed_BPI(InteractiveActorRef, PlayerPawnRef, CurrentAmount, MaxAmount);

	if (InteractiveTipWidgetRef != nullptr && InteractiveTipWidgetRef->Implements<UIS_WidgetInterface>())
	{
		IIS_WidgetInterface::Execute_ChangeMultiKeyPressCount_BPI(InteractiveTipWidgetRef, InteractiveActorRef, CurrentAmount, MaxAmount);
	}
}

void UAC_InteractionSystem_Player::SR_MultiPressedAmountChanged_Implementation(AActor* InteractiveActor, const APawn* Interactor, const int CurrentAmount, const int MaxAmount)
{
	if (InteractiveActor != nullptr && InteractiveActor->Implements<UIS_ObjectInterface>())
		IIS_ObjectInterface::Execute_OnMultiInputPressed_BPI(InteractiveActor, Interactor, CurrentAmount, MaxAmount);
}

void UAC_InteractionSystem_Player::BeginPlay()
{
	Super::BeginPlay();

	ComponentInit();
}


// Called every frame
void UAC_InteractionSystem_Player::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DetectionUpdate();
	PrintDebugMessage();
}

void UAC_InteractionSystem_Player::ComponentInit()
{
	PrimaryComponentTick.SetTickFunctionEnable(false);

	FTimerHandle TimerHandle_Init;

	PlayerPawnRef = Cast<APawn>(this->GetOwner());
	if (PlayerPawnRef == nullptr)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Init, this, &UAC_InteractionSystem_Player::ComponentInit, 0.2f);
		return;
	}

	PlayerControllerRef = Cast<APlayerController>(PlayerPawnRef->GetController());
	if (PlayerControllerRef == nullptr)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Init, this, &UAC_InteractionSystem_Player::ComponentInit, 0.2f);
		return;
	}

	if (PlayerControllerRef->IsLocalController())
	{
		if(DetectionMode != E_InteractionDetectionMode::LineTrace)
		{
			TArray<UActorComponent*> CompArray = PlayerPawnRef->GetComponentsByTag(UPrimitiveComponent::StaticClass(), CollisionComponentTag);
			if (CompArray[0] != nullptr)
				CollisionComponentRef = Cast<UPrimitiveComponent>(CompArray[0]);
		}

		PrimaryComponentTick.SetTickFunctionEnable(true);
	}
}

void UAC_InteractionSystem_Player::UpdatePreInteractiveActor(AActor* ActorRef)
{
	if(ActorRef == nullptr)
	{
		SetInteractiveActor(nullptr);
	}
	else if(ActorRef != InteractiveActorRef && IIS_ObjectInterface::Execute_CanInteract_BPI(ActorRef, PlayerPawnRef))
	{
		SetInteractiveActor(ActorRef);
	}
}

void UAC_InteractionSystem_Player::SetInteractiveActor(AActor* ActorRef)
{
	if (InteractiveActorRef != nullptr)
	{
		OpenPreInteractiveActorCustomDepth(false);
	}
	if(ActorRef == nullptr)
	{
		InteractiveActorRef = nullptr;
		RemoveInteractiveTipWidget();
	}
	else
	{
		InteractiveActorRef = ActorRef;
		OpenPreInteractiveActorCustomDepth(true);
		CreateInteractiveTipWidget();
	}
}

void UAC_InteractionSystem_Player::OpenPreInteractiveActorCustomDepth(bool bOpen)
{
	if (InteractiveActorRef != nullptr && InteractiveActorRef->Implements<UIS_ObjectInterface>())
	{
		IIS_ObjectInterface::Execute_OpenCustomDepth_BPI(InteractiveActorRef, bOpen);
	}
}

void UAC_InteractionSystem_Player::CreateInteractiveTipWidget()
{
	RemoveInteractiveTipWidget();
	if (InteractiveActorRef != nullptr)
	{
		UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(InteractiveActorRef->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
		if (comp != nullptr)
		{
			if(IsValid(comp->InteractionTipWidgetClass))
			{
				InteractiveTipWidgetRef = CreateWidget<UUserWidget>(PlayerControllerRef, comp->InteractionTipWidgetClass);
				InteractiveTipWidgetRef->AddToViewport(InteractiveWidgetZOrder);
				ChangeInteractionState(E_InteractionState::Pre);
			}
		}
	}
	
}

void UAC_InteractionSystem_Player::RemoveInteractiveTipWidget()
{
	if(bInteractSucceed)
	{
		ChangeInteractionState(E_InteractionState::Finish);
		return;
	}
	if(InteractiveTipWidgetRef != nullptr)
	{
		InteractiveTipWidgetRef->RemoveFromParent();
		InteractiveTipWidgetRef = nullptr;
	}
}

void UAC_InteractionSystem_Player::ChangeInteractionState(E_InteractionState State)
{
	if(InteractiveActorRef == nullptr || InteractiveTipWidgetRef == nullptr)
	{
		SetInteractiveActor(nullptr);
		PrimaryComponentTick.SetTickFunctionEnable(true);
		return;
	}
	if(!InteractiveActorRef->Implements<UIS_ObjectInterface>() || !InteractiveTipWidgetRef->Implements<UIS_WidgetInterface>())
		return;

	if(IIS_WidgetInterface::Execute_SetInteractStateForWidget_BPI(InteractiveTipWidgetRef, InteractiveActorRef, State))
	{
		if (InteractiveActorRef->GetIsReplicated())
			SR_ChangeInteractionState(InteractiveActorRef, State, PlayerPawnRef);
		else
			IIS_ObjectInterface::Execute_OnInteractionStateChanged_BPI(InteractiveActorRef, State, PlayerPawnRef);

		OnInteractionStateChanged.Broadcast(State, InteractiveActorRef);

		switch (State)
		{
		case E_InteractionState::Start:
		{
			PrimaryComponentTick.SetTickFunctionEnable(false);
			break;
		}
		case E_InteractionState::Fail:
		{
			bInteractSucceed = false;
			StopMontage();
			SetInteractiveActor(nullptr);
			PrimaryComponentTick.SetTickFunctionEnable(true);
			break;
		}
		case E_InteractionState::Succeed:
		{
			bInteractSucceed = true;
			UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(InteractiveActorRef->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
			if (comp != nullptr && comp->bFinishOnSuccess)
				ChangeInteractionState(E_InteractionState::Finish);
			break;
		}
		case E_InteractionState::Finish:
		{
			bInteractSucceed = false;
			SetInteractiveActor(nullptr);
			PrimaryComponentTick.SetTickFunctionEnable(true);
			break;
		}
		}
	}	
}

void UAC_InteractionSystem_Player::SR_ChangeInteractionState_Implementation(AActor* InteractiveActor, const E_InteractionState State, const APawn* Interactor)
{
	if (InteractiveActor != nullptr && InteractiveActor->Implements<UIS_ObjectInterface>())
		IIS_ObjectInterface::Execute_OnInteractionStateChanged_BPI(InteractiveActor, State, Interactor);
}

void UAC_InteractionSystem_Player::DetectionUpdate()
{
	if (!bShowAnActor)
	{
		GetCollisionOverlapResult();
		return;
	}

	AActor* ResultActorRef;
	switch (DetectionMode)
	{
	case E_InteractionDetectionMode::LineTrace:
	{	
		ResultActorRef = GetLineTraceResult();
		UpdatePreInteractiveActor(ResultActorRef);
		break;
	}
	case E_InteractionDetectionMode::CollisionOverlap:
	{
		ResultActorRef = GetCollisionOverlapResult();
		UpdatePreInteractiveActor(ResultActorRef);
		break;
	}
	case E_InteractionDetectionMode::BothAndLineTraceFirst:
	{
		ResultActorRef = GetLineTraceResult();
		if(IsValid(ResultActorRef))
		{
			UpdatePreInteractiveActor(ResultActorRef);
			break;
		}
		ResultActorRef = GetCollisionOverlapResult();
		UpdatePreInteractiveActor(ResultActorRef);
		break;
	}
	case E_InteractionDetectionMode::BothAndCollisionOverlapFirst:
	{
		ResultActorRef = GetCollisionOverlapResult();
		if(IsValid(ResultActorRef))
		{
			UpdatePreInteractiveActor(ResultActorRef);
			break;
		}
		ResultActorRef = GetLineTraceResult();
		UpdatePreInteractiveActor(ResultActorRef);
		break;
	}
	}
}

void UAC_InteractionSystem_Player::PrintDebugMessage()
{
	if(!bDebug)
		return;

	FString out_DebugStr("Interact System Debug\r\nDetection Mode:");

	FString DetectionModeStr = UEnum::GetValueAsString(DetectionMode);
	out_DebugStr.Append(DetectionModeStr);

	out_DebugStr.Append(TEXT("\r\nInteractResult:\r\n"));
	out_DebugStr.Append(IsValid(InteractiveActorRef) ? InteractiveActorRef->GetClass()->GetName() : TEXT("None"));

	check(GEngine != nullptr);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, out_DebugStr);
}

AActor* UAC_InteractionSystem_Player::GetLineTraceResult()
{
	if(PlayerControllerRef == nullptr)
		return nullptr;

	FHitResult LineTraceResult;
	FVector Start;
	FVector End;
	FCollisionQueryParams Params;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(PlayerPawnRef);
	Params.AddIgnoredActors(IgnoreActors);

	if(bTraceByMousePosition)
	{
		FVector WorldDirection;
		PlayerControllerRef->DeprojectMousePositionToWorld(Start, WorldDirection);
		End = Start + (WorldDirection*TraceDistance);
	}
	else
	{
		Start = PlayerControllerRef->PlayerCameraManager->GetCameraLocation();
		End = Start + (PlayerControllerRef->PlayerCameraManager->GetCameraRotation().Vector()*TraceDistance);
	}
	bool result = GetWorld()->LineTraceSingleByObjectType(LineTraceResult, Start, End, TraceObjectTypes, Params);

	if (!result && !LineTraceResult.bBlockingHit)
		return nullptr;
	
	AActor* HitActor = LineTraceResult.GetHitObjectHandle().FetchActor();
	if(CheckActorValid(HitActor))
	{
		UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(HitActor->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
		if(comp != nullptr && comp->InteractionType != E_InteractionType::ByCollision)
			return HitActor;
	}

	return nullptr;
}

AActor* UAC_InteractionSystem_Player::GetCollisionOverlapResult()
{
	if(CollisionComponentRef == nullptr)
		return nullptr;

	CollisionOverlappingActors.Empty();
	float CloserAngle = FieldOfView / 2;
	AActor* OutActorRef = nullptr;

	TArray<AActor*> OverlappingActors;
	CollisionComponentRef->GetOverlappingActors(OverlappingActors);

	for (auto element : OverlappingActors)
	{
		if(!CheckActorValid(element))
			continue;
		UAC_InteractionSystem_Object* comp = Cast<UAC_InteractionSystem_Object>(element->GetComponentByClass(UAC_InteractionSystem_Object::StaticClass()));
		if (comp == nullptr)
			continue;
		if (comp->InteractionType != E_InteractionType::ByLineTrace)
		{
			CollisionOverlappingActors.AddUnique(element);
			if(CheckCloserActor(CloserAngle, element))
			{
				OutActorRef = element;
			}
		}
	}

	return OutActorRef;
}

bool UAC_InteractionSystem_Player::CheckActorValid(AActor* ActorRef)
{
	if(ActorRef != nullptr && ActorRef->Implements<UIS_ObjectInterface>())
	{
		return true;
		//IIS_ObjectInterface::Execute_CanInteract_BPI(ActorRef, PlayerPawnRef);
	}
	
	return false;
}

bool UAC_InteractionSystem_Player::CheckCloserActor(float& CurrentCloserAngle, AActor* ActorRef)
{
	if(ActorRef == nullptr)
		return false;

	FVector RelativeVector = ActorRef->GetActorLocation() - PlayerPawnRef->GetActorLocation();
	RelativeVector = FVector(RelativeVector.X, RelativeVector.Y, 0.0f);
	FVector NormVec = RelativeVector.GetSafeNormal();

	float DotResult = NormVec.Dot(PlayerPawnRef->GetActorForwardVector());
	float angle = FMath::Abs(UKismetMathLibrary::DegAcos(DotResult));

	if(angle < CurrentCloserAngle)
	{
		CurrentCloserAngle = angle;
		return true;
	}
	return false;
}

void UAC_InteractionSystem_Player::CallInteractiveObjectFunction(AActor* InteractiveActor, const TArray<FS_InteractionParameter> Parameters)
{
	if (InteractiveActor == nullptr)
		return;
	if (InteractiveActor->GetIsReplicated())
		SR_CallInteractiveObjectFunction(InteractiveActor, Parameters);
	else
		CR_CallInteractiveObjectFunction(InteractiveActor, Parameters);
}

void UAC_InteractionSystem_Player::SR_CallInteractiveObjectFunction_Implementation(AActor* InteractiveActor, const TArray<FS_InteractionParameter>& Parameters)
{
	if (InteractiveActor != nullptr && InteractiveActor->Implements<UIS_ObjectInterface>())
		IIS_ObjectInterface::Execute_CallFunctionByParameters_BPI(InteractiveActor, Parameters);
}

void UAC_InteractionSystem_Player::CR_CallInteractiveObjectFunction_Implementation(AActor* InteractiveActor, const TArray<FS_InteractionParameter>& Parameters)
{
	if (InteractiveActor != nullptr && InteractiveActor->Implements<UIS_ObjectInterface>())
		IIS_ObjectInterface::Execute_CallFunctionByParameters_BPI(InteractiveActor, Parameters);
}
