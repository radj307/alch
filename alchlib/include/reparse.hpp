/**
 * @file reparse.hpp
 * @author radj307
 * @brief Parsing library using a recursive element type, allowing files with any number of sub-elements to be parsed.
 */
 // ReSharper disable CppUseRangeAlgorithm
#pragma once
#include <sysarch.h>
#include <fileio.hpp>
#include <fileutil.hpp>
#include <strmanip.hpp>

#include <string>
#include <optional>
#include <utility>
#include <vector>

/**
 * @namespace reparse
 * @brief Namespace containing methods for parsing data structures that allow any number of sub-sections.
 */
namespace reparse {
	/**
	 * @struct Param
	 * @brief Parameter wrapper for various reparse functions.
	 */
	struct Param final {
		std::string
			_line_delims,		///< @brief Characters treated as line breaks / line endings. Required for JSON parsing.
			_setters,			///< @brief Characters treated as setters. (ex. equals sign)
			_open_brackets,		///< @brief Characters treated as opening brackets.
			_close_brackets,	///< @brief Characters treated as closing brackets.
			_comments,			///< @brief Characters treated as line comments.
			_whitespace;		///< @brief Characters treated as whitespace.
		bool
			_fuzzy_brackets,	///< @brief When true, doesn't throw exceptions for unmatched brackets.
			_multiple_setters;	///< @brief When true, doesn't throw exceptions for multiple equals signs on the same line.

		/**
		 * @constructor Param()
		 * @brief Default constructor.
		 */
		Param() : _line_delims{ "\n;" }, _setters{ "=:" }, _open_brackets{ "{[(<" }, _close_brackets{ "}])>" }, _comments{ "" }, _whitespace{ " \t\r\n" }, _fuzzy_brackets{ false }, _multiple_setters{ false } {}
		/**
		 * @constructor Param(const std::string&, const std::string&, const std::string&, const std::string&, const bool, const bool)
		 * @brief Constructor.
		 * @param lineDelims			- Characters treated as line breaks.
		 * @param setters				- Characters treated as setters. (ex. equals sign)
		 * @param openBrackets			- Characters treated as opening brackets.
		 * @param closeBrackets			- Characters treated as closing brackets.
		 * @param comments				- Characters treated as line comments.
		 * @param whitespace			- Characters treated as whitespace.
		 * @param fuzzyBrackets			- When true, doesn't throw exceptions for unmatched brackets.
		 * @param multipleSettersOnLine	- When true, doesn't throw exceptions for multiple equals signs on the same line.
		 */
		Param(std::string lineDelims, std::string setters, std::string openBrackets, std::string closeBrackets, std::string comments, std::string whitespace, const bool fuzzyBrackets, const bool multipleSettersOnLine) : _line_delims{ std::move(lineDelims) }, _setters{ std::move(setters) }, _open_brackets{ std::move(openBrackets) }, _close_brackets{ std::move(closeBrackets) }, _comments{ std::move(comments) }, _whitespace{ std::move(whitespace) }, _fuzzy_brackets{ fuzzyBrackets }, _multiple_setters{ multipleSettersOnLine } {}

		[[nodiscard]] bool is_setter(const char c) const { return _setters.find(c) != std::string::npos; }
		[[nodiscard]] bool is_bracket_open(const char c) const { return _open_brackets.find(c) != std::string::npos; }
		[[nodiscard]] bool is_bracket_close(const char c) const { return _close_brackets.find(c) != std::string::npos; };
		[[nodiscard]] bool is_bracket(const char c) const { return is_bracket_open(c) || is_bracket_close(c); }
	};

	/**
	 * @class Elem
	 * @brief Represents a single section or variable in a data file, which can contain either a value (as a string) or a vector of other Elem instances.
	 */
	class Elem {
	public: using Cont = std::vector<Elem>;
	protected:
		bool _is_var;							///< @brief When true, this Elem instance is a variable, else it is a list of variables. If a type-specific function is called on the wrong Elem type, a std::exception("INVALID_OPERATION") is thrown from that function.
		std::string _name;						///< @brief The name of this element. If this is a variable, it is the name of the variable. If this is a section, it is the header that appears before the opening bracket of a section.
		std::optional<std::string> _val;		///< @brief The value of this variable, used only if this Elem is a variable.
		std::optional<char> _val_enclosure;		///< @brief The quotes around a value, if the value is a string type and Param::_uses_quotes
		std::optional<Cont> _vec;				///< @brief Container of sub-elements, used only if this Element is a section.
		Elem* _parent;							///< @brief Pointer to this Elem instances parent Elem instance, if it has one, else is nullptr. This is used by the parse() function to jump back one element once it's done parsing this element's contents.
		size_t _indent{ 0 };					// this is required to prevent infinite recursion loop during output stream operations for some stupid reason.

