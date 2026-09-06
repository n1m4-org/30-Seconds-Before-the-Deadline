#include "PC.h"
#include <config/ResourcePath.h>
#include <cmath>
#include <algorithm>
#include <Features/DeltaTimeManager/DeltaTimeManager.h>

PC::PC()
{
	objectType_ = ObjectType2d::kPC;
	isDynamic_ = false;
}

PC::~PC()
{
}

void PC::Initialize()
{
	objectType_ = ObjectType2d::kPC;
	isDynamic_ = false;
	dataProgress_ = 0.0f;
	isCleared_ = false;

	pSprite_ = std::make_unique<Sprite>();
	pSprite_->Initialize(Path::Image::InGame::kPC);
	//pSprite_->SetColor({ 0.8f, 0.3f, 0.9f, 1.0f }); // 仮置き：紫色
}

void PC::UpdateSignal(bool isReceived, int strength)
{
	isSignalReceived_ = isReceived;
	signalStrength_ = strength;
}

void PC::Update()
{
	BaseObject2d::Update();

	if (pSprite_)
	{
		float dt = 1.0f / 60.0f;
		try
		{
			dt = DeltaTimeManager::GetInstance()->GetDeltaTime(static_cast<uint32_t>(DeltaTimeChannelReserved::Game));
		}
		catch (...)
		{
			dt = 1.0f / 60.0f;
		}

		// 電波が届いている場合、強度に応じた速度でデータ転送蓄積
		// 強度10 -> 1秒, 強度9 -> 2秒 ... 強度1 -> 10秒 (所要時間 = 11 - strength 秒)
		if (isSignalReceived_ && signalStrength_ > 0 && !isCleared_)
		{
			float requiredSeconds = 11.0f - static_cast<float>(signalStrength_);
			if (requiredSeconds < 1.0f) requiredSeconds = 1.0f;

			float fillSpeed = (1.0f / requiredSeconds) * dt;
			dataProgress_ += fillSpeed;

			if (dataProgress_ >= 1.0f)
			{
				dataProgress_ = 1.0f;
				isCleared_ = true; // クリア達成！
			}
		}


		blinkTimer_ += 4.8f * dt; // 0.08f * 60fps 相当
		float factor = 0.5f + 0.5f * std::sin(blinkTimer_ * 3.5f);
		float easeFactor = factor * factor * (3.0f - 2.0f * factor); // Smoothstep イージング

		Vector4 purpleColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 基本色：紫

		// 1. クリア達成時 (100% 蓄積完了): 紫 ↔ 黄色 の点滅
		if (isCleared_)
		{
			Vector4 yellowTarget = { 0.2f, 1.0f, 0.1f, 1.0f };
			Vector4 color = {
				purpleColor.x + (yellowTarget.x - purpleColor.x) * easeFactor,
				purpleColor.y + (yellowTarget.y - purpleColor.y) * easeFactor,
				purpleColor.z + (yellowTarget.z - purpleColor.z) * easeFactor,
				1.0f
			};
			pSprite_->SetColor(color);
		}
		// 2. 電波受信中 (チャージ中): 紫 ↔ 白 の点滅
		else if (isSignalReceived_)
		{
			Vector4 whiteTarget = { 1.0f, 0.9f, 0.1f, 1.0f } ;
			Vector4 color = {
				purpleColor.x + (whiteTarget.x - purpleColor.x) * easeFactor,
				purpleColor.y + (whiteTarget.y - purpleColor.y) * easeFactor,
				purpleColor.z + (whiteTarget.z - purpleColor.z) * easeFactor,
				1.0f
			};
			pSprite_->SetColor(color);
		}
		// 3. 電波未到達時: 通常の紫色
		else
		{
			pSprite_->SetColor(purpleColor);
		}

	}
}

void PC::Draw()
{
	BaseObject2d::Draw();
}



