// Copyright Epic Games, Inc. All Rights Reserved.

#include "SwordsRoyaleCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SwordsRoyaleWeapon.h"


//////////////////////////////////////////////////////////////////////////
// ASwordsRoyaleCharacter

ASwordsRoyaleCharacter::ASwordsRoyaleCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

// Replicated Properties
void ASwordsRoyaleCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(ASwordsRoyaleCharacter, CurrentHealth);
	DOREPLIFETIME(ASwordsRoyaleCharacter, bIsAttacking);
	DOREPLIFETIME(ASwordsRoyaleCharacter, bIsBlocking);
	DOREPLIFETIME(ASwordsRoyaleCharacter, bIsStunned);


}

void ASwordsRoyaleCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	if (GetLocalRole() == ROLE_Authority)
	{
		FVector SocketLocationL = GetMesh()->GetSocketLocation("weapon_socket_l");
		FRotator SocketRotatorL = GetMesh()->GetSocketRotation("weapon_socket_l");

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		Weapon = GetWorld()->SpawnActor<ASwordsRoyaleWeapon>(
			SocketLocationL,
			SocketRotatorL,
			spawnParameters
		);

		Weapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules(
				EAttachmentRule::SnapToTarget,
				false
			),
			"weapon_socket_l"
		);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASwordsRoyaleCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASwordsRoyaleCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASwordsRoyaleCharacter::Look);

		//Striking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ASwordsRoyaleCharacter::StartAttack);
		
		//Blocking
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &ASwordsRoyaleCharacter::Block);
		
		// Dodging
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Completed, this, &ASwordsRoyaleCharacter::Dodge);

	}

}

void ASwordsRoyaleCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASwordsRoyaleCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


void ASwordsRoyaleCharacter::StartAttack()
{
	if (!bIsAttacking)
	{
		HandleAttack();
	}
	
}

void ASwordsRoyaleCharacter::StopAttack()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsAttacking = false;
	}
}

void ASwordsRoyaleCharacter::HandleAttack_Implementation()
{
	bIsAttacking = true;
	OnAttack();
	GetWorld()->GetTimerManager().SetTimer(AttackTimer, this, &ASwordsRoyaleCharacter::StopAttack, AttackAnimMontage->GetPlayLength() / 2, false);
}

void ASwordsRoyaleCharacter::OnRep_Attack()
{
	OnAttack();
}

void ASwordsRoyaleCharacter::OnAttack()
{
	if (AttackAnimMontage != NULL && bIsAttacking)
	{
		// Get the animation object
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(AttackAnimMontage, 2.0f);
		}
	}
}

void ASwordsRoyaleCharacter::Block()
{
	if (!bIsBlocking)
	{
		HandleBlock();
	}
	
}

void ASwordsRoyaleCharacter::StopBlocking()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsBlocking = false;
	}
}

void ASwordsRoyaleCharacter::HandleBlock_Implementation()
{
	bIsBlocking = true;
	OnBlock();
	GetWorld()->GetTimerManager().SetTimer(BlockTimer, this, &ASwordsRoyaleCharacter::StopBlocking, BlockAnimMontage->GetPlayLength() / 2, false);
}

void ASwordsRoyaleCharacter::OnRep_Block() 
{
	OnBlock();
}

void ASwordsRoyaleCharacter::OnBlock() 
{
	if (BlockAnimMontage != NULL && bIsBlocking)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(BlockAnimMontage, 2.0f);
		}
	}
}

void ASwordsRoyaleCharacter::SetStunned()
{
	if (!bIsStunned)
	{
		HandleStunned();
	}
}

void ASwordsRoyaleCharacter::EndStunned()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsStunned = false;
	}
}

void ASwordsRoyaleCharacter::HandleStunned_Implementation()
{
	bIsStunned = true;
	OnStunned();
	GetWorld()->GetTimerManager().SetTimer(StunnedTimer, this, &ASwordsRoyaleCharacter::EndStunned, StunAnimMontage->GetPlayLength(), false);
}

void ASwordsRoyaleCharacter::OnRep_Stunned()
{
	OnStunned();
}

void ASwordsRoyaleCharacter::OnStunned()
{
	if (StunAnimMontage != NULL && bIsStunned)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(StunAnimMontage, 1.0f);
		}
	}
}

void ASwordsRoyaleCharacter::Dodge()
{
	UE_LOG(LogTemp, Warning, TEXT("Player pressed dodge"));
}

void ASwordsRoyaleCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining. Local"), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining. Server"), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
	if (OnHitAnimMontage != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(OnHitAnimMontage, 1.0f);
		}
	}

}

void ASwordsRoyaleCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void ASwordsRoyaleCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float ASwordsRoyaleCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}
