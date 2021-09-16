#pragma once
#include <TermAPI.hpp>
#include <map>
#include "ObjectBase.hpp"

namespace caco_alch {
	/**
	 * @namespace Help
	 * @brief Contains methods related to the inline terminal help display.
	 */
	namespace Help {
		/**
		 * @struct Helper
		 * @brief Provides a convenient extensible help display.
		 */
		struct Helper : ObjectBase {
		private:
			template<class T, typename = std::enable_if_t<std::is_same_v<T, std::string>>> struct more {
				bool operator()(const T& a, const T& b) const
				{
					if (a.at(1) == '-' && b.at(1) != '-')
						return false;
					else if (a.at(1) != '-' && b.at(1) == '-')
						return true;
					else return a < b;
				}
			};
		public:
			using Cont = std::map<std::string, std::string, more<std::string>>;
			std::string _usage;
			Cont _doc;
			/**
			 * @constructor Helper(const std::string&, std::map<std::string, std::string>&&)
			 * @brief Default constructor, takes a string containing usage instructions, and a map of all commandline parameters and a short description of them.
			 * @param prog_name	- The string to show as the program name in the usage display. (ex. USAGE: <prog_name> <argument-syntax>)
			 * @param usage_str	- Brief string showing the commandline syntax for this program.
			 * @param doc		- A map where the key represents the commandline option, and the value is the documentation for that option.
			 */
			Helper(const std::string& prog_name, std::string usage_str, Cont doc) : ObjectBase(prog_name), _usage{ std::move(usage_str) }, _doc{ std::move(doc) } { validate(); }

			/**
			 * @function validate()
			 * @brief Called at construction time, iterates through _doc and corrects any abnormalities.
			 */
			void validate()
			{
				for (auto it{ _doc.begin() }; it != _doc.end(); ++it) {
					auto& key{ it->first };
					const auto& doc{ it->second };
					if (key.empty()) // If key is empty, delete entry
						_doc.erase(it, it);
					else {                                                         // key isn't empty
						if (const auto fst_char{ key.at(0) }; fst_char != '-') { // if key does not have a dash prefix

							_doc.erase(it, it);
							std::string mod{ '-' };
							if (key.size() > 1) // is longopt
								mod += '-';
							_doc[mod + key] = doc;
						}
					}
				}
			}

			/**
			 * @function operator<<(std::ostream&, const Helper&)
			 * @brief Stream insertion operator, returns usage string and documentation for all options.
			 * @param os	- (implicit) Output stream instance.
			 * @param h		- (implicit) Helper instance.
			 * @returns std::ostream&
			 */
			friend std::ostream& operator<<(std::ostream& os, const Helper& h)
			{
				os << "Usage:\n  " << h._name << ' ' << h._usage << "\nOptions:\n";
				// get longest key name, use as indent
				std::streamsize indent{ 0 };
				for (const auto& [key, doc] : h._doc)
					if (const auto size{ key.size() }; size > indent)
						indent = static_cast<std::streamsize>(size);
				for (auto& [key, doc] : h._doc)
					os << "  " << std::left << std::setw(indent + 2ll) << key << doc << '\n';
				os.flush();
				return os;
			}
		};

		/**
		 * @function print_help(const Helper& = _default_doc)
		 * @brief Display help information.
		 * @param documentation	- Documentation to display
		 */
		inline void print(const Helper& documentation)
		{
			std::cout << documentation << std::endl;
		}
	}
}