#include "extra.h"
#include "bvh.h"
#include "light.h"
#include "recursive.h"
#include "shading.h"
#include <framework/trackball.h>
#include "texture.h"
#include <iostream>

// TODO; Extra feature
// Given the same input as for `renderImage()`, instead render an image with your own implementation
// of Depth of Field. Here, you generate camera rays s.t. a focus point and a thin lens camera model
// are in play, allowing objects to be in and out of focus.
// This method is not unit-tested, but we do expect to find it **exactly here**, and we'd rather
// not go on a hunting expedition for your implementation, so please keep it here!
void renderImageWithDepthOfField(const Scene& scene, const BVHInterface& bvh, const Features& features, const Trackball& camera, Screen& screen)
{
    if (!features.extra.enableDepthOfField) {
        return;
    }

    // ...
}

// TODO; Extra feature
// Given the same input as for `renderImage()`, instead render an image with your own implementation
// of motion blur. Here, you integrate over a time domain, and not just the pixel's image domain,
// to give objects the appearance of "fast movement".
// This method is not unit-tested, but we do expect to find it **exactly here**, and we'd rather
// not go on a hunting expedition for your implementation, so please keep it here!
void renderImageWithMotionBlur(const Scene& scene, const BVHInterface& bvh, const Features& features, const Trackball& camera, Screen& screen)
{
    if (!features.extra.enableMotionBlur) {
        return;
    }

}

// TODO; Extra feature
// Given a rendered image, compute and apply a bloom post-processing effect to increase bright areas.
// This method is not unit-tested, but we do expect to find it **exactly here**, and we'd rather
// not go on a hunting expedition for your implementation, so please keep it here!
void postprocessImageWithBloom(const Scene& scene, const Features& features, const Trackball& camera, Screen& image)
{
    if (!features.extra.enableBloomEffect) {
        return;
    }

    // ...
}


// TODO; Extra feature
// Given a camera ray (or reflected camera ray) and an intersection, evaluates the contribution of a set of
// glossy reflective rays, recursively evaluating renderRay(..., depth + 1) along each ray, and adding the
// results times material.ks to the current intersection's hit color.
// - state;    the active scene, feature config, bvh, and sampler
// - ray;      camera ray
// - hitInfo;  intersection object
// - hitColor; current color at the current intersection, which this function modifies
// - rayDepth; current recursive ray depth
// This method is not unit-tested, but we do expect to find it **exactly here**, and we'd rather
// not go on a hunting expedition for your implementation, so please keep it here!
void renderRayGlossyComponent(RenderState& state, Ray ray, const HitInfo& hitInfo, glm::vec3& hitColor, int rayDepth)
{
    // Generate an initial specular ray, and base secondary glossies on this ray
    // auto numSamples = state.features.extra.numGlossySamples;
    // ...
}

// TODO; Extra feature
// Given a camera ray (or reflected camera ray) that does not intersect the scene, evaluates the contribution
// along the ray, originating from an environment map. You will have to add support for environment textures
// to the Scene object, and provide a scene with the right data to supply this.
// - state; the active scene, feature config, bvh, and sampler
// - ray;   ray object
// This method is not unit-tested, but we do expect to find it **exactly here**, and we'd rather
// not go on a hunting expedition for your implementation, so please keep it here!
glm::vec3 sampleEnvironmentMap(RenderState& state, Ray ray)
{
    if (state.features.extra.enableEnvironmentMap) {
        float x = std::fabs(ray.direction.x), y = std::fabs(ray.direction.y), z = std::fabs(ray.direction.z);
        float maxComponent = std::max(x, std::max(y, z));
        glm::vec3 r = ray.direction / maxComponent; // [-1, 1]
        glm::vec3 coords = (r + glm::vec3(1.0f)) / 2.0f; // [0, 1]

        // +- 1 => choose face
        // take other 2 coords and sample from face
        float one = 1.0f - FLT_EPSILON;
        float u, v;
        
        /* texture is 4 squares wide and 3 squares:

                   UP
            LEFT FRONT RIGHT BACK
                  DOWN
        */
        // Some 'coords' components need to be flipped to account for the way a cube is unfolded onto a flat plane (some faces are inverted)
        if (r.x > one) { // right
            u = coords.z / 4.0f + 2.0f / 4.0f;
            v = coords.y / 3.0f + 1.0f / 3.0f;
        } else if (r.x < -one) { // left
            u = (1 - coords.z) / 4.0f;
            v = coords.y / 3.0f + 1.0f / 3.0f;
        } else if (r.y > one) { // up
            u = coords.x / 4.0f + 1.0f / 4.0f;
            v = coords.z / 3.0f + 2.0f / 3.0f;
        } else if (r.y < -one) { // down
            u = coords.x / 4.0f + 1.0f / 4.0f;
            v = (1 - coords.z) / 3.0f;
        } else if (r.z < -one) { // front
            u = coords.x / 4.0f + 1.0f / 4.0f;
            v = coords.y / 3.0f + 1.0f / 3.0f;
        } else /*if (r.z > one)*/ { // back
            u = (1 - coords.x) / 4.0f + 3.0f / 4.0f;
            v = coords.y / 3.0f + 1.0f / 3.0f;
        }
        if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
            std::cout << "OUT OF BOUNDS\n";
        glm::vec2 mapTexCoords = glm::vec2(u, v);

        if (state.features.enableBilinearTextureFiltering) 
            return sampleTextureBilinear(state.scene.environmentMap, mapTexCoords);

        return sampleTextureNearest(state.scene.environmentMap, mapTexCoords);

    } else {
        return glm::vec3(0.f);
    }
}


// TODO: Extra feature
// As an alternative to `splitPrimitivesByMedian`, use a SAH+binning splitting criterion. Refer to
// the `Data Structures` lecture for details on this metric.
// - aabb;       the axis-aligned bounding box around the given triangle set
// - axis;       0, 1, or 2, determining on which axis (x, y, or z) the split must happen
// - primitives; the modifiable range of triangles that requires splitting
// - return;     the split position of the modified range of triangles
// This method is unit-tested, so do not change the function signature.
size_t splitPrimitivesBySAHBin(const AxisAlignedBox& aabb, uint32_t axis, std::span<BVH::Primitive> primitives)
{
    using Primitive = BVH::Primitive;

    return 0; // This is clearly not the solution
}