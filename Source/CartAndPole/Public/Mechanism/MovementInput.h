#pragma once

#include "Containers/Array.h"
#include "MovementInput.generated.h"

USTRUCT(BlueprintType)
struct FMovementInput
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite)
    float CartPosition;

    UPROPERTY(BlueprintReadWrite)
    float CartVelocity;

    UPROPERTY(BlueprintReadWrite)
    float PoleAngle;

    UPROPERTY(BlueprintReadWrite)
    float PoleAngularVelocity;

    FORCEINLINE void ResetToZero()
    {
        CartPosition = CartVelocity = PoleAngle = PoleAngularVelocity = 0;
    }

    FORCEINLINE TArray<float> ToArray() const
    {
        return { CartPosition, CartVelocity, PoleAngle, PoleAngularVelocity };
    }
};
