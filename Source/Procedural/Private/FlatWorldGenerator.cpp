// Fill out your copyright notice in the Description page of Project Settings.

#include "FlatWorldGenerator.h"

int UFlatWorldGenerator::GetDefaultValue(FIntVector Position)
{
	return (Position.Z == Height) ? 0 : ((Position.Z > Height) ? 100 : -100);
}

FColor UFlatWorldGenerator::GetDefaultColor(FIntVector Position)
{
	return (Position.Z == Height) ? FColor::White : ((Position.Z > Height) ? FColor::Red : FColor::Green);
}