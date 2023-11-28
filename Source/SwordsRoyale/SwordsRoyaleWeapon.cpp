// Fill out your copyright notice in the Description page of Project Settings.


#include "SwordsRoyaleWeapon.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ASwordsRoyaleWeapon::ASwordsRoyaleWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Definition for the Mesh that will serve as your visual representation.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> DefaultMesh(TEXT("/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_HeroSword11/SK_Blade_HeroSword11"));
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SkeletalMesh->SetupAttachment(RootComponent);

	//Set the Static Mesh and its position/scale if you successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		SkeletalMesh->SetSkeletalMesh(DefaultMesh.Object);
	}
}

// Called when the game starts or when spawned
void ASwordsRoyaleWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASwordsRoyaleWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

