// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class UNREALSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*
	 * Called via input to turn a given rate.
	 * @param Rate this is  a normalized rate, i.e 1.0 means 100% of desired turn rate.
	 */
	void TurnAtRate(float turnRateMultiplier);
	
	/*
	 * Called via input to look up/down at given rate.
	 * @param Rate this is  a normalized rate, i.e 1.0 means 100% of desired turn rate.
	 */
	void LookUpRate(float lookUpRateMultiplier);

private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	/*Camera boom positioning the camera behind the character. */
	class USpringArmComponent* _cameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	/*Camera that follows a character. */
	class UCameraComponent* _followCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	/*Base turn rate in degres/sec. Other scaling may affect final turn rate. */
	float _baseTurnRate;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	/*Base turn up/down rate, in degres/sec. Other scaling may affect final turn rate.*/
	float _baseLookUpRate;
	
public:
	/* Returns Camera Boom sub object.*/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return _cameraBoom; }
	
	/** Returns FollowCamera sub object. **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return _followCamera; }
	
};
