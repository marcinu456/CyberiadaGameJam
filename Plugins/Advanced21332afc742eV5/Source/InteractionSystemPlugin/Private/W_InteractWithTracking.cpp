// Copyright 2023, zblogin, All Rights Reserved.


#include "W_InteractWithTracking.h"
#include "Components/PanelWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UW_InteractWithTracking::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	RelativePosition = UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / 2.0f;
}

void UW_InteractWithTracking::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if(TargetActorRef != nullptr && GetRootRef() != nullptr)
	{
		CalculatePosition();
		CalculateAngle();
	}
}

UPanelWidget* UW_InteractWithTracking::GetRootRef_Implementation() const
{
	UPanelWidget* RootRef = nullptr;
	return RootRef;
}

void UW_InteractWithTracking::SetPrimitivePosition()
{
	FVector TargetLocation = TargetActorRef->GetActorLocation() + TargetOffset;

	FVector2D ScreenPosition;
	bool Result = GetOwningPlayer()->ProjectWorldLocationToScreen(TargetLocation, ScreenPosition, true);
	if(Result)
		PrimitivePosition = ScreenPosition;
	else
	{
		APlayerCameraManager* CameraManager = GetOwningPlayer()->PlayerCameraManager;
		FVector CameraLocation = CameraManager->GetCameraLocation();
		FVector CameraToTarget = TargetLocation - CameraLocation;

		FVector ProjectRight = CameraToTarget.ProjectOnTo(CameraManager->GetActorRightVector());
		FVector ProjectUp = CameraToTarget.ProjectOnTo(CameraManager->GetActorUpVector());

		float Loc_X = ProjectRight.Length();
		float Loc_Y = ProjectUp.Length();
				
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(CameraManager->GetCameraRotation(), UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation));
		bool bUp = DeltaRot.Pitch >= 0.0f;
		bool bRight = DeltaRot.Yaw <= 0.0f;

		if(bUp && bRight)
		{
			Loc_Y *= -1.0f;
		}
		else if(bUp && !bRight)
		{
			Loc_X *= -1.0f;
			Loc_Y *= -1.0f;
		}
		else if (!bUp && !bRight)
		{
			Loc_X *= -1.0f;
		}

		PrimitivePosition = RelativePosition + (FVector2D(Loc_X, Loc_Y).Normalize() * 10000.0f);
	}
}

void UW_InteractWithTracking::CalculatePosition()
{
	switch (ShowType)
	{
	case E_TrackingType::Always:
	{
		CalculatePosition_Always();
		break;
	}
	case E_TrackingType::ClipToBounds:
	{
		CalculatePosition_ClipToBounds();
		break;
	}
	}
}

void UW_InteractWithTracking::CalculatePosition_Always()
{
	SetPrimitivePosition();

	FVector2D ReturnValue(0.0f, 0.0f);
	bool Result = ClipPosition(PrimitivePosition, RelativePosition, RelativePosition*2.0f, ReturnValue);

	ClippingPosition = (Result?ReturnValue:PrimitivePosition) / UWidgetLayoutLibrary::GetViewportScale(GetWorld());

	FVector2D InPosition = bUseWidgetOffset?GetClampPosition():ClippingPosition;
	UWidgetLayoutLibrary::SlotAsCanvasSlot(GetRootRef())->SetPosition(InPosition);
}

void UW_InteractWithTracking::CalculatePosition_ClipToBounds()
{
	SetPrimitivePosition();

	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());

	if(PrimitivePosition.X >= 0.0f && PrimitivePosition.X <= ViewportSize.X && PrimitivePosition.Y >= 0.0f && PrimitivePosition.Y <= ViewportSize.Y)
	{
		FVector2D InPosition = PrimitivePosition / UWidgetLayoutLibrary::GetViewportScale(GetWorld());
		UWidgetLayoutLibrary::SlotAsCanvasSlot(GetRootRef())->SetPosition(InPosition);
		GetRootRef()->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
		GetRootRef()->SetVisibility(ESlateVisibility::Collapsed);
}

