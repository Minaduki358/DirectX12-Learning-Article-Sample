#pragma once

class Material
{
public:
	Material();
	virtual ~Material();

	/// <summary>
	/// パイプラインをセットする
	/// </summary>
	bool SetPipeline();

protected:
	std::string m_Pipelinename;
};
