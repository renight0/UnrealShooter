// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"



// Sets default values

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
	// calculate delta for this frame from the rate information
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
		UE_LOG(LogTemp, Warning, TEXT("Fire Weapon."));
		UGameplayStatics::PlaySound2D(this, _fireSound);
	}
	
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform socketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (_muzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _muzzleFlash, socketTransform);
		}
	}
	
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if(animInstance && _hipFireMontage)
	{
		animInstance->Montage_Play(_hipFireMontage); // Play hipFire
		animInstance->Montage_JumpToSection(FName("StartFire")); // Jump to the StartFire beginning of the montage section
	}
}