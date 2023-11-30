// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwordsRoyaleWeapon.generated.h"

UCLASS()
class SWORDSROYALE_API ASwordsRoyaleWeapon : public AActor
{
	GENERATED_BODY()

	// FHitResult will hold all data returned by our line collision query
	FHitResult Hit;

	class USkeletalMeshComponent* SkeletalMesh;

	TSubclassOf<class UDamageType> DamageType;

	float Damage;

	UPROPERTY(EditAnywhere, Category = "Collision")
	TEnumAsByte<ECollisionChannel> TraceChannelProperty = ECC_Pawn;
	FCollisionQueryParams QueryParams;

	class ASwordsRoyaleCharacter* Wielder;
	
public:	
	// Sets default values for this actor's properties
	ASwordsRoyaleWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called by Tick to see i weapon collided
	void CheckWeponHit();

	UFUNCTION(Server, Reliable)
	void HandleHit(ASwordsRoyaleCharacter* OtherActor);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
