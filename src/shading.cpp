#include "render.h"
#include "texture.h"
#include <cmath>
#include <fmt/core.h>
#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <shading.h>

// This function is provided as-is. You do not have to implement it (unless
// you need to for some extra feature).
// Given render state and an intersection, based on render settings, sample
// the underlying material data in the expected manner.
glm::vec3 sampleMaterialKd(RenderState& state, const HitInfo& hitInfo)
{
    if (state.features.enableTextureMapping && hitInfo.material.kdTexture) {
        if (state.features.enableBilinearTextureFiltering) {
            return sampleTextureBilinear(*hitInfo.material.kdTexture, hitInfo.texCoord);
        } else {
            return sampleTextureNearest(*hitInfo.material.kdTexture, hitInfo.texCoord);
        }
    } else {
        return hitInfo.material.kd;
    }
}

// This function is provided as-is. You do not have to implement it.
// Given a camera direction, a light direction, a relevant intersection, and a color coming in
// from the light, evaluate the scene-selected shading model, returning the reflected light towards the target.
glm::vec3 computeShading(RenderState& state, const glm::vec3& cameraDirection, const glm::vec3& lightDirection, const glm::vec3& lightColor, const HitInfo& hitInfo)
{
    // Hardcoded linear gradient. Feel free to modify this
    static LinearGradient gradient = {
        .components = {
            { 0.1f, glm::vec3(215.f / 256.f, 210.f / 256.f, 203.f / 256.f) },
            { 0.22f, glm::vec3(250.f / 256.f, 250.f / 256.f, 240.f / 256.f) },
            { 0.5f, glm::vec3(145.f / 256.f, 170.f / 256.f, 175.f / 256.f) },
            { 0.78f, glm::vec3(255.f / 256.f, 250.f / 256.f, 205.f / 256.f) },
            { 0.9f, glm::vec3(170.f / 256.f, 170.f / 256.f, 170.f / 256.f) },
        }
    };

    if (state.features.enableShading) {
        switch (state.features.shadingModel) {
            case ShadingModel::Lambertian:
                return computeLambertianModel(state, cameraDirection, lightDirection, lightColor, hitInfo);
            case ShadingModel::Phong:
                return computePhongModel(state, cameraDirection, lightDirection, lightColor, hitInfo);
            case ShadingModel::BlinnPhong:
                return computeBlinnPhongModel(state, cameraDirection, lightDirection, lightColor, hitInfo);
            case ShadingModel::LinearGradient:
                return computeLinearGradientModel(state, cameraDirection, lightDirection, lightColor, hitInfo, gradient);
        };
    }

    return lightColor * sampleMaterialKd(state, hitInfo);
}

// Given a camera direction, a light direction, a relevant intersection, and a color coming in
// from the light, evaluate a Lambertian diffuse shading, returning the reflected light towards the target.
glm::vec3 computeLambertianModel(RenderState& state, const glm::vec3& cameraDirection, const glm::vec3& lightDirection, const glm::vec3& lightColor, const HitInfo& hitInfo)
{
    glm::vec3 l = glm::normalize(lightDirection);
    glm::vec3 n = glm::normalize(hitInfo.normal);

    float dot = glm::dot(l, n);

    if (state.features.enableTransparency && hitInfo.material.transparency < 1.0f && dot < 0) {
        dot = -dot;
    }

    if (dot <= 0)
        return glm::vec3 { 0 };

    return sampleMaterialKd(state, hitInfo) * lightColor * dot;
}

// TODO: Standard feature
// Given a camera direction, a light direction, a relevant intersection, and a color coming in
// from the light, evaluate the Phong Model returning the reflected light towards the target.
// Note: materials do not have an ambient component, so you can ignore this.
// Note: use `sampleMaterialKd` instead of material.kd to automatically forward to texture
//       sampling if a material texture is available!
//
// - state;           the active scene, feature config, and the bvh
// - cameraDirection; exitant vector towards the camera (or secondary position)
// - lightDirection;  exitant vector towards the light
// - lightColor;      the color of light along the lightDirection vector
// - hitInfo;         hit object describing the intersection point
// - return;          the result of shading along the cameraDirection vector
//
// This method is unit-tested, so do not change the function signature.
glm::vec3 computePhongModel(RenderState& state, const glm::vec3& cameraDirection, const glm::vec3& lightDirection, const glm::vec3& lightColor, const HitInfo& hitInfo)
{
    // No ambient, diffuse = Lambertian, so only calculate specular

    glm::vec3 diffuse = computeLambertianModel(state, cameraDirection, lightDirection, lightColor, hitInfo);

    glm::vec3 d = glm::normalize(-lightDirection);
    glm::vec3 n = glm::normalize(hitInfo.normal);

    float dot = glm::dot(-d, n);

    if (state.features.enableTransparency && hitInfo.material.transparency < 1.0f && dot < 0) {
        dot = -dot;
    }

    if (dot <= 0)
        return diffuse;

    glm::vec3 v = glm::normalize(cameraDirection);
    glm::vec3 r = glm::normalize(d - 2.0f * glm::dot(d, n) * n);

    float VdotR = glm::dot(v, r);

    if (VdotR <= 0)
        return diffuse;

    float pow = glm::pow(VdotR, hitInfo.material.shininess);

    glm::vec3 specular = pow * hitInfo.material.ks * lightColor;

    return diffuse + specular;
}