	public:
		// Section Constructors
		explicit Elem(std::string name, Elem* parent = nullptr) : _is_var{ false }, _name{ std::move(name) }, _val{ std::nullopt }, _val_enclosure{ std::nullopt }, _vec{ Cont() }, _parent{ parent } {}
		explicit Elem(std::string name, const Cont& elements, Elem* parent = nullptr) : _is_var{ false }, _name{ std::move(name) }, _val{ std::nullopt }, _val_enclosure{ std::nullopt }, _vec{ elements }, _parent{ parent } {}
		// Element Constructor
		explicit Elem(std::string name, const std::string& val, Elem* parent = nullptr, const std::optional<char>& quotes = std::nullopt) : _is_var{ true }, _name{ std::move(name) }, _val{ val }, _val_enclosure{ quotes }, _vec{ std::nullopt }, _parent{ parent } {}

		/**
		 * @function isVar()
		 * @brief Check if this element is a variable or a section.
		 * @return true		- Element is a variable.
		 * @return false	- Element is a section.
		 */
		[[nodiscard]] bool isVar() const { return _is_var; }
		/**
		 * @function name()
		 * @brief Retrieve the name of this element.
		 * @returns std::string
		 */
		[[nodiscard]] std::string name() const { return _name; }
		/**
		 * @function value()
		 * @brief Retrieve the value of this element.
		 * @param include_quotes	- When true, returns this value exactly as it appeared in the file, including quotes.
		 * @returns std::string
		 * @throws std::exception("INVALID_OPERATION") - Element type is not a variable, and does not have a value to retrieve.
		 */
		[[nodiscard]] std::string value(const bool include_quotes = false) const
		{
			if (_is_var) {
				if (include_quotes && _val_enclosure.has_value())
					return { _val_enclosure.value() + _val.value_or("") + _val_enclosure.value() };
				else
					return _val.value_or("");
			}
			throw std::exception("INVALID_OPERATION");
		}
		/**
		 * @function getVec()
		 * @brief Retrieve a reference of this element's section.
		 * @returns Cont&
		 * @throws std::exception("INVALID_OPERATION") - Element type is a variable, and does not have a subvector to retrieve.
		 */
		Cont& getVec() { if (!_is_var && _vec.has_value()) return _vec.value(); throw std::exception("INVALID_OPERATION"); }
		/**
		 * @function getVec() const
		 * @brief Retrieve a reference of this element's section.
		 * @returns Cont&
		 * @throws std::exception("INVALID_OPERATION") - Element type is a variable, and does not have a subvector to retrieve.
		 */
		[[nodiscard]] Cont& getVec() const { if (!_is_var && _vec.has_value()) return const_cast<Cont&>(_vec.value()); throw std::exception("INVALID_OPERATION"); }
		/**
		 * @function vec() const
		 * @brief Retrieve a copy of this elements section, if it has one, else returns an empty Cont.
		 * @returns Cont
		 */
		[[nodiscard]] Cont vec() const { return _vec.value_or(Cont()); }
		/**
		 * @function parent()
		 * @brief Retrieve this elements parent pointer, or nullptr if no parent was set.
		 * @returns El[[nodiscard]] em*
		 */
		[[nodiscard]] Elem* parent() const { return _parent; }
		/**
		 * @function operator<<(std::ostream&, Elem&)
		 * @brief Stream insertion operator.
		 * @param os	- (implicit) Output Stream.
		 * @param e		- (implicit) Elem instance.
		 * @returns std::ostream&
		 */
		friend std::ostream& operator<<(std::ostream& os, Elem& e)
		{
			const std::string tabs = std::string(e._indent, '\t'); // setw() doesn't work for some reason, so calculate a constant string to contain each lines indentation.
			os << tabs << e.name();
			if (e._is_var) // this element is a variable, push it to the stream and return
				os << " = " << e.value(false) << '\n';
			else { // this element is a section, recurse into it.
				os << '\n' << tabs << "{\n";
				for (auto& it : e.getVec()) {
					it._indent = e._indent + 1; // set each sub-elements indent value to this elements indent value + 1.
					os << it;
				}
				os << tabs << "}\n";
			}
			os.flush(); // Flush output stream
			return os;
		}
		/**
		 * @function operator<<(std::ostream&) const
		 * @brief Const stream insertion operator.
		 * @param os	- (implicit) Output Stream.
		 * @returns std::ostream&
		 */
		std::ostream& operator<<(std::ostream& os) const
		{
			const std::string tabs = std::string(_indent, '\t'); // setw() doesn't work for some reason, so calculate a constant string to contain each lines indentation.
			os << tabs << name();
			if (_is_var) // this element is a variable, push it to the stream and return
				os << " = " << value(false) << '\n';
			else { // this element is a section, recurse into it.
				os << '\n' << tabs << "{\n";
				for (auto& it : getVec()) {
					it._indent = _indent + 1; // set each sub-elements indent value to this elements indent value + 1.
					os << const_cast<Elem*>(&it);
				}
				os << tabs << "}\n";
			}
			os.flush(); // Flush output stream
			return os;
		}
	};

