/**
 * @file OutputBuffer.hpp
 * @brief Acts as a relay between the program and the STDOUT stream.
 * @author radj307
 */
#pragma once
#include <cassert>
#include <ostream>
#include <vector>


namespace caco_alch::output {
	struct Indent {
	private:
		std::streamoff _count;
	
	public:
		enum class AlignTo {
			LEFT,
			RIGHT
		};
		AlignTo _align{ AlignTo::LEFT };
		
		explicit Indent(std::streamoff count) : _count{ std::move(count) } {}
		
		void reset(std::streamoff count)
		{
			_count = std::move(count);
		}

		friend std::ostream& operator<<(std::ostream& os, const Indent& obj)
		{
			switch (obj._align) {
			case AlignTo::LEFT:
				os << std::left;
				break;
			case AlignTo::RIGHT:
				os << std::right;
				break;
			}
			os << std::setw(obj._count);
			return os;
		}
	};
	/**
	 * @struct center
	 * @brief Designed to be used inline in stream insertion statements, this struct will center a given string given a max bound.
	 */
	struct center {
		const std::string _str;
		const std::string::size_type _target_size;

		/**
		 * @brief Default Constructor
		 * @param str			- 
		 * @param target_size	- 
		 */
		center(const std::string& str, const std::string::size_type target_size) : _str{ str }, _target_size{ target_size / 2u - str.size() / 2u }
		{
			if ( target_size < str.size() )
				throw std::exception("center(): Cannot center string; str.size() > target_size");
		}

		~center() = default;
		center(center&&) = delete;
		center(const center&) = delete;
		center& operator=(center&&) = delete;
		center& operator=(const center&) = delete;

		[[nodiscard]] auto size() const
		{
			return _str.size() + _target_size * 2;
		}
		
		friend std::ostream& operator<<(std::ostream& os, const center& obj)
		{
			const auto indent{ std::string(obj._target_size, ' ') };
			os << indent << obj._str << indent;
			return os;
		}
	};
}
