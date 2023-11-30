// Fill out your copyright notice in the Description page of Project Settings.


#include "SwordsRoyaleWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "SwordsRoyaleCharacter.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASwordsRoyaleWeapon::ASwordsRoyaleWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Definition for the Mesh that will serve as your visual representation.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> DefaultMesh(TEXT("/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_HeroSword11/SK_Blade_HeroSword11"));
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(SkeletalMesh);

	//Set the Static Mesh and its position/scale if you successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		SkeletalMesh->SetSkeletalMesh(DefaultMesh.Object);
	}

	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;

}

// Called when the game starts or when spawned
void ASwordsRoyaleWeapon::BeginPlay()
{
	Super::BeginPlay();
	Wielder = Cast<ASwordsRoyaleCharacter>(this->Owner);
	// You can use FCollisionQueryParams to further configure the query
	// Here we add ourselves to the ignored list so we won't block the trace
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(this->Owner);
}

// Called every frame
void ASwordsRoyaleWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsValid(Wielder) && Wielder->GetIsAttacking())
	{
		CheckWeponHit();
	}
	// Reset ignored actors after attack when wielder is not attacking anymore.
	if (QueryParams.GetIgnoredActors().Num() > 2 && !Wielder->GetIsAttacking())
	{
		QueryParams.ClearIgnoredActors();
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(this->Owner);
	}

}

void ASwordsRoyaleWeapon::CheckWeponHit()
{
	/*
		Hit detection courtesy of:
		https://dev.epicgames.com/community/snippets/2rR/simple-c-line-trace-collision-query
	*/
	// We set up a line trace from our current location to a point 1000cm ahead of us
	FVector TraceStart = SkeletalMesh->GetSocketLocation("sword_start");
	FVector TraceEnd = SkeletalMesh->GetSocketLocation("sword_end");

	// To run the query, you need a pointer to the current level, which you can get from an Actor with GetWorld()
	// UWorld()->LineTraceSingleByChannel runs a line trace and returns the first actor hit over the provided collision channel.
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TraceChannelProperty, QueryParams);

	// You can use DrawDebug helpers and the log to help visualize and debug your trace queries.
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, Hit.bBlockingHit ? FColor::Blue : FColor::Red, false, 1.0f, 0, 1.0f);

	if (Hit.bBlockingHit && IsValid(Hit.GetActor()))
	{
		ASwordsRoyaleCharacter* OtherActor = Cast<ASwordsRoyaleCharacter>(Hit.GetActor());
		if (IsValid(OtherActor))
		{
			QueryParams.AddIgnoredActor(OtherActor);
			HandleHit(OtherActor);
		}
	}
}

void ASwordsRoyaleWeapon::HandleHit_Implementation(ASwordsRoyaleCharacter* OtherActor)
{
	if (OtherActor->GetIsBlocking())
	{
		Wielder->SetStunned();
	}
	else
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigator()->Controller, this, DamageType);
	}
}


