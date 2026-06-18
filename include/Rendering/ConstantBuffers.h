#pragma once
#include <Windows.h>
#include "Math/Matrix.h"
#include "Math/Vector2.h"
#define LIGHTS 8
struct CameraConstantBuffer
{
    Matrix viewProjection;
    Vector3 eyePosition;
    float padding;
};
struct MeshConstantBuffer
{
    Matrix world;
    Matrix modelViewProjection;
    BOOL hasDiffuseMap;
    BOOL hasNormalMap;
    BOOL hasSpecularMap;
    BOOL hasEmissiveMap;
    BOOL hasAlphaMap;
    float vertexColorBlending = 0.f;
    BOOL renderFlat = false;
    BOOL enableAutoTangent = false;
    Vector4 materialDiffuse = 0.5f;
    Vector4 materialAmbient = 0.25f;
    Vector4 materialEmissive = 0.f;
    Vector4 materialSpecular = 0.f;
    Vector4 materialReflect = 0.f;
    float materialShininess = 1.f;
    Vector3 padding;
    float blurRadius;
    float blurSigma;
    Vector2 blurDirection;
    Vector2 renderTargetResolution;
    Vector2 padding2;
    Vector4 outlineColor;
    Vector4 meshColor;
};
struct Light
{
    int lightType;
    Vector3 padding;
    Vector4 lightDirection;
    Vector4 lightPosition;
    Vector4 lightAttenuation;
    Vector4 lightSpot;
    Vector4 lightColor;
};
struct LightConstantBuffer
{
    Light lights[LIGHTS];
    Vector4 lightAmbient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
    int lightCount;
    Vector3 padding;
};