#include "IO/InputMemoryStream.h"
InputMemoryStream::InputMemoryStream() : buffer(nullptr), size(0), position(0)
{
}
InputMemoryStream::InputMemoryStream(void* buffer, const size_t dataSize) : buffer(buffer), size(dataSize), position(0)
{
}
const void* InputMemoryStream::GetBuffer(bool seekToCurrentPosition) const
{
	if (seekToCurrentPosition)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(buffer) + position);
	}
	return buffer;
}
void* InputMemoryStream::GetBuffer(bool seekToCurrentPosition)
{
	if (seekToCurrentPosition)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(buffer) + position);
	}
	return buffer;
}
const size_t InputMemoryStream::GetPosition()
{
	return position;
}
const size_t InputMemoryStream::GetSize() const
{
	return size;
}
std::string InputMemoryStream::ReadString(const char delimiter)
{
	char c;
	std::string result;
	while (position < size)
	{
		c = Read<char>();
		if (c == delimiter) break;
		result += c;
	}
	return result;
}
std::string InputMemoryStream::ReadString(const size_t readSize, const bool isNullTerminated)
{
	size_t size2 = readSize;
	if (isNullTerminated)
	{
		++size2;
	}
	if (position + size2 > size) throw std::out_of_range("Read out of bounds");
	char* outBuffer = new char[size2];
	memcpy(outBuffer, reinterpret_cast<const char*>(this->buffer) + position, size2);
	position += size2;
	std::string result = std::string(outBuffer, readSize);
	delete[] outBuffer;
	return result;
}
void InputMemoryStream::Skip(const size_t count)
{
	if (position + count > size) throw std::out_of_range("Seek out of bounds");
	position += count;
}
void InputMemoryStream::Seek(const size_t offset, const SeekOrigin seekOrigin)
{
	size_t newPosition = position;
	switch (seekOrigin)
	{
	case SeekOrigin::Begin:
		newPosition = offset;
		break;
	case SeekOrigin::Current:
		newPosition = position + offset;
		break;
	case SeekOrigin::End:
		if (offset > size) throw std::out_of_range("Seek out of bounds");
		newPosition = size - offset;
		break;
	}
	if (newPosition > size) throw std::out_of_range("Seek out of bounds");
	position = newPosition;
}
void InputMemoryStream::AlignTo(const unsigned char alignment)
{
}