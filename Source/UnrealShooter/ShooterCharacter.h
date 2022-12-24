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

	// Line trace function for bullet hits and FX.
	const void BulletLineTraceAndFX(const FTransform bulletFireSocketTransform, const bool bDrawDebugLines);

	// Line trace function for bullet hits  and FX using cross-hairs.
	const void BulletLineTraceAndFX_FromCrosshair(const FTransform bulletFireSocketTransform, const bool bDrawDebugLines);


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

	// Rotate controller based on mouse X movement. @param Value is the input value from the mouse X movement.
	void Turn (float value);

	// Rotate controller based on mouse Y movement. @param Value is the input value from the mouse Y movement.
	void LookUp (float value);

	// Called when the fire button is pressed.
	void FireWeapon();

	/* Set _bIsAiming to true when button press*/
	void AimingButtonPressed();
	/* Set _bIsAiming to false when button release*/
	void AimingButtonReleased();

	// Handle interpolation for zoom when aiming.
	void CameraInterpolatingZoomFOV(float DeltaTime);

	// Set _baseTurnRates and _baseLookUpRate based on aiming. This function changes the look sensitivity based on aiming.
	void SetLookRates();

	// Calculate crosshair spread multiplier.
	void CalculateCrosshairSpread(float Deltatime);

	// Start bullet fire timer for crosshairs.
	void StartCrosshairBulletFire();
	
	UFUNCTION()
	void FinishCrosshairBulletFire();
	
	
private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	/*Camera boom positioning the camera behind the character. */
	class USpringArmComponent* _cameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	/*Camera that follows a character. */
	class UCameraComponent* _followCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	/*Base turn rate in degres/sec. Other scaling may affect final turn rate. */
	float _baseTurnRate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	/*Base turn up/down rate, in degres/sec. Other scaling may affect final turn rate.*/
	float _baseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	// Turn rate while not aiming.
	float _hipTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	// Look up rate when not aiming.
	float _hipLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	// Turn rate when aiming.
	float _aimingTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	// Look up rate when aiming.
	float _aimingLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"));
	// Scale multiplier for mouse sensitivity. Turn rate when not aiming.
	float _mouseHipTurnRate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"));
	// Scale multiplier for mouse sensitivity. Look up rate when not aiming.
	float _mouseHipLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"));
	// Scale multiplier for mouse sensitivity. Turn rate when aiming.
	float _mouseAimingTurnRate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"));
	// Scale multiplier for mouse sensitivity. Look up rate when aiming.
	float _mouseAimingLookUpRate;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	/* Randomize gunshot cue.*/
	class USoundCue* _fireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	/* Flash spawned at BarrelSocket. */
	class UParticleSystem* _muzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	/* Montage for firing the weapon. */
	class UAnimMontage* _hipFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	/* Particles spawned upon bullet impact */
	UParticleSystem* _impactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	/* Smoke trail for bullets. */
	UParticleSystem* _beamParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"));
	// True when aiming.
	bool _bIsAiming;

	// Default camera view value.
	float _cameraDefaultFOV;

	// Field of view value for when zoomed in.
	float _cameraZoomedFOV;

	// Current field of view in the current frame.
	float _cameraCurrentFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	// Interpolation speed for zooming when aiming.
	float _zoomInterpolationSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshaira, meta = (AllowPrivateAccess = "true"));
	// Determines the spread of the cross hairs.
	float _crossHairSpreadMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshaira, meta = (AllowPrivateAccess = "true"));
	// Velocity component for cross hairs spread.
	float _crossHairVelocityMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshaira, meta = (AllowPrivateAccess = "true"));
	// In air component for cross hairs spread.
	float _crossHairInAirMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshaira, meta = (AllowPrivateAccess = "true"));
	// Aim component for cross hairs spread.
	float _crossHairAimMultiplier;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshaira, meta = (AllowPrivateAccess = "true"));
	// Shooting component for cross hairs spread.
	float _crossHairShootingMultiplier;

	float _shootTimeDuration;

	bool _bFiringBullet;

	FTimerHandle _crosshairShootTimer;

public:
	/* Returns Camera Boom sub object.*/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return _cameraBoom; }
	
	/** Returns FollowCamera sub object. **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return _followCamera; }

	// Returns _bIsAiming bool value.
	FORCEINLINE bool GetAiming() const { return _bIsAiming; }

	
	UFUNCTION(BlueprintCallable)
	// Getter function for _crossHairSpreadMultiplier .
	float GetCrosshairSpreadMultiplier() const;

	
	
};
