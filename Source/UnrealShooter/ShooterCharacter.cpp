// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"


AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Base rates for turning/looking up.
	_baseTurnRate = 45.f;
	_baseLookUpRate = 45.f;

	// Turn rates for aiming/not aiming.
	_hipTurnRate = 90.f;
	_hipLookUpRate = 90.f;
	_aimingTurnRate = 20.f;
	_aimingLookUpRate = 20.f;

	// Mouse look sensitivity multiplier factors
	_mouseHipTurnRate = 1.0f;
	_mouseHipLookUpRate = 1.0f;
	_mouseAimingTurnRate = 0.2f;
	_mouseAimingLookUpRate = 0.2f;
	
	_bIsAiming = false;
	
	// Camera field of view values.
	_cameraDefaultFOV = 0.f; // cameraDefaultFOV is set in BeginPlay.
	_cameraZoomedFOV = 35.f;
	_cameraCurrentFOV = 0.f;
	_zoomInterpolationSpeed = 30.f;

	// Crosshair spread factors
	_crossHairSpreadMultiplier = 0.f;
	_crossHairVelocityMultiplier = 0.f;
	_crossHairInAirMultiplier = 0.f;
	_crossHairAimMultiplier = 0.f;
	_crossHairShootingMultiplier = 0.f;

	// Automatic gun firing variables
	_automaticFireRate = 0.1f;
	_bShouldFire = true;
	_bFireButtonPressed = false;

	// Bullet fire timer variables
	_shootTimeDuration = 0.05f;
	_bFiringBullet = false;

	_crosshairShootTimer;
	
	
	//Create a camera boom (pulls in towards the character if there is a collision)
	_cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("_cameraBoom"));
	_cameraBoom->SetupAttachment(RootComponent);
	_cameraBoom->TargetArmLength = 180.f; // Camera follows at this distance from character
	_cameraBoom->bUsePawnControlRotation = true; // Use rotation from controller
	_cameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//Create a follow camera
	_followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("_followCamera"));
	_followCamera->SetupAttachment(_cameraBoom, USpringArmComponent::SocketName); // Attach camera to end of arm
	_followCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate when the camera rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // If true character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f,0.f); // ... at this rotation rate.
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}


void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(_followCamera)
	{
		_cameraDefaultFOV = GetFollowCamera()->FieldOfView;
		_cameraCurrentFOV = _cameraDefaultFOV;
	}
	
}


void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CameraInterpolatingZoomFOV(DeltaTime);

	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);
	
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	//Action mappings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotator { 0, Rotation.Yaw, 0};

		const FVector Direction { FRotationMatrix{YawRotator}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);

		/*double rot = YawRotator.Yaw;
		UE_LOG(LogTemp, Display,TEXT("Input tensor: %f"), rot);*/
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotator { 0, Rotation.Yaw, 0};

		const FVector Direction { FRotationMatrix{YawRotator}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);

		/*double rot = YawRotator.Yaw;
		UE_LOG(LogTemp, Display,TEXT("Input tensor: %f"), rot);*/
	}
}

void AShooterCharacter::TurnAtRate(float turnRateMultiplier)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(turnRateMultiplier * _baseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::LookUpRate(float lookUpRateMultiplier)
{
	AddControllerPitchInput(lookUpRateMultiplier * _baseLookUpRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::Turn(float value)
{
	float turnScaleFactor{};
	
	if (_bIsAiming)
	{
		turnScaleFactor = _mouseAimingTurnRate;
	}
	else
	{
		turnScaleFactor = _mouseAimingTurnRate;
	}
	AddControllerYawInput(value* turnScaleFactor);
}

void AShooterCharacter::LookUp(float value)
{
	float lookUpScaleFactor{};
	
	if (_bIsAiming)
	{
		lookUpScaleFactor = _mouseAimingTurnRate;
	}
	else
	{
		lookUpScaleFactor = _mouseAimingTurnRate;
	}
	AddControllerPitchInput(value* lookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if(_fireSound)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Fire Weapon Sound."));
		UGameplayStatics::PlaySound2D(this, _fireSound);
	}

	// Play muzzle flash FX.
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		FTransform socketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		// if not moving use default socket transform location.
		if (GetCharacterMovement()->GetCurrentAcceleration().Size() == 0.f && _muzzleFlash)
		{
			
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _muzzleFlash, socketTransform);
			UE_LOG(LogTemp, Warning, TEXT("Fire not Moving."));
		}

		// if moving change socket transform location. Otherwise muzzleFlash plays at wrong location.
		else if (GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f && _muzzleFlash)
		{
		
			BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket_Moving");
			socketTransform = BarrelSocket->GetSocketTransform(GetMesh());
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _muzzleFlash, socketTransform);
			UE_LOG(LogTemp, Warning, TEXT("Fire Moving."));
			
		}

		//BulletLineTraceAndFX(socketTransform, false);
		
		BulletLineTraceAndFX_FromCrosshair( socketTransform, false);
		
	}

	StartCrosshairBulletFire();
	
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if(animInstance && _hipFireMontage)
	{
		animInstance->Montage_Play(_hipFireMontage); // Play hipFire
		animInstance->Montage_JumpToSection(FName("StartFire")); // Jump to the StartFire beginning of the montage section
	}
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	_bFiringBullet = true;

	GetWorldTimerManager().SetTimer(_crosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, _shootTimeDuration);
}
 