	/**
	 * @namespace _internal
	 * @brief Namespace internal-use methods.
	 */
	namespace _internal {
		/**
		 * @function strip_line(std::string, const Param&)
		 * @brief Removes comments and preceeding/trailing whitespace from a given string.
		 * @param str				- Input string.
		 * @param cfg				- Configuration.
		 * @param strip_brackets	- When true, also removes bracket characters when removing whitespace.
		 * @returns std::string
		 */
		inline std::string strip_line(std::string str, const Param& cfg, const bool strip_brackets = false)
		{
			if (strip_brackets)
				str.erase(std::remove_if(str.begin(), str.end(), [&cfg](const char c) { return cfg.is_bracket(c); }), str.end());
			if (const auto dPos{ str.find_first_of(cfg._comments) }; !str.empty() && dPos != std::string::npos) // remove comments first
				str.erase(str.begin() + dPos, str.end());
			const auto is_whitespace{ [&cfg](const char c) { return cfg._whitespace.find(c) != std::string::npos; } }; ///< @brief Lambda for detecting whitespace chars.
			if (const auto first{ str.find_first_not_of(cfg._whitespace) }; !str.empty() && first != std::string::npos)
				str.erase(std::remove_if(str.begin(), str.begin() + first, is_whitespace), first + str.begin()); // remove from beginning of string to 1 before first non-whitespace char
			else return{ };
			if (const auto last{ str.find_last_not_of(cfg._whitespace) }; !str.empty() && last != std::string::npos)
				str.erase(std::remove_if(last + str.begin(), str.end(), is_whitespace), str.end()); // remove from 1 after last non-whitespace char to the end of the string
			return str;
		}

		/**
		 * @function build_stream(Elem::Cont&)
		 * @brief Convert an Elem::Cont into a stringstream.
		 * @param stack	- Instance to convert.
		 * @returns std::stringstream
		 */
		inline std::stringstream build_stream(Elem::Cont& stack)
		{
			std::stringstream ss{ };
			for (auto& it : stack)
				ss << it;
			return ss;
		}
	}

	/**
	 * @operator <<(std::ostream&, const std::vector<Elem>&)
	 * @brief Recursive stream insertion operator for Elem::Cont type.
	 * @param os	- (implicit) Output Stream Ref.
	 * @param cont	- (implicit) Target Container.
	 * @returns std::ostream&
	 */
	inline std::ostream& operator<<(std::ostream& os, const std::vector<Elem>& cont)
	{
		for (auto& it : cont) {
			os << it.name();
			if (it.isVar())
				os << " = " << it.value();
			else os << " { " << it.getVec() << "}";
			os << "; ";
		}
		return os;
	}

	/**
	 * @function writeElemContainer(const std::string&, Elem::Cont&&)
	 * @brief Write an element container to a file.
	 * @param filename	- Name of the file to write to.
	 * @param data		- Element container.
	 * @return true		- Successfully wrote to the given file.
	 * @return false	- Failed to write to the specified file.
	 */
	inline bool writeElemContainer(const std::string& filename, Elem::Cont& data) { return file::write_to(filename, _internal::build_stream(data), false); }

