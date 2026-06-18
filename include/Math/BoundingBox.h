#pragma once
#include <vector>
#include "Math.h"
#include "Intersection.h"
#include "Rendering/Vertex.h"
#include "Matrix.h"
#include "Utility/Math.h"
class BoundingBox
{
public:
    BoundingBox();
    BoundingBox(const Vector3& min, const Vector3& max);
    BoundingBox(const Vector3* points, const unsigned int pointCount);
    template <typename T>
    BoundingBox(const std::vector<T>& vertices)
    {
        min = Vector3::Infinity;
        max = Vector3::InfinityNeg;
        for (unsigned int i = 0; i < vertices.size(); i++)
        {
            max.x = Math::Max(max.x, vertices[i].position.x);
            max.y = Math::Max(max.y, vertices[i].position.y);
            max.z = Math::Max(max.z, vertices[i].position.z);
            min.x = Math::Min(min.x, vertices[i].position.x);
            min.y = Math::Min(min.y, vertices[i].position.y);
            min.z = Math::Min(min.z, vertices[i].position.z);
        }
    }
    ~BoundingBox() = default;
    BoundingBox& operator=(const BoundingBox& rhs) = default;
    bool operator==(const BoundingBox& other) const;
    Vector3 GetCenter() const;
    Vector3 GetSize() const;
    Vector3 GetExtents() const;
    Intersection Intersects(const Vector3& point) const;
    Intersection Intersects(const BoundingBox& box) const;
    BoundingBox Transform(const Matrix& transform) const;
    void Merge(const BoundingBox& box);
    const Vector3& GetMin() const;
    const Vector3& GetMax() const;
    static const BoundingBox Undefined;
private:
    Vector3 min;
    Vector3 max;
};