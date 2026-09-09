#pragma once
#include <object/baseObject2d/BaseObject2d.h>
#include <vector>
#include <memory>

// 回転床の種類
enum class RotatingFloorType
{
	kRight = 0,      // 右回転（時計回り）
	kLeft = 1,       // 左回転（反時計回り）
	Count
};

class RotatingFloor : public BaseObject2d
{
public:
	RotatingFloor();
	~RotatingFloor() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

	/// <summary>
	/// 回転床の種類を設定
	/// </summary>
	void SetRotatingFloorType(RotatingFloorType type);
	RotatingFloorType GetRotatingFloorType() const { return rotatingFloorType_; }

	/// <summary>
	/// 同じマスに新しく乗った中継器を検知し、一度だけ90度回転させる (回転が発生した場合はtrueを返す)
	/// </summary>
	/// <param name="objects">全オブジェクトリスト</param>
	/// <returns>回転が行われたかどうか</returns>
	bool CheckAndRotateRepeater(const std::vector<std::unique_ptr<BaseObject2d>>& objects);

private:
	RotatingFloorType rotatingFloorType_ = RotatingFloorType::kRight;
	BaseObject2d* pOccupyingRepeater_ = nullptr; // 前フレームでこの床の上にいた中継器
};