void AShooterCharacter::FinishCrosshairBulletFire()
{
	_bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	_bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	_bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (_bShouldFire)
	{
		FireWeapon();
		_bShouldFire = false;
		GetWorldTimerManager().SetTimer(_autoFireTimer, this, &AShooterCharacter::AutoFireReset, _automaticFireRate);
	}
}

void AShooterCharacter::AutoFireReset()
{
	_bShouldFire = true;

	// If fire button is still pressed, loop the StartFireTimer function (for automatic firing).
	if (_bFireButtonPressed)
	{
		StartFireTimer();
	}
}

const void AShooterCharacter::BulletLineTraceAndFX(const FTransform bulletFireSocketTransform, bool bDrawDebugLines)
{
	// Implementing line trace for bullet hits.
	FHitResult fireHit;
	const FVector shotStartLocation {bulletFireSocketTransform.GetLocation()};
	const FQuat ShotQuaternion {bulletFireSocketTransform.GetRotation()};

	const FVector shotRotationAxis {ShotQuaternion.GetAxisX()}; // This is the same X-axis as our firing sockets.
	const FVector shotEndLocation {shotStartLocation + shotRotationAxis*50'000.f};

	FVector beamEndPoint{ shotEndLocation }; // Location used for smoke trails.

		
	GetWorld()->LineTraceSingleByChannel(fireHit, shotStartLocation, shotEndLocation, ECC_Visibility );

	if (fireHit.bBlockingHit)
	{
		if(bDrawDebugLines == true)
		{
			DrawDebugLine(GetWorld(), shotStartLocation, shotEndLocation, FColor::Red, false, 2.f);
			DrawDebugPoint(GetWorld(), fireHit.Location, 5.f, FColor::Red, false, 2.f );
		}
		
		beamEndPoint = fireHit.Location;
			
		if(_impactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _impactParticles, fireHit.Location);
		}
	}

	// Beam trails between weapon fire socket and the end point of the bullet trajectory
	if (_beamParticles)
	{
		UParticleSystemComponent* beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),_beamParticles, bulletFireSocketTransform);
		if (beam)
		{
			beam->SetVectorParameter(FName("Target"), beamEndPoint);
		}
	}
}

const void AShooterCharacter::BulletLineTraceAndFX_FromCrosshair( const FTransform bulletFireSocketTransform, bool bDrawDebugLines)
{
	// Create a vector 2D and assign its value to the current viewport size.
		FVector2d viewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(viewportSize);
		}

		// Get screen space location of cross-hairs.
		FVector2d crossHairLocation(viewportSize.X/2.f , viewportSize.Y / 2.f);
		//crossHairLocation.Y -= 50.f; // Only uncomment this if also subtracting from screen center position in hudBP to get the new crosshair location.

		FVector crossHairWorldPosition;
		FVector crossHairWorldDirection;

		APlayerController* GetPlayerController = UGameplayStatics::GetPlayerController(this, 0);

		/*Get the values of the vectors for the cross hair world position and direction and set them to out variables.
		 The following function also returns a boolean and it is true if it was successful.*/
		bool bScreenToWorldDeprojectionSuccessful = UGameplayStatics::DeprojectScreenToWorld(
			GetPlayerController,
			crossHairLocation,
			crossHairWorldPosition,
			crossHairWorldDirection);

		if (bScreenToWorldDeprojectionSuccessful == true)
		{
			FHitResult screenTraceHit;
			const FVector shotStartLocation{crossHairWorldPosition};
			const FVector shotEndLocation{crossHairWorldPosition + crossHairWorldDirection * 50'000.f};

			// Set beam end point to the line trace end point.
			FVector beamEndPoint {shotEndLocation};

			// Trace outward from cross-hairs world location.
			GetWorld()->LineTraceSingleByChannel(screenTraceHit, shotStartLocation, shotEndLocation, ECC_Visibility);

			if (screenTraceHit.bBlockingHit) // was there a trace hit?
			{
				
				
				beamEndPoint = screenTraceHit.Location; // beam end point is now trace hit location

				if(_impactParticles != nullptr) // was impact particles assigned in the character blueprint?
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _impactParticles, screenTraceHit.Location);
				}
			}

			/* Perform a second trace, this time from the gun barrel to the end point to check if there is another
			object in the way */
			FHitResult	weaponTraceHit;

			const FVector weaponTraceStart { bulletFireSocketTransform.GetLocation() };
			const FVector weaponTraceEnd { beamEndPoint };
			
			GetWorld()->LineTraceSingleByChannel(weaponTraceHit, weaponTraceStart, weaponTraceEnd, ECC_Visibility);

			if (weaponTraceHit.bBlockingHit) // Was there an object between barrel and beamEndPoint? 
			{
				beamEndPoint = weaponTraceHit.Location;
			}
			
			if (_beamParticles != nullptr) // was beam asset assigned in the character blueprint?
			{
				UParticleSystemComponent* beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),_beamParticles, bulletFireSocketTransform);

				if (beam != nullptr)
				{
					beam->SetVectorParameter(FName("Target"), beamEndPoint);
				}
			}

			if(bDrawDebugLines == true)
			{
				DrawDebugLine(GetWorld(), weaponTraceStart, beamEndPoint, FColor::Red, false, 2.f);
				DrawDebugPoint(GetWorld(), beamEndPoint, 5.f, FColor::Red, false, 2.f );
			}
			
		}
}


