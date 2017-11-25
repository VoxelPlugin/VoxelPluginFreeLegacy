float4 ReadTexture4D(Texture3D Texture, SamplerState TextureSampler, float Radius, float Mu, float MuS, float Nu)
{
	float H = sqrt(RadiusAtmosphere * RadiusAtmosphere - RadiusGround * RadiusGround);
	float Rho = sqrt(Radius * Radius - RadiusGround * RadiusGround);
#if INSCATTER_NON_LINEAR
	float RMu = Radius * Mu;
	float Delta = RMu * RMu - Radius * Radius + RadiusGround * RadiusGround;
	float4 TexOffset = RMu < 0.0 && Delta > 0.0 ? float4(1.0, 0.0, 0.0, 0.5 - 0.5 / float(InscatteringMuSampleCount)) : float4(-1.0, H * H, H, 0.5 + 0.5 / float(InscatteringMuSampleCount));
	float MuR = 0.5 / float(InscatteringAltitudeSampleCount) + Rho / H * (1.0 - 1.0 / float(InscatteringAltitudeSampleCount));
	float MuMu = TexOffset.w + (RMu * TexOffset.x + sqrt(Delta + TexOffset.y)) / (Rho + TexOffset.z) * (0.5 - 1.0 / float(InscatteringMuSampleCount));
	// paper formula
	//float MuMuS = 0.5 / float(InscatterMuSSampleCount) + max((1.0 - exp(-3.0 * MuS - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / float(InscatterMuSSampleCount));
	// better formula
	float MuMuS = 0.5 / float(InscatteringMuSSampleCount) + (atan(max(MuS, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / float(InscatteringMuSSampleCount));
#else
	float MuR = 0.5 / float(InscatteringAltitudeSampleCount) + Rho / H * (1.0 - 1.0 / float(InscatteringAltitudeSampleCount));
	float MuMu = 0.5 / float(InscatteringMuSampleCount) + (Mu + 1.0) * 0.5f * (1.0 - 1.0 / float(InscatteringMuSampleCount));
	float MuMuS = 0.5 / float(InscatteringMuSSampleCount) + max(MuS + 0.2, 0.0) / 1.2 * (1.0 - 1.0 / float(InscatteringMuSSampleCount));
#endif
	float LerpValue = (Nu + 1.0) * 0.5f * (float(InscatteringNuSampleCount) - 1.0);
	float MuNu = floor(LerpValue);
	LerpValue = LerpValue - MuNu;


	return Texture.SampleLevel(TextureSampler, float3((MuNu + MuMuS) / float(InscatteringNuSampleCount), MuMu, MuR), 0) * (1.0 - LerpValue) +
		Texture.SampleLevel(TextureSampler, float3((MuNu + MuMuS + 1.0) / float(InscatteringNuSampleCount), MuMu, MuR), 0) * LerpValue;
}




//inscattered light along ray x+tv, when sun in direction s (=S[L]-T(x,x0)S[L]|x0)
float3 GetInscatterColor(in float3 X, inout float T, float3 V, float3 S, float Radius, float Mu, out float3 Attenuation)
{
	const float epsilon = 0.005f;


	float3 Result = 0;
	Attenuation = 1;


	float D = -Radius * Mu - sqrt(Radius * Radius * (Mu * Mu - 1.0) + RadiusAtmosphere * RadiusAtmosphere);
	if (D > 0.0)
	{ // if x in space and ray intersects atmosphere
	  // move x to nearest intersection of ray with top atmosphere boundary
		X += D * V;
		T -= D;
		Mu = (Radius * Mu + D) / RadiusAtmosphere;
		Radius = RadiusAtmosphere;
	}
	if (Radius <= RadiusAtmosphere)
	{ // if ray intersects atmosphere
		float Nu = dot(V, S);
		float MuS = dot(X, S) / Radius;
		float PhaseR = PhaseFunctionR(Nu);
		float PhaseM = PhaseFunctionM(Nu);
		float4 Inscatter = max(ReadTexture4D(AtmosphereInscatterTexture, AtmosphereInscatterTextureSampler, Radius, Mu, MuS, Nu), 0.0);
		if (T > 0.0)
		{
			float3 X0 = X + T * V;
			float R0 = length(X0);
			float RMu0 = dot(X0, V);
			float Mu0 = RMu0 / R0;
			float MuS0 = dot(X0, S) / R0;
#ifdef ATMOSPHERIC_TEXTURE_SAMPLE_FIX
			// avoids imprecision problems in transmittance computations based on textures
			Attenuation = AnalyticTransmittance(Radius, Mu, T);
#else
			Attenuation = TransmittanceWithDistance(Radius, Mu, V, X0);
#endif
			if (R0 > RadiusGround + 0.01)
			{
				// computes S[L]-T(x,x0)S[L]|x0
				Inscatter = max(Inscatter - Attenuation.rgbr * ReadTexture4D(AtmosphereInscatterTexture, AtmosphereInscatterTextureSampler, R0, Mu0, MuS0, Nu), 0.0);
#ifdef ATMOSPHERIC_TEXTURE_SAMPLE_FIX
				// avoids imprecision problems near horizon by interpolating between two points above and below horizon
				float MuHoriz = -sqrt(1.0 - (RadiusGround / Radius) * (RadiusGround / Radius));
				if (abs(Mu - MuHoriz) < epsilon)
				{
					float Alpha = ((Mu - MuHoriz) + epsilon) / (2.0 * epsilon);


					Mu = MuHoriz - epsilon;
					R0 = sqrt(Radius * Radius + T * T + 2.0 * Radius * T * Mu);
					Mu0 = (Radius * Mu + T) / R0;
					float4 InScatter0 = ReadTexture4D(AtmosphereInscatterTexture, AtmosphereInscatterTextureSampler, Radius, Mu, MuS, Nu);
					float4 InScatter1 = ReadTexture4D(AtmosphereInscatterTexture, AtmosphereInscatterTextureSampler, R0, Mu0, MuS0, Nu);
					float4 InScatterA = max(InScatter0 - Attenuation.rgbr * InScatter1, 0.0);


					Mu = MuHoriz + epsilon;
					R0 = sqrt(Radius * Radius + T * T + 2.0 * Radius * T * Mu);
					Mu0 = (Radius * Mu + T) / R0;
					InScatter0 = ReadTexture4D(AtmosphereInscatterTexture, AtmosphereInscatterTextureSampler, Radius, Mu, MuS, Nu);
					InScatter1 = ReadTexture4D(AtmosphereInscatterTexture, AtmosphereInscatterTextureSampler, R0, Mu0, MuS0, Nu);
					float4 InScatterB = max(InScatter0 - Attenuation.rgbr * InScatter1, 0.0);


					Inscatter = lerp(InScatterA, InScatterB, Alpha);
				}
#endif
			}
		}
#ifdef ATMOSPHERIC_TEXTURE_SAMPLE_FIX
		// avoids imprecision problems in Mie scattering when sun is below horizon
		Inscatter.w *= smoothstep(0.00, 0.02, MuS);
#endif
		Result = max(Inscatter.rgb * PhaseR + GetMie(Inscatter) * PhaseM, 0.0);
	}




	return Result;
}




//ground radiance at end of ray x+tv, when sun in direction s
//attenuated bewteen ground and viewer (=R[L0]+R[L*])
float3 GetGroundColor(float4 SceneColor, float3 x, float t, float3 v, float3 s, float r, float mu, float3 attenuation)
{
	float3 result = 0;
	if (t > 0.0)
	{ // if ray hits ground surface
	  // ground reflectance at end of ray, x0
		float3 x0 = x + t * v;
		float r0 = length(x0);
		float3 n = x0 / r0;
		float4 reflectance = SceneColor* float4(0.2, 0.2, 0.2, 1.0);


		// direct sun light (radiance) reaching x0
		float muS = dot(n, s);
		float3 sunLight = TransmittanceWithShadow(r0, muS);


		// precomputed sky light (irradiance) (=E[L*]) at x0
		float3 groundSkyLight = Irradiance(AtmosphereIrradianceTexture, AtmosphereIrradianceTextureSampler, r0, muS);


		// light reflected at x0 (=(R[L0]+R[L*])/T(x,x0))
		float3 groundColor = (reflectance.rgb * (max(muS, 0.0) * sunLight + groundSkyLight)) / PI;


		// water specular color due to sunLight
		if (reflectance.w > 0.0)
		{
			float3 h = normalize(s - v);
			float fresnel = 0.02 + 0.98 * pow(1.0 - dot(-v, h), 5.0);
			float waterBrdf = fresnel * pow(max(dot(h, n), 0.0), 150.0);
			groundColor += reflectance.w * max(waterBrdf, 0.0) * sunLight;
		}


		result = attenuation * groundColor; //=R[L0]+R[L*]
	}


	return result;
}


// direct sun light for ray x+tv, when sun in direction s (=L0)
float3 GetSunColor(float3 x, float t, float3 v, float3 s, float r, float mu)
{
	if (t > 0.0)
	{
		return float3(0.f, 0.f, 0.f);
	}
	else
	{
		float3 transmittance = r <= RadiusAtmosphere ? TransmittanceWithShadow(r, mu) : float3(1.f, 1.f, 1.f); // T(x,xo)
		float isun = step(cos(PI / 180.0), dot(v, s)) /** ISun*/; // Lsun
		return transmittance * isun; // Eq (9)
	}
}






float4 GetAtmosphere(float3 ViewPosition, float3 ViewVector, float SceneDepth, float3 SceneColor)
{
	const float AtmosphereHeightScale = 0.0001f;




	ViewPosition *= AtmosphereHeightScale;
	ViewPosition.y += RadiusGround + 0.01f;




	float3 V = normalize(ViewVector.xyz);
	float  Radius = length(ViewPosition);
	float  Mu = dot(ViewPosition, V) / Radius;
	float  T = -Radius*Mu - sqrt(Radius*Radius*(Mu*Mu - 1.f) + RadiusGround * RadiusGround);


	float3 Attenuation;


	float3 InscatterColor = GetInscatterColor(ViewPosition, T, V, Camera.AtmosphereSunDirection, Radius, Mu, Attenuation); ////S[L]-T(x,xs)S[l]|xs
	float3 GroundColor = GetGroundColor(float4(SceneColor.xyz, 1.f), ViewPosition, T, V, Camera.AtmosphereSunDirection, Radius, Mu, Attenuation); ////R[L0]+R[L*]
	float3 SunColor = GetSunColor(ViewPosition, T, V, Camera.AtmosphereSunDirection, Radius, Mu); //L0


	float3 Color = (InscatterColor /*+ GroundColor*/ + SunColor) * Camera.AtmosphereSunPower * Camera.AtmosphereSunColor;
	return float4(Color, 1.f);
}