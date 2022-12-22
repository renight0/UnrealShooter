// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(_shooterCharacter == nullptr)
	{
		_shooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (_shooterCharacter)
	{
		// Get the lateral speed of the character from velocity
		FVector velocity{_shooterCharacter->GetVelocity()};
		velocity.Z = 0;
		_speed = velocity.Size(); // _speed is now the magnitude of the velocity vector while ignoring any z-axis movement since we zeroed the the z-axis in the line before this one.

		// Is character in air?
		_bIsInAir = _shooterCharacter->GetCharacterMovement()->IsFalling();

		// Is the character accelerating?
		if (_shooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			_bIsAccelerating = true; // this is whether or not the character is moving. Which means that even at constant velocity this variable is true.
		}
		else { _bIsAccelerating = false; }

		FRotator aimRotation = _shooterCharacter->GetBaseAimRotation();
		FRotator movementRotation = UKismetMathLibrary::MakeRotFromX(_shooterCharacter->GetVelocity());
		
		// Getting the angle between cross-hair direction and character movement direction 
		_movementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(movementRotation, aimRotation).Yaw;

		/* _movementOffsetYaw uses the velocity, and when the character stops running, velocity is zero. So we must store the value of
		 * _movementOffsetYaw from the previous frame into _lastMovementeOffsetYaw and use that in the blendspace Input (inside the animation blueprint)
		 * when they stop running*/
		
		if (_shooterCharacter->GetVelocity().Size() > 0.f)
		{
			_lastMovementOffsetYaw = _movementOffsetYaw;

		}

		//code for seeing Offset Yaw On Screen
		if (_bSeeOffsetYallOnScreen == true)
		{
			FString rotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), aimRotation.Yaw);
			FString movementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), movementRotation.Yaw);

			FString offsetMessage = FString::Printf(TEXT("Movement Rotation: %f"), _movementOffsetYaw);

			if (GEngine != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, rotationMessage);
				GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, movementRotationMessage);
				GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, offsetMessage);
			}
		}
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	// Trying to get the object APawn that has this script attached (in blueprint mostly) and cast its AShooterCharacter class. Remember ACharacter derives from APawn.*/
	_shooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	//Super::NativeInitializeAnimation();
	_bSeeOffsetYallOnScreen = false;
}