void AShooterCharacter::AimingButtonPressed()
{
	_bIsAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	_bIsAiming = false;
}

void AShooterCharacter::CameraInterpolatingZoomFOV(float DeltaTime)
{
	// Set current camera field of view.
	if (_bIsAiming)
	{
		// Interpolated to zoomed FOV.
		_cameraCurrentFOV = FMath::FInterpTo(_cameraCurrentFOV, _cameraZoomedFOV,DeltaTime,_zoomInterpolationSpeed);
	}
	else if (!_bIsAiming)
	{
		// Interpolate to default field of view.
		_cameraCurrentFOV = FMath::FInterpTo(_cameraCurrentFOV, _cameraDefaultFOV,DeltaTime,_zoomInterpolationSpeed);
	}
	GetFollowCamera()->SetFieldOfView(_cameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (_bIsAiming)
	{
		_baseTurnRate = _aimingTurnRate;
		_baseLookUpRate = _aimingTurnRate;
	}
	else if (_bIsAiming == false)
	{
		_baseTurnRate = _hipTurnRate;
		_baseLookUpRate = _hipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float Deltatime)
{
	FVector2d walkSpeedRange{0.f , 600.f};

	FVector2d velocityMultiplierRange{0.f , 1.f};

	FVector velocity { GetVelocity() };
	velocity.Z = 0.f;
	

	
	_crossHairVelocityMultiplier = FMath::GetMappedRangeValueClamped(walkSpeedRange, velocityMultiplierRange, velocity.Size());

	// Is character in air?
	if (GetCharacterMovement()->IsFalling())
	{
		// Spread the crosshairs slowly while in air.
		_crossHairInAirMultiplier = FMath::FInterpTo(_crossHairInAirMultiplier, 2.25f, Deltatime, 2.25f);
	}
	else if (GetCharacterMovement()->IsFalling() == false)
	{
		//Shrink the crosshairs rapidly while on the ground
		_crossHairInAirMultiplier = FMath::FInterpTo(_crossHairInAirMultiplier, 0.f, Deltatime, 30.f);
		
	}

	// Is the character aiming?
	if (_bIsAiming == true)
	{
		_crossHairAimMultiplier = FMath::FInterpTo(_crossHairAimMultiplier, 0.6f, Deltatime, 30.f);
	}
	else
	{
		_crossHairAimMultiplier = FMath::FInterpTo(_crossHairAimMultiplier, 0.f, Deltatime, 30.f);
	}

	// Is the character shooting?
	if (_bFiringBullet)
	{
		_crossHairShootingMultiplier = FMath::FInterpTo(_crossHairShootingMultiplier, 0.3f, Deltatime, 60.f);
	}
	else
	{
		_crossHairShootingMultiplier = FMath::FInterpTo(_crossHairShootingMultiplier, 0.f, Deltatime, 60.f);
	}


	
	_crossHairSpreadMultiplier = 0.5f + _crossHairVelocityMultiplier +_crossHairInAirMultiplier -  _crossHairAimMultiplier + _crossHairShootingMultiplier;
	
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return  _crossHairSpreadMultiplier;
}

