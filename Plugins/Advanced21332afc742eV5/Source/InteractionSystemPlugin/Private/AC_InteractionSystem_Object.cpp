// Copyright 2023, zblogin, All Rights Reserved.


#include "AC_InteractionSystem_Object.h"

// Sets default values for this component's properties
UAC_InteractionSystem_Object::UAC_InteractionSystem_Object()
{

}

bool UAC_InteractionSystem_Object::GetParameterValueByKey(const TArray<FS_InteractionParameter>& Parameters, const FString Key, FString& OutValue)
{
	for (auto& param : Parameters)
	{
		if (param.Key == Key)
		{
			OutValue = param.Value;
			return true;
		}
	}
	return false;
}


