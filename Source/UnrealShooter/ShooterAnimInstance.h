// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UNREALSHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	//This function is like "Begin Play" for the UShooterAnimInstance class.
	virtual void NativeInitializeAnimation() override;


private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess));
	class AShooterCharacter* _shooterCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess));
	float _speed;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess));
	// Whether or not the character is in the air.
	bool _bIsInAir;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess));
	//For this specific code, this is whether or not the character is moving. Which means that even at
	//constant velocity this variable is true.
	bool _bIsAccelerating;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess));
	/* Offset yaw used for strafing. */
	float _movementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess));
	/* Offset yaw the frame before the character stops the movement. Used for strafing. */
	float _lastMovementOffsetYaw;

	// See Offset Yaw On Screen?
	bool _bSeeOffsetYallOnScreen;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"));
	// Is characterAiming?
	bool _bAiming;
	
};
