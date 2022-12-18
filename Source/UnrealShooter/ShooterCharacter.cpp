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
	
	//Create a camera boom (pulls in towards the character if there is a collision)
	_cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	_cameraBoom->SetupAttachment(RootComponent);
	_cameraBoom->TargetArmLength = 300.f; // Camera follows at this distance from character
	_cameraBoom->bUsePawnControlRotation = true; // Use rotation from controller

	//Create a follow camera
	_followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	_followCamera->SetupAttachment(_cameraBoom, USpringArmComponent::SocketName); // Attach camera to end of arm
	_followCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate when the camera rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f,0.f); // ... at this rotation rate.
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	_baseTurnRate = 45.f;
	_baseLookUpRate = 45.f;
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

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	//Action mappings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireWeapon);

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

		BulletLineTraceAndFX(socketTransform, false);
	}

	
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if(animInstance && _hipFireMontage)
	{
		animInstance->Montage_Play(_hipFireMontage); // Play hipFire
		animInstance->Montage_JumpToSection(FName("StartFire")); // Jump to the StartFire beginning of the montage section
	}
}


const void AShooterCharacter::BulletLineTraceAndFX(const FTransform bulletFireSocketTransform, bool drawDebugLines)
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
		if(drawDebugLines == true)
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
