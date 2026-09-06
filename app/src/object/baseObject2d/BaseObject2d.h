#pragma once
#include <drawable/sprite/Sprite.h>
#include <memory>

struct Vector2Int
{
	int x = 0;
	int y = 0;

	Vector2Int operator+(const Vector2Int& other) const
	{
		return { x + other.x, y + other.y };
	}

	Vector2Int operator-(const Vector2Int& other) const
	{
		return { x - other.x, y - other.y };
	}

	bool operator==(const Vector2Int& other) const
	{
		return x == other.x && y == other.y;
	}

	bool operator!=(const Vector2Int& other) const
	{
		return !(*this == other);
	}
}; 

enum class ObjectType2d
{
	None = 0,
	kPlayer,
	kRouter,
	kRepeater,
	kAlumiWall,
	kPC,
	Count
};

class BaseObject2d
{
public:
    BaseObject2d();
    virtual ~BaseObject2d();

    virtual void Initialize() = 0;
    virtual void Update();
    virtual void Draw();

	// 移動制御
	virtual void Move(const Vector2Int& delta);
	void SetPosition(const Vector2Int& position);
	void SetAngle(const Vector2Int& angle) { angle_ = angle; }
	void SetSize(const Vector2& size) { size_ = size; }

	// スプライトの画面上の位置をマス目（tileSize / mapOffset）に合わせて更新
	virtual void UpdateSpritePosition(float tileSize, const Vector2& mapOffset);

	const Vector2Int& GetPosition() const { return position_; }
	const Vector2Int& GetAngle() const { return angle_; }
	const Vector2& GetSize() const { return size_; }
	const ObjectType2d& GetObjectType() const { return objectType_; }
	bool IsDynamic() const { return isDynamic_; }
	bool IsMoving() const { return isMoving_; }

	// 向きベクトル(angle_)から回転ラジアン角を取得
	float GetRotationAngleRad() const
	{
		if (angle_.x == 0 && angle_.y == -1) return 0.0f;               // 上
		if (angle_.x == 1 && angle_.y == 0)  return 1.5707963f;        // 右 (90 deg)
		if (angle_.x == 0 && angle_.y == 1)  return 3.14159265f;       // 下 (180 deg)
		if (angle_.x == -1 && angle_.y == 0) return -1.5707963f;       // 左 (-90 deg)
		return 0.0f;
	}

	virtual void ApplyRotationToSprite()
	{
		if (pSprite_)
		{
			float rotationRad = GetRotationAngleRad();
			pSprite_->SetRotation(rotationRad);
		}
	}

	void ResetRotation()
	{
		if (pSprite_)
		{
			pSprite_->SetRotation(0.0f);
		}
	}


protected:
    std::unique_ptr<Sprite> pSprite_ = nullptr;
	Vector2Int position_ = { 0, 0 };
	Vector2Int targetPosition_ = { 0, 0 };
	Vector2Int angle_ = { 0, -1 };
	Vector2 size_ = { 100.0f, 100.0f };
	ObjectType2d objectType_ = ObjectType2d::None;
	bool isDynamic_ = false;

	// アニメーション移動用
	bool isMoving_ = false;
	float moveProgress_ = 0.0f;
	float moveSpeed_ = 8.0f; // 1秒あたりの補間速度倍率など
	Vector2 currentRenderPos_ = { 0.0f, 0.0f };
	Vector2 targetRenderPos_ = { 0.0f, 0.0f };
};


