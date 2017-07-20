#pragma once

struct FVector;


struct TRIANGLE
{
    FVector p[3];
};

struct GRIDCELL
{
    FVector p[8];
    float val[8];
};

int PolygoniseF(GRIDCELL grid, TRIANGLE *triangles);

FVector VertexInterp(FVector p1, FVector p2, float valp1, float valp2);
