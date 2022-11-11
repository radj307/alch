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
			<< "  " << h.programName << " <MODE> <INPUT>..." << '\n'
			<< '\n'
			<< "  Arguments that include whitespace must be enclosed with quotes (\"), or they'll be split into multiple inputs." << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help          Shows this help display, then exits." << '\n'
			<< "  -v, --version       Shows the current version number, then exits." << '\n'
			<< "  -n, --no-color      Disables all colorized output." << '\n'
			<< "  -q, --quiet         Prevents detailed console output from being shown." << '\n'
			<< "  -a, --all           Shows all detailed console output." << '\n'
			<< "  -e, --exact         Match whole search terms rather than allowing any result that contains the search term." << '\n'
			<< "  -i, --ingr <PATH>   Override the default search path for the ingredients registry." << '\n'
			<< "  -g, --gmst <PATH>   Override the default search path for the game settings config. This only applies to build mode." << '\n'
			//< continue [OPTIONS] here
			<< '\n'
			<< "MODES:\n"
			<< "  -l, --list          Lists all ingredients present in the registry. This mode does not accept any inputs." << '\n'
			<< "  -s, --search        Search for ingredients or effects. Requires at least one <INPUT>." << '\n'
			<< "  -S, --smart         Search for ingredients that have effects matching all of the given <INPUTS>." << '\n'
			<< "  -B, --build         " << '\n'
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
			opt3::make_template(opt3::CaptureStyle::Required, opt3::ConflictStyle::Conflict, 'i', "ingr"),
			opt3::make_template(opt3::CaptureStyle::Required, opt3::ConflictStyle::Conflict, 'g', "gmst"),
			opt3::make_template(opt3::CaptureStyle::Disabled, opt3::ConflictStyle::Conflict, 'l', "list"),
		};
		const auto& [programPath, programName] { env::PATH{}.resolve_split(argv[0]) };

		const std::filesystem::path registryPath{ args.castgetv_any<std::filesystem::path, opt3::Flag, opt3::Option>('i', "ingr").value_or(std::filesystem::path{ "alch.ingredients" }) };
		const bool quiet{ args.check_any<opt3::Flag, opt3::Option>('q', "quiet") };
		const bool all{ args.check_any<opt3::Flag, opt3::Option>('a', "all") };
		const bool noColor{ args.check_any<opt3::Flag, opt3::Option>('n', "no-color") };
		const bool exact{ args.check_any<opt3::Flag, opt3::Option>('e', "exact") };

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
				throw make_exception("Couldn't find a valid ingredients registry at ", registryPath, "!\n", shared::indent(10), "You can generate an ingredients registry with this tool:\n", shared::indent(10), "https://github.com/radj308/alch-registry-generator");

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

				std::cout << "Listing all ingredients:"
					<< '\n' << csync(color::red) << '{' << csync() << '\n';

				bool fst{ true };
				for (const auto& ingr : registry.Ingredients) {
					if (fst) fst = false;
					else std::cout << '\n';
					fmt.print(std::cout, ingr);
				}

				std::cout << "\n" << csync(color::red) << '}' << csync() << '\n';
				break;
			}
			case Mode::Search: {
				if (params.empty())
					throw make_exception("Not enough search terms were specified for search mode. (Min 1)");

				for (const auto& name : params) {
					const auto results{ registry.copy_inclusive_filter(name, exact, true, true) };

					std::cout << "Showing results for: \"" << csync(fmt.searchTermHighlightColor) << name << csync() << "\"\n"
						<< csync(color::red) << '{' << csync() << '\n';

					bool fst{ true };
					for (const auto& ingr : results.Ingredients) {
						if (fst) fst = false;
						else std::cout << '\n';
						fmt.print(std::cout, ingr, name, exact);
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

				const auto results{ registry.copy_if([&params, &exact](alchlib2::Ingredient const& ingredient) {
					return std::all_of(params.begin(), params.end(), [&ingredient, &exact](auto&& name) { return ingredient.AnyEffectIsSimilarTo(name, exact); });
				}) };

				fst = true;
				for (const auto& ingr : results.Ingredients) {
					if (fst) fst = false;
					else std::cout << '\n';
					fmt.print(std::cout, ingr, params, exact);
				}

				std::cout << '\n' << csync(color::red) << '}' << csync() << '\n';
				break;
			}
			case Mode::Build: {
				if (params.size() < 2)
					throw make_exception("Not enough ingredients were specified for build mode. (Min 2)");
				
				// retrieve the game settings config:
				alchlib2::AlchemyCoreGameSettings coreGameSettings{};

				if (const auto gameSettingsConfigPath{ args.castgetv_any<std::filesystem::path, opt3::Flag, opt3::Option>('g', "gmst").value_or("alch.gmst") };
					file::exists(gameSettingsConfigPath))
					coreGameSettings = alchlib2::AlchemyCoreGameSettings::ReadFrom(gameSettingsConfigPath);

				// collect all the ingredients:
				const auto& results{ registry.find_best_fit(params, true, false) };
				alchlib2::PotionBuilder builder{ coreGameSettings };
				alchlib2::perks::VanillaPerks vanillaPerks{};
				const auto potion{ builder.Build(results.Ingredients, vanillaPerks.GetAllPerks()) };

				// print input ingredients:
				std::cout << "Combining ingredients:" << '\n' << csync(color::red) << '{' << csync() << '\n';
				bool fst{ true };
				for (const auto& ingr : results.Ingredients) {
					if (fst) fst = false;
					else std::cout << '\n';
					fmt.print(std::cout, ingr, params, exact);
				}
				std::cout << '\n' << csync(color::red) << '}' << csync() << '\n';

				// print potion name & alchemy stats
				std::cout << "Produces: \"" << csync(color::bold) << potion.name << csync(color::no_bold) << "\"\n";
				if (all) {
					std::cout
						<< csync(color::gray) << "With alchemy stats:" << '\n'
						<< "  Skill:     " << csync(color::green) << coreGameSettings.fAlchemyAV.value << csync(color::gray) << '\n'
						<< "  Modifier:  " << csync(color::green) << coreGameSettings.fAlchemyMod.value << csync() << '\n';
				}

				// print effects:
				std::cout << "Effects:" << '\n' << csync(color::red) << '{' << csync() << '\n';
				fst = true;
				for (const auto& effect : potion.effects) {
					if (fst) fst = false;
					else std::cout << '\n';
					fmt.print(std::cout, effect);
				}
				std::cout << '\n' << csync(color::red) << '}' << csync() << '\n';

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
