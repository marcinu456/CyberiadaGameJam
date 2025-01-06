// Copyright 2023, zblogin, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IS_Data.h"
#include "W_InteractWithTracking.generated.h"

class UPanelWidget;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class INTERACTIONSYSTEMPLUGIN_API UW_InteractWithTracking : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail")
		AActor* TargetActorRef;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail")
		FVector TargetOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail")
		E_TrackingType ShowType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail")
		bool bUseWidgetOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail")
		FVector2D WidgetOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail")
		bool bUseRotation;

private:
	FVector2D PrimitivePosition;
	FVector2D XClamp;
	FVector2D YClamp;
	FVector2D RelativePosition;
	FVector2D ClippingPosition;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Base")
		UPanelWidget* GetRootRef() const;

private:
	void SetPrimitivePosition();

	void CalculatePosition();
	void CalculatePosition_Always();
	void CalculatePosition_ClipToBounds();
	bool ClipPosition(const FVector2D Target, const FVector2D Relative, const FVector2D WindowsSize, FVector2D& ReturnValue);
	FVector2D GetClampPosition();

	void CalculateAngle();
};
