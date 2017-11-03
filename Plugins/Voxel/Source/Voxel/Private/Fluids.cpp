#include "VoxelPrivatePCH.h"
#include "Fluids.h"

#define Ix(i, j) ((i) + (N + 2) * (j))

void SetBnd(const int N, const int b, TArray<float>& x)
{
	for (int i = 1; i <= N; i++)
	{
		x[Ix(0, i)] = b == 1 ? -x[Ix(1, i)] : x[Ix(1, i)];
		x[Ix(N + 1, i)] = b == 1 ? -x[Ix(N, i)] : x[Ix(N, i)];
		x[Ix(i, 0)] = b == 2 ? -x[Ix(i, 1)] : x[Ix(i, 1)];
		x[Ix(i, N + 1)] = b == 2 ? -x[Ix(i, N)] : x[Ix(i, N)];
	}
	x[Ix(0, 0)] = 0.5 * (x[Ix(1, 0)] + x[Ix(0, 1)]);
	x[Ix(0, N + 1)] = 0.5 * (x[Ix(1, N + 1)] + x[Ix(0, N)]);
	x[Ix(N + 1, 0)] = 0.5 * (x[Ix(N, 0)] + x[Ix(N + 1, 1)]);
	x[Ix(N + 1, N + 1)] = 0.5 * (x[Ix(N, N + 1)] + x[Ix(N + 1, N)]);
}


void AddSource(const int N, TArray<float>& x, TArray<float>& s, const float dt)
{
	for (int i = 0; i < (N + 2) * (N + 2); i++)
	{
		x[i] += dt * s[i];
	}
}

void Diffuse(const int N, const int b, TArray<float>& x, TArray<float>& x0, const float diff, const float dt)
{
	float a = dt * diff * N * N;
	for (int k = 0; k < 20; k++)
	{
		for (int i = 1; i <= N; i++)
		{
			for (int j = 1; j <= N; j++)
			{
				x[Ix(i, j)] = (x0[Ix(i, j)] + a * (x[Ix(i - 1, j)] + x[Ix(i + 1, j)] + x[Ix(i, j - 1)] + x[Ix(i, j + 1)])) / (1 + 4 * a);
			}
		}
		SetBnd(N, b, x);
	}
}

void Advect(const int N, const int b, TArray<float>& d, TArray<float>& d0, TArray<float>& u, TArray<float>& v, const float dt)
{
	const float dt0 = dt * N;
	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			float x = i - dt0 * u[Ix(i, j)];
			float y = j - dt0 * v[Ix(i, j)];

			if (x < 0.5)
			{
				x = 0.5;
			}
			if (x > N + 0.5)
			{
				x = N + 0.5;
			}
			const int i0 = (int)x;
			const int i1 = i0 + 1;

			if (y < 0.5)
			{
				y = 0.5;
			}
			if (y > N + 0.5)
			{
				y = N + 0.5;
			}
			const int j0 = (int)y;
			const int j1 = j0 + 1;

			const float s1 = x - i0;
			const float s0 = 1 - s1;
			const float t1 = y - j0;
			const float t0 = 1 - t1;

			d[Ix(i, j)] = s0 * (t0 * d0[Ix(i0, j0)] + t1 * d0[Ix(i0, j1)]) + s1 * (t0 * d0[Ix(i1, j0)] + t1 * d0[Ix(i1, j1)]);
		}
	}
	SetBnd(N, b, d);
}

void DensityStep(const int N, TArray<float>& x, TArray<float>& x0, TArray<float>& u, TArray<float>& v, const float diff, const float dt)
{
	AddSource(N, x, x0, dt);
	Diffuse(N, 0, x0, x, diff, dt);
	Advect(N, 0, x, x0, u, v, dt);
}

void Project(const int N, TArray<float>& u, TArray<float>& v, TArray<float>& p, TArray<float>& div)
{
	const float h = 1.0 / N;
	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			div[Ix(i, j)] = -0.5 * h * (u[Ix(i + 1, j)] - u[Ix(i - 1, j)] + v[Ix(i, j + 1)] - v[Ix(i, j - 1)]);
			p[Ix(i, j)] = 0;
		}
	}

	SetBnd(N, 0, div);
	SetBnd(N, 0, p);

	for (int k = 0; k < 20; k++)
	{
		for (int i = 1; i <= N; i++)
		{
			for (int j = 1; j <= N; j++)
			{
				p[Ix(i, j)] = (div[Ix(i, j)] + p[Ix(i - 1, j)] + p[Ix(i + 1, j)] + p[Ix(i, j - 1)] + p[Ix(i, j + 1)]) / 4;
			}
		}
		SetBnd(N, 0, p);
	}
	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			u[Ix(i, j)] -= 0.5 * (p[Ix(i + 1, j)] - p[Ix(i - 1, j)]) / h;
			v[Ix(i, j)] -= 0.5 * (p[Ix(i, j + 1)] - p[Ix(i, j - 1)]) / h;
		}
	}

	SetBnd(N, 1, u);
	SetBnd(N, 2, v);
}

void VelocityStep(const int N, TArray<float>& u, TArray<float>& v, TArray<float>& u0, TArray<float>& v0, const float visc, const float dt)
{
	AddSource(N, u, u0, dt);
	AddSource(N, v, v0, dt);

	Diffuse(N, 1, u0, u, visc, dt);
	Diffuse(N, 2, v0, v, visc, dt);
	Project(N, u0, v0, u, v);

	Advect(N, 1, u, u0, u0, v0, dt);
	Advect(N, 2, v, v0, u0, v0, dt);
	Project(N, u, v, u0, v0);
}

void FluidStep(const int N, TArray<float> PrevDens, TArray<float> PrevU, TArray<float> PrevV, const float Visc, const float Diff, const float Dt, TArray<float>& OutDens, TArray<float>& OutU, TArray<float>& OutV)
{
	OutDens.SetNum((N + 2) * (N + 2));
	OutU.SetNum((N + 2) * (N + 2));
	OutV.SetNum((N + 2) * (N + 2));

	VelocityStep(N, OutU, OutV, PrevU, PrevV, Visc, Dt);
	DensityStep(N, OutDens, PrevDens, OutU, OutV, Diff, Dt);
}
