#include "version.h"
#include "copyright.h"

#include "ObjectFormatter.hpp"

#include <alchlib2.hpp>
#include <opt3.hpp>
#include <envpath.hpp>

#include <iostream>

struct help {
	std::string programName;
	STRCONSTEXPR help(std::string const& programName) : programName{ programName } {}
	friend std::ostream& operator<<(std::ostream& os, const help& h)
	{
		return os
			<< "alch v" << alch_VERSION_EXTENDED << ' ' << alch_COPYRIGHT << '\n'
			<< "  Commandline alchemy helper utility for Skyrim." << '\n'
			<< '\n'
			<< "USAGE:\n"
			<< "  " << h.programName << " <OPTIONS>" << '\n'
			<< "  " << h.programName << " <MODE> <INPUTS> [...]" << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help             Shows this help display, then exits." << '\n'
			<< "  -v, --version          Shows the current version number, then exits." << '\n'
			<< "  -q, --quiet            Prevents detailed console output from being shown." << '\n'
			<< "  -a, --all              Shows all detailed console output." << '\n'
			<< "  -r, --registry <PATH>  Specifies an ingredients registry to use." << '\n'
			<< "  -n, --no-color         Disables color escape sequences." << '\n'
			//< continue [OPTIONS] here
			<< '\n'
			<< "MODES:\n"
			<< "  -l, --list             Lists all ingredients present in the registry." << '\n'
			<< "  -s, --search           " << '\n'
			<< "  -S, --smart            " << '\n'
			<< "  -B, --build            " << '\n'
			//< continue [MODES] here
			;
	}
};

enum Mode : std::uint8_t {
	None,
	List,
	Search,
	/// @brief	Searches for ingredients that have ALL of the specified names
	SmartSearch,
	Build,
};

int main(const int argc, char** argv)
{
	try {
		opt3::ArgManager args{ argc, argv,
			opt3::make_template(opt3::CaptureStyle::Required, opt3::ConflictStyle::Conflict, 'r', "registry"),
			opt3::make_template(opt3::CaptureStyle::Disabled, opt3::ConflictStyle::Conflict, 'l', "list"),
		};
		const auto& [programPath, programName] { env::PATH{}.resolve_split(argv[0]) };

		const std::filesystem::path registryPath{ args.castgetv_any<std::filesystem::path, opt3::Flag, opt3::Option>('r', "registry").value_or(std::filesystem::path{ "alch.ingredients" }) };
		const bool quiet{ args.check_any<opt3::Flag, opt3::Option>('q', "quiet") };
		const bool all{ args.check_any<opt3::Flag, opt3::Option>('a', "all") };
		const bool noColor{ args.check_any<opt3::Flag, opt3::Option>('n', "no-color") };

		csync.setEnabled(!noColor);
		keywordColors.setEnabled(!noColor);

		if (const bool noArgs{ args.empty() }; noArgs || args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << help(programName.generic_string()) << std::endl;
			if (noArgs) std::cerr << std::endl << csync(color::red) << "No arguments specified!" << csync() << std::endl;
		}
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			if (!quiet) std::cout << "alch v";
			std::cout << alch_VERSION_EXTENDED;
			if (!quiet) std::cout << ' ' << alch_COPYRIGHT;
			std::cout << std::endl;
		}
		else {
			if (!file::exists(registryPath))
				throw make_exception("Couldn't find a valid ingredients registry at ", registryPath);

			alchlib2::Registry registry{ alchlib2::Registry::ReadFrom(registryPath) };

			// Find which exclusive mode the user specified
			Mode mode{ Mode::None };

			const auto& trySetMode{ [&mode](const Mode& m) {
				if (mode != Mode::None) throw make_exception("Multiple modes cannot be specified at the same time!");
				else mode = m;
			} };

			if (args.check_any<opt3::Flag, opt3::Option>('l', "list"))
				trySetMode(Mode::List);
			else if (args.check_any<opt3::Flag, opt3::Option>('s', "search"))
				trySetMode(Mode::Search);
			else if (args.check_any<opt3::Flag, opt3::Option>('S', "smart"))
				trySetMode(Mode::SmartSearch);
			else if (args.check_any<opt3::Flag, opt3::Option>('B', "build"))
				trySetMode(Mode::Build);
			else // user specified multiple modes:
				throw make_exception("No mode was specified!");

			// Get all uncaptured parameters
			const auto& params{ args.getv_all<opt3::Parameter>() };

			ObjectFormatter fmt{ color::setcolor::yellow, quiet, all };

			// Execute mode-specific operations
			switch (mode) {
			case Mode::List: {
				if (!params.empty()) { // if parameters WERE specified, show a warning message:
					std::cerr << "Ignoring arguments: ";
					bool fst{ true };
					for (const auto& param : params) {
						if (fst) fst = false;
						else std::cerr << ", ";

						std::cerr << '"' << param << '"';
					}
				}

				for (const auto& ingr : registry.Ingredients) {
					fmt.print(std::cout, ingr);
				}
				break;
			}
			case Mode::Search: {
				if (params.empty())
					throw make_exception("Not enough search terms were specified for search mode. (Min 1)");

				for (const auto& name : params) {
					const auto results{ registry.copy_inclusive_filter(name, true, true) };

					std::cout << "Showing results for: \"" << csync(fmt.searchTermHighlightColor) << name << csync() << "\"\n"
						<< csync(color::red) << '{' << csync() << '\n';

					bool fst{ true };
					for (const auto& ingr : results.Ingredients) {
						if (fst) fst = false;
						else std::cout << '\n';
						fmt.print(std::cout, ingr, name);
					}
					std::cout << "\n" << csync(color::red) << '}' << csync() << '\n';
				}
				break;
			}
			case Mode::SmartSearch: {
				if (params.empty())
					throw make_exception("Not enough effects were specified for smart search mode. (Min 1)");

				std::cout << "Showing results for: ";
				bool fst{ true };
				for (const auto& name : params) {
					if (fst) fst = false;
					else std::cout << ", ";
					std::cout << '\"' << csync(fmt.searchTermHighlightColor) << name << csync() << '\"';
				}
				std::cout << '\n' << csync(color::red) << '{' << csync() << '\n';

				const auto results{ registry.DuplicateIfAll(predicates) };

				std::cout << '\n' << csync(color::red) << '}' << csync() << '\n';
				break;
			}
			case Mode::Build: {
				if (params.size() < 2)
					throw make_exception("Not enough ingredients were specified for build mode. (Min 2)");
				else if (params.size() > 4)
					throw make_exception("Too many ingredients specified for build mode. (Max 4)");

				// TODO: build
				break;
			}
			}
		}

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << csync.get_fatal() << ex.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << csync.get_fatal() << "An undefined exception occurred!" << std::endl;
		return 1;
	}
}
