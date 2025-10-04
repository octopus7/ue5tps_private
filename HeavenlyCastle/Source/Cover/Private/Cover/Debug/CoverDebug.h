#pragma once

#include "CoreMinimal.h"

class UCoverControllerComponent;

extern COVER_API TAutoConsoleVariable<int32> CVarCoverDebug;

class COVER_API FCoverDebug
{
public:
    static void Draw(const UCoverControllerComponent& Controller);
};
