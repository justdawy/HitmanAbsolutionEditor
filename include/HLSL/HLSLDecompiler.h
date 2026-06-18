#pragma once
#include <Windows.h>
#include <D3Dcompiler.h>
#include <HLSLDecompiler/DecompileHLSL.h>
#include <log.h>
#include <version.h>
static HRESULT DisassembleMS(const void* pShaderBytecode, size_t BytecodeLength, std::string* asmText)
{
	ID3DBlob* disassembly = nullptr;
	UINT flags = D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS;
	std::string comments = "//   using 3Dmigoto command line v" + std::string(VER_FILE_VERSION_STR) + " on " + LogTime() + "//\n";
	HRESULT hr = D3DDisassemble(pShaderBytecode, BytecodeLength, flags, comments.c_str(), &disassembly);
	if (FAILED(hr))
	{
		LogInfo("  disassembly failed. Error: %x\n", hr);
		return hr;
	}
	*asmText = std::string(static_cast<char*>(disassembly->GetBufferPointer()));
	disassembly->Release();
	return S_OK;
}
static HRESULT Decompile(const void* pShaderBytecode, size_t BytecodeLength, string* hlslText, string* shaderModel)
{
	ParseParameters p = { 0 };
	DecompilerSettings d;
	bool patched = false;
	bool errorOccurred = false;
	std::string disassembly;
	HRESULT hret;
	hret = DisassembleMS(pShaderBytecode, BytecodeLength, &disassembly);
	if (FAILED(hret))
		return E_FAIL;
	LogInfo("    creating HLSL representation\n");
	p.bytecode = pShaderBytecode;
	p.decompiled = disassembly.c_str();
	p.decompiledSize = disassembly.size();
	p.G = &d;
	d.IniParamsReg = -1;
	d.StereoParamsReg = -1;
	*hlslText = DecompileBinaryHLSL(p, patched, *shaderModel, errorOccurred);
	if (!hlslText->size() || errorOccurred)
	{
		LogInfo("    error while decompiling\n");
		return E_FAIL;
	}
	return S_OK;
}