#include "Rendering/Scene/ModelBone.h"
#include "Rendering/Scene/Transform.h"
#include "Rendering/DirectXRenderer.h"
#include "Rendering/RHIStaticStates.h"
#include "Rendering/Scene/Geometry.h"
#include "Rendering/Renderer3D.h"
ModelBone::ModelBone(const char* name, const char* icon, std::weak_ptr<Entity> entity) : Component(name, icon, entity)
{
}
void ModelBone::Initialize(const VirtualNode* virtualNode, EditBone& editBone)
{
    static std::vector<VertexPosition> boneVertices;
    static std::vector<unsigned short> boneIndices;
    static std::vector<VertexPositionNormal> sphereVertices;
    static std::vector<unsigned short> sphereIndices;
    if (boneVertices.empty())
    {
        Geometry::CreateOctahedralBone(boneVertices, boneIndices);
        Geometry::CreateSphere(sphereVertices, sphereIndices, 0.05f);
    }
    std::shared_ptr<Mesh> head = meshes[0];
    std::shared_ptr<Mesh> tail = meshes[1];
    std::shared_ptr<Mesh> bone = meshes[2];
    head->Initialize(sphereVertices, sphereIndices, Renderer3D::Shaders::SimpleVertex, Renderer3D::Shaders::SimplePixel, Vector3(0.721f, 0.709f, 0.709f));
    tail->Initialize(sphereVertices, sphereIndices, Renderer3D::Shaders::SimpleVertex, Renderer3D::Shaders::SimplePixel, Vector3(0.721f, 0.709f, 0.709f));
    bone->Initialize(boneVertices, boneIndices, Renderer3D::Shaders::SimpleVertex, Renderer3D::Shaders::SimplePixel, Vector3(0.721f, 0.709f, 0.709f));
    editBone.headPosition = virtualNode->armatureMatrix * SVector3(0, 0, 0);
    editBone.tailPosition = virtualNode->armatureMatrix * SVector3(0, 1, 0);
    SetLength(editBone, virtualNode->boneLength);
    AlignRoll(editBone, virtualNode->armatureMatrix * SVector3(0, 0, 1) - editBone.headPosition);
    armatureHeadPosition = editBone.headPosition;
    armatureTailPosition = editBone.tailPosition;
    armatureRoll = editBone.roll;
    roll = 0.f;
    if (virtualNode->parent)
    {
        ModelBone* parent = this->parent.lock().get();
        SMatrix44 inverseParentArmatureMatrix = parent->armatureMatrix.Inverted();
        headPosition = armatureHeadPosition - parent->armatureTailPosition;
        tailPosition = armatureTailPosition - parent->armatureTailPosition;
        headPosition = SMatrix33(inverseParentArmatureMatrix).Transposed() * headPosition;
        tailPosition = SMatrix33(inverseParentArmatureMatrix).Transposed() * tailPosition;
    }
    else
    {
        headPosition = armatureHeadPosition;
        tailPosition = armatureTailPosition;
    }
    CalculateArmatureBoneMatrix(virtualNode);
    SMatrix33 preMat = ConvertArmatureBoneToMatrix(editBone);
    SMatrix33 inverseMat = preMat.Inverted();
    SMatrix33 postMat = SMatrix33(armatureMatrix);
    SMatrix33 difMat = postMat * inverseMat;
    roll = -atan2f(difMat[2][0], difMat[2][2]);
    CalculateArmatureBoneMatrix(virtualNode);
    SVector3 position;
    SQuaternion rotation;
    SVector3 scale;
    virtualNode->GetTransform(position, rotation, scale);
    poseBonePosition = virtualNode->editBoneRotation.Conjugated() * (position - virtualNode->editBonePosition);
    poseBoneRotation = virtualNode->editBoneRotation.Conjugated() * rotation;
    poseBoneScale = scale;
    SMatrix44 channelMatrix = CalculateChannelMatrix();
    poseMatrix = ArmatureBoneMatrixToPoseMatrix(channelMatrix);
    SVector3 headPosition;
    SQuaternion headRotation;
    SVector3 headScale;
    poseMatrix.Decompose(headPosition, headRotation, headScale);
    SMatrix44 poseMatrix2 = poseMatrix;
    SVector3 tailPosition;
    SQuaternion tailRotation;
    SVector3 tailScale;
    poseMatrix2.Rescale(SVector3(length, length, length));
    poseMatrix2.Translate(SVector3(0.0f, 1.0f, 0.0f));
    poseMatrix2.Decompose(tailPosition, tailRotation, tailScale);
    head->GetTransform()->SetLocalPosition(headPosition);
    bone->GetTransform()->SetLocalPosition(headPosition);
    tail->GetTransform()->SetLocalPosition(tailPosition);
    bone->GetTransform()->SetLocalRotation(headRotation);
    head->GetTransform()->SetLocalScale(length);
    bone->GetTransform()->SetLocalScale(length);
    tail->GetTransform()->SetLocalScale(length);
    meshes[0]->GetTransform()->SetParent(GetTransform());
    meshes[1]->GetTransform()->SetParent(GetTransform());
    meshes[2]->GetTransform()->SetParent(GetTransform());
    GetTransform()->AddChild(meshes[0]->GetTransform());
    GetTransform()->AddChild(meshes[1]->GetTransform());
    GetTransform()->AddChild(meshes[2]->GetTransform());
}
const float ModelBone::GetLength() const
{
    return length;
}
void ModelBone::SetLength(EditBone& editBone, const float length)
{
    editBone.length = length;
    SVector3 delta = editBone.tailPosition - editBone.headPosition;
    float length2 = 0;
    delta.Normalize(length2);
    if (length2 == 0.f)
    {
        delta[2] = 1.0f;
    }
    editBone.tailPosition = editBone.headPosition + delta * length;
}
void ModelBone::AlignRoll(EditBone& editBone, const SVector3& no)
{
    editBone.roll = CalculateRoll(editBone, no, false);
}
float ModelBone::CalculateRoll(EditBone& editBone, const SVector3& alignAxis, const bool axisOnly)
{
    float roll = 0.f;
    SVector3 nor = editBone.tailPosition - editBone.headPosition;
    float length = 0;
    nor.Normalize(length);
    if (length <= FLT_EPSILON || (fabsf(SVector3::Dot(alignAxis, nor)) >= (1.0f - FLT_EPSILON)))
    {
        return roll;
    }
    SMatrix33 mat = RollVectorToNormalizedMatrix(nor, 0.0f);
    SVector3 vec = SVector3::ProjectNormalized(alignAxis, nor);
    SVector3 alignAxisProj = alignAxis - vec;
    if (axisOnly)
    {
        if (SVector3::Angle(alignAxisProj, mat[2]) > (float)(M_PI_2))
        {
            alignAxisProj.Negate();
        }
    }
    roll = SVector3::Angle(alignAxisProj, mat[2]);
    vec = SVector3::Cross(mat[2], alignAxisProj);
    if (SVector3::Dot(vec, nor) < 0.0f)
    {
        return -roll;
    }
    return roll;
}
void ModelBone::AddMesh(std::shared_ptr<Mesh> mesh)
{
    meshes.push_back(mesh);
}
void ModelBone::Render()
{
}
void ModelBone::RenderProperties()
{
}
SMatrix44 ModelBone::GetBoneOffsetMatrix()
{
    ModelBone* parent = this->parent.lock().get();
    SMatrix44 boneOffsetMatrix = SMatrix44(boneMatrix);
    boneOffsetMatrix[3][0] = headPosition.x;
    boneOffsetMatrix[3][1] = headPosition.y;
    boneOffsetMatrix[3][2] = headPosition.z;
    boneOffsetMatrix[3][1] += parent->length;
    return boneOffsetMatrix;
}
void ModelBone::SetParent(std::weak_ptr<ModelBone> parent)
{
    this->parent = parent;
}
SMatrix44 ModelBone::CalculateChannelMatrix()
{
    SMatrix33 scaleMatrix = SMatrix33::ScaleToMatrix(poseBoneScale);
    SQuaternion quaternion = poseBoneRotation.Normalized();
    SMatrix33 rotationMatrix = quaternion.ToMatrix();
    SMatrix44 channelMatrix = scaleMatrix * rotationMatrix;
    channelMatrix[3][0] = poseBonePosition.x;
    channelMatrix[3][1] = poseBonePosition.y;
    channelMatrix[3][2] = poseBonePosition.z;
    return channelMatrix;
}
SMatrix44 ModelBone::ArmatureBoneMatrixToPoseMatrix(const SMatrix44& channelMatrix)
{
    SMatrix44 rotationScaleMatrix;
    SMatrix44 positionMatrix;
    SVector3 postScale = SVector3(1.f, 1.f, 1.f);
    CalculateParentBoneTransform(rotationScaleMatrix, positionMatrix);
    SVector3 position = SVector3(channelMatrix[3][0], channelMatrix[3][1], channelMatrix[3][2]);
    SVector3 newPosition = positionMatrix.Transposed() * position;
    SMatrix44 poseMatrix = channelMatrix * rotationScaleMatrix;
    poseMatrix[3][0] = newPosition.x;
    poseMatrix[3][1] = newPosition.y;
    poseMatrix[3][2] = newPosition.z;
    poseMatrix.Rescale(postScale);
    return poseMatrix;
}
void ModelBone::CalculateParentBoneTransform(SMatrix44& rotationScaleMatrix, SMatrix44& positionMatrix)
{
    ModelBone* parent = this->parent.lock().get();
    if (parent)
    {
        SMatrix44 boneOffsetMatrix = GetBoneOffsetMatrix();
        CalculateParentBoneTransformFromMatrices(boneOffsetMatrix, &parent->poseMatrix, rotationScaleMatrix, positionMatrix);
    }
    else
    {
        CalculateParentBoneTransformFromMatrices(armatureMatrix, nullptr, rotationScaleMatrix, positionMatrix);
    }
}
void ModelBone::CalculateParentBoneTransformFromMatrices(const SMatrix44& boneOffsetMatrix, const SMatrix44* parentPoseMatrix,
    SMatrix44& rotationScaleMatrix, SMatrix44& positionMatrix)
{
    if (parentPoseMatrix)
    {
        rotationScaleMatrix = boneOffsetMatrix * *parentPoseMatrix;
        positionMatrix = rotationScaleMatrix;
    }
    else
    {
        rotationScaleMatrix = boneOffsetMatrix;
        positionMatrix = rotationScaleMatrix;
    }
}
void ModelBone::CalculateArmatureBoneMatrix(const VirtualNode* virtualNode)
{
    ModelBone* parent = this->parent.lock().get();
    const SVector3 vector = tailPosition - headPosition;
    length = vector.Length();
    boneMatrix = RollVectorToMatrix(vector, roll);
    if (virtualNode->parent)
    {
        SMatrix44 boneOffsetMatrix = GetBoneOffsetMatrix();
        armatureMatrix = boneOffsetMatrix * parent->armatureMatrix;
    }
    else
    {
        armatureMatrix = boneMatrix;
        armatureMatrix[3][0] = headPosition.x;
        armatureMatrix[3][1] = headPosition.y;
        armatureMatrix[3][2] = headPosition.z;
    }
}
SMatrix33 ModelBone::ConvertArmatureBoneToMatrix(EditBone& editBone)
{
    SVector3 delta = editBone.tailPosition - editBone.headPosition;
    float roll = editBone.roll;
    float length = 0;
    delta.Normalize(length);
    if (!length)
    {
        if (editBone.parent)
        {
            delta = editBone.parent->tailPosition - editBone.parent->headPosition;
            SVector3::Normalize(delta);
            roll = editBone.parent->roll;
        }
    }
    return RollVectorToNormalizedMatrix(delta, roll);
}
SMatrix33 ModelBone::RollVectorToNormalizedMatrix(const SVector3& nor, const float roll)
{
    const float SAFE_THRESHOLD = 6.1e-3f;
    const float CRITICAL_THRESHOLD = 2.5e-4f;
    const float THRESHOLD_SQUARED = CRITICAL_THRESHOLD * CRITICAL_THRESHOLD;
    const float x = nor[0];
    const float y = nor[1];
    const float z = nor[2];
    float theta = 1.0f + y;
    const float thetaAlt = x * x + z * z;
    SMatrix33 rMatrix;
    SMatrix33 bMatrix;
    if (theta > SAFE_THRESHOLD || thetaAlt > THRESHOLD_SQUARED)
    {
        bMatrix[0][1] = -x;
        bMatrix[1][0] = x;
        bMatrix[1][1] = y;
        bMatrix[1][2] = z;
        bMatrix[2][1] = -z;
        if (theta <= SAFE_THRESHOLD)
        {
            theta = thetaAlt * 0.5f + thetaAlt * thetaAlt * 0.125f;
        }
        bMatrix[0][0] = 1 - x * x / theta;
        bMatrix[2][2] = 1 - z * z / theta;
        bMatrix[2][0] = bMatrix[0][2] = -x * z / theta;
    }
    else
    {
        bMatrix.Unit();
        bMatrix[0][0] = bMatrix[1][1] = -1.0;
    }
    rMatrix = SMatrix33::NormalizedAxisAngleToMatrix(nor, roll);
    return bMatrix * rMatrix;
}
SMatrix33 ModelBone::RollVectorToMatrix(const SVector3& vector, const float roll)
{
    const SVector3 nor = SVector3::Normalize(vector);
    return RollVectorToNormalizedMatrix(nor, roll);
}