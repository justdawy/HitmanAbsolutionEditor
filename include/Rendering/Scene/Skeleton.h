#pragma once
#include "Component.h"
#include "ModelBone.h"
#include "Transform.h"
#include "VirtualNode.h"
class Skeleton : public Component
{
public:
	Skeleton(const char* name, const char* icon, std::weak_ptr<Entity> entity);
	void Initialize(std::shared_ptr<BoneRig> boneRig);
	void InitializeVirtualNodes(const unsigned int parentIndex, std::vector<VirtualNode>& virtualNodes, std::shared_ptr<BoneRig> boneRig);
	void AddModelBone(std::shared_ptr<ModelBone> modelBone);
	void Render() override;
	void RenderProperties() override;
	void PrettifyBones(VirtualNode* virtualNode, const std::shared_ptr<SQuaternion> parentBoneRotation = nullptr);
	float PickBoneLength(const VirtualNode* virtualNode);
	std::shared_ptr<SQuaternion> PickBoneRotation(const VirtualNode* virtualNode, const std::shared_ptr<SQuaternion> parentBoneRotation);
	std::shared_ptr<SQuaternion> Temperance(const VirtualNode* virtualNode, const std::shared_ptr<SQuaternion> parentBoneRotation);
	void RotateBone(VirtualNode* virtualNode, const std::shared_ptr<SQuaternion> rotation);
	void LocalRotation(VirtualNode* virtualNode, const std::shared_ptr<SQuaternion> rotation);
	void CalculateBoneMatrices(VirtualNode* virtualNode);
	void ConvertPosition(SVector3& vector);
	void ConvertRotation(SQuaternion& SQuaternion);
	void ConvertMatrix(SMatrix44& matrix);
private:
	enum class BoneHeuristic
	{
		Blender,
		Temperance,
		Fortune
	};
	std::vector<std::shared_ptr<ModelBone>> modelBones;
	inline static const float minBoneLength = 0.004f;
	BoneHeuristic boneHeuristic = BoneHeuristic::Temperance;
};