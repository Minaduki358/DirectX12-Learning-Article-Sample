#pragma once

/// <summary>
/// 基本情報
/// </summary>
namespace SystemData
{
	// FPS
	static constexpr int k_FPS = 60;

	// ウィンドウの幅
	static constexpr int k_ScreenWidth = 960;

	// ウィンドウの高さ
	static constexpr int k_ScreenHeight = 540;

	// アスペクトレート
	static constexpr float k_AspectRate = static_cast<float>(k_ScreenWidth) / static_cast<float>(k_ScreenHeight);
}
