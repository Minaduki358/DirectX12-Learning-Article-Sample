#include "shader.h"

using namespace Microsoft::WRL;

bool Shader::Load(const wchar_t* filepath, const char* entryPoint, const char* target)
{
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT result = D3DCompileFromFile(
		filepath,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルードはデフォルト
		entryPoint,
		target,
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用および最適化なし
		0,
		m_Blob.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf()); // エラー時はerrorBlob にメッセージが入る

	if (FAILED(result))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		return false;
	}

	return true;
}
