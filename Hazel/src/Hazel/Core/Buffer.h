#pragma once

namespace Hazel
{
	// Non-Owning raw buffer class.
	struct Buffer
	{
		uint8_t* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;

		Buffer(uint64_t size)
		{
			Allocate(size);
		}

		Buffer(const Buffer&) = default;

		static Buffer Copy(const Buffer& other)
		{
			const Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

		void Allocate(uint64_t size)
		{
			Release();

			Data = new uint8_t[size];
			Size = size;
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		template<typename T>
		T* As()
		{
			return reinterpret_cast<T*>(Data);
		}

		operator bool() const
		{
			return static_cast<bool>(Data);
		}
	};

	struct ScopedBuffer
	{
		ScopedBuffer(Buffer buffer)
			: _buffer(buffer) {}

		ScopedBuffer(uint64_t size)
			: _buffer(size) {}

		~ScopedBuffer()
		{
			_buffer.Release();
		}

		uint8_t* Data() const { return _buffer.Data; }
		uint64_t Size() const { return _buffer.Size; }

		template<typename T>
		T* As()
		{
			return _buffer.As<T>();
		}

		operator bool() const
		{
			return _buffer;
		}

	private:
		Buffer _buffer;
	};
}
