// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


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
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	// Trying to get the object APawn that has this script attached (in blueprint mostly) and cast its AShooterCharacter class.
	// Remember ACharacter Deriver from APawn.
	_shooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	//Super::NativeInitializeAnimation();
}
