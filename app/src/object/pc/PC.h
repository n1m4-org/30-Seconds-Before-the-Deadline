#pragma once
#include <object/baseObject2d/BaseObject2d.h>

class PC : public BaseObject2d
{
public:
	PC();
	~PC() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

	// 電波受信状態と強度の設定 (強度 1~10 に応じてデータ進捗が蓄積)
	void UpdateSignal(bool isReceived, int strength);

	float GetDataProgress() const { return dataProgress_; }
	bool IsCleared() const { return isCleared_; }
	void ResetProgress() { dataProgress_ = 0.0f; isCleared_ = false; }
	void SetProgress(float progress, bool isCleared) { dataProgress_ = progress; isCleared_ = isCleared; }

	// PCは向きによる回転を行わない（常に正立）
	void ApplyRotationToSprite() override { ResetRotation(); }

private:
	bool isSignalReceived_ = false;
	int signalStrength_ = 0;
	float dataProgress_ = 0.0f; // 0.0f ～ 1.0f (100%)
	bool isCleared_ = false;
	float blinkTimer_ = 0.0f;
};


