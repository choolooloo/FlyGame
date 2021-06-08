// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone.h"
#include <Components/StaticMeshComponent.h>
#include <Components/BoxComponent.h>
#include <PhysicsEngine/PhysicsThrusterComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>

// Sets default values
ADrone::ADrone()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	OutCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("OutCollision"));
	RootComponent = OutCollision;
	OutCollision->SetBoxExtent(FVector(100.0f));
	OutCollision->SetSimulatePhysics(true);
	OutCollision->BodyInstance.bLockXRotation = true;
	OutCollision->BodyInstance.bLockYRotation = true;
	OutCollision->SetCollisionProfileName(TEXT("Pawn"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(OutCollision);

	Paddle1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Paddle1"));
	Paddle2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Paddle2"));
	Paddle3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Paddle3"));
	Paddle4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Paddle4"));
	Paddle1->SetupAttachment(Mesh, TEXT("Paddle1"));
	Paddle2->SetupAttachment(Mesh, TEXT("Paddle2"));
	Paddle3->SetupAttachment(Mesh, TEXT("Paddle3"));
	Paddle4->SetupAttachment(Mesh, TEXT("Paddle4"));

	PaddleList.Add(Paddle1);
	PaddleList.Add(Paddle2);
	PaddleList.Add(Paddle3);
	PaddleList.Add(Paddle4);

	UpThruster = CreateDefaultSubobject<UPhysicsThrusterComponent>(TEXT("UpThruster"));
	UpThruster->SetupAttachment(RootComponent);

	//UE_LOG(LogTemp, Warning, TEXT("the value of mass:%i"), OutCollision->GetMass());
	UpThruster->ThrustStrength = 980.0f;

	UpThruster->SetAutoActivate(true);
	UpThruster->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(-this->GetActorUpVector()));

	ForwardThruster = CreateDefaultSubobject<UPhysicsThrusterComponent>(TEXT("ForwardThruster"));
	ForwardThruster->SetupAttachment(RootComponent);
	ForwardThruster->ThrustStrength = 0.0f;
	ForwardThruster->SetAutoActivate(true);
	ForwardThruster->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(-this->GetActorForwardVector()));

	Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));
	Arm->SetupAttachment(Mesh);
	Arm->TargetArmLength = 500.0f;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Arm);
}

// Called when the game starts or when spawned
void ADrone::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetInputAxisValue(TEXT("Lift")) == 0)
	{
		UpThruster->ThrustStrength = 980.0f;
	}
	if (GetInputAxisValue(TEXT("Forward")) == 0)
	{
		ForwardThruster->ThrustStrength = 0.0f;

		float CurrentPitch = Mesh->GetRelativeRotation().Pitch;
		if (CurrentPitch != 0.0f)
		{
			Mesh->AddRelativeRotation(FRotator(-CurrentPitch * DeltaTime, 0.0f, 0.0f));
			if (FMath::Abs(Mesh->GetRelativeRotation().Pitch) <= KINDA_SMALL_NUMBER)
			{
				Mesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
			}
		}
	}
	RotatePaddle(DeltaTime);
	//Arm->AddRelativeRotation(FRotator(GetInputAxisValue(TEXT("Mouse_Y")), GetInputAxisValue(TEXT("Mouse_X")), 0.0f));
	//AddControllerYawInput(GetInputAxisValue(TEXT("Mouse_X")));
}

// Called to bind functionality to input
void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("Lift"), this, &ADrone::Lift);
	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &ADrone::Forward);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ADrone::Trun);
	PlayerInputComponent->BindAxis(TEXT("Mouse_X"));
	PlayerInputComponent->BindAxis(TEXT("Mouse_Y"));
}

void ADrone::Lift(float value)
{
	UpThruster->ThrustStrength += value * LiftAcc * GetWorld()->DeltaTimeSeconds;
	UpThruster->ThrustStrength = FMath::Clamp(UpThruster->ThrustStrength, -LiftThrustMax, LiftThrustMax);
}

void ADrone::Forward(float value)
{
	ForwardThruster->ThrustStrength += value * ForwardAcc * GetWorld()->DeltaTimeSeconds;
	ForwardThruster->ThrustStrength = FMath::Clamp(ForwardThruster->ThrustStrength, -ForwardThrustMax, ForwardThrustMax);

	if (FMath::Abs(Mesh->GetRelativeRotation().Pitch) < 20.0f)
	{
		Mesh->AddRelativeRotation(FRotator(-30.0f * value * GetWorld()->DeltaTimeSeconds, 0.0f, 0.0f));
	}
}

void ADrone::Trun(float value)
{
	OutCollision->AddTorqueInDegrees(-this->GetActorUpVector() * value * TurnStrength);
}

void ADrone::RotatePaddle(float DeltaTime)
{
	for (auto paddle : PaddleList)
	{
		paddle->AddRelativeRotation(FRotator(0.0f, DeltaTime * RotationSpeed, 0.0f));
	}
}

