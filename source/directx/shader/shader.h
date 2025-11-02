#pragma once

class Shader
{
public:
    bool Load(const wchar_t* filepath, const char* entryPoint, const char* target);
    ID3DBlob* GetBlob() const { return m_Blob.Get(); }
private:
    Microsoft::WRL::ComPtr<ID3DBlob> m_Blob;
};