// TODO: Standard feature
// Given a camera direction, a light direction, a relevant intersection, and a color coming in
// from the light, evaluate the Blinn-Phong Model returning the reflected light towards the target.
// Note: materials do not have an ambient component, so you can ignore this.
// Note: use `sampleMaterialKd` instead of material.kd to automatically forward to texture
//       sampling if a material texture is available!
//
// - state;           the active scene, feature config, and the bvh
// - cameraDirection; exitant vector towards the camera (or secondary position)
// - lightDirection;  exitant vector towards the light
// - lightColor;      the color of light along the lightDirection vector
// - hitInfo;         hit object describing the intersection point
// - return;          the result of shading along the cameraDirection vector
//
// This method is unit-tested, so do not change the function signature.
glm::vec3 computeBlinnPhongModel(RenderState& state, const glm::vec3& cameraDirection, const glm::vec3& lightDirection, const glm::vec3& lightColor, const HitInfo& hitInfo)
{
    // Similarly to computePhongModel(), only the specular is needed here

    glm::vec3 diffuse = computeLambertianModel(state, cameraDirection, lightDirection, lightColor, hitInfo);

    glm::vec3 d = glm::normalize(lightDirection);
    glm::vec3 n = glm::normalize(hitInfo.normal);
    float dot = glm::dot(n, d);

    if (dot <= 0)
        return diffuse;

    if (state.features.enableTransparency && hitInfo.material.transparency < 1.0f && dot < 0) {
        dot = -dot;
    }

    glm::vec3 h = glm::normalize(d + cameraDirection);

    float NdotH = glm::dot(n, h);

    if (NdotH <= 0)
        return diffuse;

    float pow = glm::pow(NdotH, hitInfo.material.shininess);

    glm::vec3 specular = pow * hitInfo.material.ks * lightColor;

    return diffuse + specular;
}

// TODO: Standard feature
// Given a number ti between [-1, 1], sample from the gradient's components and return the
// linearly interpolated color, for which ti lies in the interval between the t-values of two
// components, or on a boundary. If ti falls outside the gradient's smallest/largest components,
// the nearest component must be sampled.
// - ti; a number between [-1, 1]
// This method is unit-tested, so do not change the function signature.
glm::vec3 LinearGradient::sample(float ti) const
{
    if (components[0].t > ti)
        return components[0].color;

    for (int i = 0; i < components.size() - 1; i++) {
        if (std::fabs(components[i].t - ti) <= 1e-6f) // ti is on a boundary
            return components[i].color;

        if (components[i].t < ti && components[i + 1].t > ti) {
            float tLeft = components[i].t;
            float tRight = components[i + 1].t;
            glm::vec3 colorLeft = components[i].color;
            glm::vec3 colorRight = components[i + 1].color;

            float alpha = (ti - tLeft) / (tRight - tLeft);
            return (1.0f - alpha) * colorLeft + alpha * colorRight;
        }
    }

    return components[components.size() - 1].color;
}

// TODO: Standard feature
// Given a camera direction, a light direction, a relevant intersection, and a color coming in
// from the light, evaluate a diffuse shading model, such that the diffuse component is sampled not
// from the intersected material, but a provided linear gradient, based on the cosine of theta
// as defined in the diffuse shading part of the Phong model.
//
// - state;           the active scene, feature config, and the bvh
// - cameraDirection; exitant vector towards the camera (or secondary position)
// - lightDirection;  exitant vector towards the light
// - lightColor;      the color of light along the lightDirection vector
// - hitInfo;         hit object describing the intersection point
// - gradient;        the linear gradient object
// - return;          the result of shading
//
// This method is unit-tested, so do not change the function signature.
glm::vec3 computeLinearGradientModel(RenderState& state, const glm::vec3& cameraDirection, const glm::vec3& lightDirection, const glm::vec3& lightColor, const HitInfo& hitInfo, const LinearGradient& gradient)
{
    float cos_theta = glm::dot(glm::normalize(lightDirection), glm::normalize(hitInfo.normal));
    glm::vec3 sampledColor = gradient.sample(cos_theta);
    return lightColor * sampledColor;
}