bool UW_InteractWithTracking::ClipPosition(const FVector2D Target, const FVector2D Relative, const FVector2D WindowsSize, FVector2D& ReturnValue)
{
	ReturnValue = FVector2D();

	float x1 = Target.X;
	float y1 = Target.Y;

	int Area1 = 0;
	int Area2 = 0;

	if(x1 <= 0.0f)
	{
		Area1 |= 4; 
	}
	else if(x1 >= WindowsSize.X)
	{
		Area1 |= 8;
	}

	if(y1 <= 0.0f)
	{
		Area1 |= 1;	
	}
	else if(y1 >= WindowsSize.Y)
	{
		Area1 |= 2;
	}

	if((Area1 == 0 && Area2 == 0) || ((Area1 & Area2) != 0))
		return false;

	switch(Area1)
	{
	case 4:
	{
		x1 = 0.0f;
		y1 = Relative.Y + Relative.X * (Target.Y - Relative.Y) / (Relative.X - Target.X);
		break;
	}
	case 8:
	{
		x1 = WindowsSize.X;
		y1 = Target.Y - (Target.X - WindowsSize.X) * (Target.Y - Relative.Y) / (Target.X - Relative.X);
		break;
	}
	case 1:
	{
		y1 = 0.0f;
		x1 = Target.X - Target.Y * (Relative.X - Target.X) / (Relative.Y - Target.Y);
		break;
	}
	case 2:
	{
		y1 = WindowsSize.Y;
		x1 = Relative.X - (WindowsSize.Y - Relative.Y) * (Relative.X - Target.X) / (Target.Y - Relative.Y);
		break;
	}
	case 5 :
	{
		x1 = 0.0f;
		y1 = Relative.Y + Relative.X * (Target.Y - Relative.Y) / (Relative.X - Target.X);
		if(y1 < 0.0f)
		{
			y1 = 0.0f;
			x1 = Target.X - Target.Y * (Relative.X - Target.X) / (Relative.Y - Target.Y);
		}
		break;
	}
	case 6:
	{
		x1 = 0.0f;
		y1 = Relative.Y + Relative.X * (Target.Y - Relative.Y) / (Relative.X - Target.X);
		if(y1 > WindowsSize.Y)
		{
			y1 = WindowsSize.Y;
			x1 = Relative.X - (WindowsSize.Y - Relative.Y) * (Relative.X - Target.X) / (Target.Y - Relative.Y);
		}
		break;
	}
	case 9:
	{
		x1 = WindowsSize.X;
		y1 = Target.Y - (Target.X - WindowsSize.X) * (Target.Y - Relative.Y) / (Target.X - Relative.X);
		if(y1 < 0.0f)
		{
			y1 = 0.0f;
			x1 = Target.X - Target.Y * (Relative.X - Target.X) / (Relative.Y - Target.Y);
		}
		break;
	}
	case 10:
	{
		x1 = WindowsSize.X;
		y1 = Target.Y - (Target.X - WindowsSize.X) * (Target.Y - Relative.Y) / (Target.X - Relative.X);
		if(y1 > WindowsSize.Y)
		{
			y1 = WindowsSize.Y;
			x1 = Relative.X - (WindowsSize.Y - Relative.Y) * (Relative.X - Target.X) / (Target.Y - Relative.Y);
		}
		break;
	}
	}

	if(x1 < 0.0f || x1 > WindowsSize.X || y1 < 0.0f || y1 > WindowsSize.Y)
		return false;

	ReturnValue.X = x1;
	ReturnValue.Y = y1;

	return true;
}

FVector2D UW_InteractWithTracking::GetClampPosition()
{
	FVector2D ClampedPosition;

	FVector2D Loc_PracticalViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	
	XClamp.X = WidgetOffset.X;
	XClamp.Y = Loc_PracticalViewportSize.X - WidgetOffset.X;
	YClamp.X = WidgetOffset.Y;
	YClamp.Y = Loc_PracticalViewportSize.Y - WidgetOffset.Y;

	if(ClippingPosition.X >= XClamp.X && ClippingPosition.X <= XClamp.Y)
		ClampedPosition.X = ClippingPosition.X;
	else
		ClampedPosition.X = UKismetMathLibrary::FClamp(ClippingPosition.X, XClamp.X, XClamp.Y);

	if (ClippingPosition.Y >= YClamp.X && ClippingPosition.Y <= YClamp.Y)
		ClampedPosition.Y = ClippingPosition.Y;
	else
		ClampedPosition.Y = UKismetMathLibrary::FClamp(ClippingPosition.Y, YClamp.X, YClamp.Y);

	return ClampedPosition;
}

void UW_InteractWithTracking::CalculateAngle()
{
	if(!bUseRotation)
		return;

	FVector2D Loc_Direction = UWidgetLayoutLibrary::SlotAsCanvasSlot(GetRootRef())->GetPosition() - (RelativePosition / UWidgetLayoutLibrary::GetViewportScale(GetWorld()));

	FVector2D Loc_Temp = Loc_Direction.X >= 0 ? FVector2D(0.0f, -1.0f) : FVector2D(0.0f, 1.0f);
	float InAngle = Loc_Direction.X >= 0 ? 0.0f : 180.0f;
	InAngle += UKismetMathLibrary::DegAcos(UKismetMathLibrary::DotProduct2D(Loc_Direction.GetSafeNormal(), Loc_Temp));

	GetRootRef()->SetRenderTransformAngle(InAngle);
}