	/**
	 * TODO: Implement arrays of elements with only names, designated with square brackets. []
	 * @function parse(std::stringstream)
	 * @brief Parse a stringstream into an Elem container.
	 * @param ss	- Stringstream containing unparsed data.
	 * @param cfg	- Parse settings object.
	 * @returns Elem::Cont	- Container of all elements located in the global space.
	 * @throws std::exception("INVALID_STRINGSTREAM")					- Stringstream failbit was set, and is invalid.
	 * @throws std::exception("INVALID_BRACKETS_AT_LN_[<line number>]")	- If unmatched brackets were found. Returns line number and char index. (Only if cfg._fuzzy_brackets == false)
	 * @throws std::exception("INVALID_SYNTAX_AT_LN_[<line number>]")	- If multiple equals signs were found on the same line. (Only if cfg._multiple_setters == false)
	 */
	inline Elem::Cont parse(std::stringstream ss, const Param cfg = { })
	{
		if (ss.fail()) throw std::exception("INVALID_STRINGSTREAM"); ///< @brief sstream failbit was set
		std::stringstream sb;
		for (std::string s{ }; str::getline(ss, s, cfg._line_delims); sb << s << '\n') { // move data to buffer, replace all occurrences of line delims with a newline.
			std::replace_if(s.begin(), s.end(), [&cfg](const char c) { return cfg._setters.find(c) != std::string::npos; }, '=');
			for (auto it{ s.begin() }; it != s.end(); ++it)
				if (cfg.is_bracket_open(*it))
					it = s.insert(it, '\n') + 1;
		}

		Elem::Cont stack;		///< @brief Stores all parsed data.
		Elem* sub{ nullptr };	///< @brief Pointer to current insertion pos.
		std::string header;		///< @brief Used to store the last parsed header line.

		const auto push_elem{ [&stack, &sub](const Elem& e) {
			if (sub != nullptr) {
				sub->getVec().push_back(e);
				if (!e.isVar())
					sub = &sub->getVec().back();
			}
			else {
				stack.push_back(e);
				if (!e.isVar())
					sub = &stack.back();
			}
		} }; ///< @brief Lambda for pushing elements to the correct stack
		constexpr auto is_of{ [](const char c, const std::string& list) { return list.find(c) != std::string::npos; } }; ///< @brief Simple lambda that checks if a char is present in a string.
		const auto find_bracket{ [&is_of, &cfg](const std::string& ln, const size_t off = 0) -> std::pair<size_t, bool> {
			for (size_t i{ off }; i < ln.size(); ++i) {
				if (is_of(ln.at(i), cfg._open_brackets))
					return{ i, true };
				if (is_of(ln.at(i), cfg._close_brackets))
					return{ i, false };
			}
			return { std::string::npos, true };
		} }; ///< @brief Finds the next bracket from the given position of the given string. @returns pair<size_t, bool> where { { index | npos }, { true = opening bracket | false = closing bracket } }

		bool queue_bracket_close{ false };
		size_t	line_count{ 1 };	// used for exception output, index starts at 1, counts all lines, whether they're empty or not.
		for (std::string ln{ }; std::getline(sb, ln, '\n'); ++line_count, queue_bracket_close = false) {
			ln = _internal::strip_line(ln, cfg); // remove comments & preceeding/trailing whitespace
			if (!ln.empty()) { // if line is still not empty, process it
				// iterate through brackets on line
				for (auto bracket{ find_bracket(ln) }; bracket.first != std::string::npos; bracket = find_bracket(ln, bracket.first + 1)) {
					if (bracket.second) // is opening bracket
						push_elem(Elem(header, sub));
					else if (!bracket.second) // is closing bracket
						queue_bracket_close = true;
					else if (!cfg._fuzzy_brackets) throw std::exception(("INVALID_BRACKETS_AT_LN_[" + std::to_string(line_count) + "]").c_str());
				}
				// find variables
				if (const auto eqPos{ ln.find('=') }; eqPos != std::string::npos) { // check for equals sign on line
					if (!cfg._multiple_setters && ln.find('=', eqPos + 1) != std::string::npos) throw std::exception(("INVALID_SYNTAX_AT_LN_[" + std::to_string(line_count) + "]").c_str());
					const auto var{ _internal::strip_line(ln.substr(0, eqPos), cfg, true) }, val{ _internal::strip_line(ln.substr(eqPos + 1), cfg, true) };
					push_elem(Elem(var, val, sub));
				}
				else { // check for headers
					const auto tmp{ _internal::strip_line(ln, cfg, true) };
					//	ln = _internal::strip_line(ln, cfg, true);
					if (!tmp.empty())
						header = tmp; // set header
				}
				if (queue_bracket_close)
					sub = sub->parent();
			} // else continue; line is empty
		}

		return stack;
	}
}