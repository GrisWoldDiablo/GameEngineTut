#include <utility>


#pragma once

namespace Hazel
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:  return  4;
		case ShaderDataType::Float2: return  4 * 2;
		case ShaderDataType::Float3: return  4 * 3;
		case ShaderDataType::Float4: return  4 * 4;
		case ShaderDataType::Mat3:   return  4 * 3 * 3;
		case ShaderDataType::Mat4:   return  4 * 4 * 4;
		case ShaderDataType::Int:    return  4;
		case ShaderDataType::Int2:   return  4 * 2;
		case ShaderDataType::Int3:   return  4 * 3;
		case ShaderDataType::Int4:   return  4 * 4;
		case ShaderDataType::Bool:   return  1;
		}
		HZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		ShaderDataType Type;
		std::string Name;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement(ShaderDataType type, std::string name, bool normalized = false)
			:Type(type), Name(std::move(name)), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Float:	return 1;
			case ShaderDataType::Float2: return 2;
			case ShaderDataType::Float3: return 3;
			case ShaderDataType::Float4: return 4;
			case ShaderDataType::Mat3:	return 3 * 3;
			case ShaderDataType::Mat4:	return 4 * 4;
			case ShaderDataType::Int:	return 1;
			case ShaderDataType::Int2:	return 2;
			case ShaderDataType::Int3:	return 3;
			case ShaderDataType::Int4:	return 4;
			case ShaderDataType::Bool:	return 1;
			}
			HZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& element)
			:_elements(element)
		{
			CalculateOffsetAndStride();
		}

		inline uint32_t GetStride() const { return _stride; }
		inline const std::vector<BufferElement>& GetElement() const { return _elements; }

		std::vector<BufferElement>::iterator begin() { return _elements.begin(); }
		std::vector<BufferElement>::iterator end() { return _elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return _elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return _elements.end(); }

	private:
		void CalculateOffsetAndStride()
		{
			uint32_t offset = 0;
			_stride = 0;
			for (auto& element : _elements)
			{
				element.Offset = offset;
				offset += element.Size;
				_stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> _elements;
		uint32_t _stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const BufferLayout& GetLayout() = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};

	// Currently only support 32-bits index buffers
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};
}
