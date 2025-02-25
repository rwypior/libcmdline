#ifndef _h_libcmdline_cmdline
#define _h_libcmdline_cmdline

#include <string>
#include <vector>
#include <functional>
#include <list>

namespace cmdline
{
	class Parser;
	struct Argument;
	struct Option;
	struct Switch;

	constexpr char NoAbbr = 0; // Argument has no abbreviation

	// Argument enablement predicate used by Argument::enabled() function
	using ArgumentEnablePred = std::function<bool()>;

	// Create argument enablement predicate that's always enabled
	ArgumentEnablePred enableAlways();

	// Create argument enablement predicate that's enabled when switch s is set
	ArgumentEnablePred enableWhenSwitchIsSet(const Switch& s);

	// Parser help predicate used to generate help string for the command
	using HelpPred = std::function<std::string()>;

	// Create help predicate that returns given help text
	HelpPred staticHelp(const std::string& help);

	enum class Req
	{
		required,
		optional
	};

	class ParseResult
	{
	public:
		ParseResult(const std::vector<std::string>& errors = {});
		virtual ~ParseResult();

		ParseResult& operator=(const ParseResult& b);

		virtual operator bool() const;
		bool merge(const ParseResult& res);

		std::string errorStr() const;

	protected:
		std::vector<std::string> errors;
	};

	class ArgumentParseResult : public ParseResult
	{
	public:
		ArgumentParseResult(bool accepted, const std::string& error = "");
		virtual ~ArgumentParseResult();

		ArgumentParseResult& operator=(const ArgumentParseResult& b);

		virtual operator bool() const override;

	protected:
		bool accepted;
	};

	struct HelpSection
	{
		std::string name;
		std::string description = "";

		HelpSection(const std::string& name, const std::string &description = "")
			: name(name)
			, description(description)
		{
		}
	};

	// Any command line argument
	struct Argument
	{
		std::string name;
		std::string value = "";
		Req required;
		ArgumentEnablePred enablePred = {};

		std::string description;
		HelpSection* helpSection = nullptr;
		size_t helpIndex = 0;

		Argument(
				const std::string& name, 
				const std::string& value = "", 
				Req required = Req::required, 
				const std::string& description = "",
				ArgumentEnablePred enablePred = enableAlways()
		)
			: name(name)
			, value(value)
			, required(required)
			, description(description)
			, enablePred(enablePred)
		{ }

		virtual ~Argument() = default;

		virtual bool expectsValue() const
		{
			return false;
		}

		bool enabled() const
		{
			return this->enablePred();
		}

		Argument& setDescription(const std::string& desc)
		{
			this->description = desc;
			return *this;
		}

		Argument& setHelpSection(HelpSection* section)
		{
			this->helpSection = section;
			return *this;
		}

		Argument& setHelpIndex(size_t i)
		{
			this->helpIndex = i;
			return *this;
		}

		Argument& setRequired(bool r = true)
		{
			this->required = r ? Req::required : Req::optional;
			return *this;
		}

		Argument& setPred(ArgumentEnablePred pred = enableAlways())
		{
			this->enablePred = pred;
			return *this;
		}
	};

	// Arguments prefixed with hyphens
	struct Option : Argument
	{
		char abbr = NoAbbr;

		Option(
				const std::string& name, 
				char abbr = NoAbbr, 
				const std::string& value = "", 
				Req required = Req::required,
				const std::string& description = "",
				ArgumentEnablePred enablePred = enableAlways()
		)
			: Argument(name, value, required, description, enablePred)
			, abbr(abbr)
		{ }
		
		virtual bool expectsValue() const
		{
			return true;
		}
	};

	// Options that may have no value (either on or off)
	struct Switch : Option
	{
		Switch(
				const std::string& name, 
				char abbr = NoAbbr, 
				const std::string& description = "",
				ArgumentEnablePred enablePred = enableAlways()
		)
			: Option(name, abbr, "", Req::optional, description, enablePred)
		{ }

		virtual bool expectsValue() const
		{
			return false;
		}

		void setValue(bool value)
		{
			this->value = value ? "1" : "";
		}

		bool on() const
		{
			return !this->value.empty();
		}
	};


	class Parser
	{
	public:
		// autohelp - when true, will add standard --help and -? switches for displaying help message. Later user can use Parser::helpRequested and Parser::getHelp functions to display the help string
		Parser(bool autohelp = true);

		// Parse given arguments. Keep in mind the first argument
		// must be the name of the application.
		ParseResult parse(int argc, char** argv);
		ParseResult parse(const std::vector<std::string>& args);

		Argument& addArgument(
				const std::string& name, 
				const std::string& value = "", 
				Req required = Req::required,
				const std::string& description = "",
				ArgumentEnablePred dependsOn = enableAlways());
		Argument& addArgument(const Argument& arg);

		Option& addOption(
				const std::string& name, 
				char abbr = NoAbbr, 
				const std::string& value = "", 
				Req required = Req::optional,
				const std::string& description = "",
				ArgumentEnablePred dependsOn = enableAlways());
		Option& addOption(const Option& option);

		Switch& addSwitch(
				const std::string& name, 
				char abbr = NoAbbr,
				const std::string& description = "",
				ArgumentEnablePred dependsOn = enableAlways());
		Switch& addSwitch(const Switch& sw);

		void addStandardHelpSwitch();
		void addHelpSection(const HelpSection& hs);
		void addHelpSection(const std::string& name, const std::string& description = "");

		Argument* getArgument(const std::string& name);
		Argument* getArgument(size_t pos);
		const Argument* getArgument(const std::string& name) const;
		Option* getOption(const std::string& name);
		Option* getOption(const char abbr);
		const Option* getOption(const std::string& name) const;
		Switch* getSwitch(const std::string& name);
		Switch* getSwitch(const char abbr);
		const Switch* getSwitch(const std::string& name) const;

		std::vector<std::reference_wrapper<const Argument>> getArguments() const;
		std::vector<std::reference_wrapper<const Option>> getOptions() const;
		std::vector<std::reference_wrapper<const Switch>> getSwitches() const;

		bool helpRequested() const
		{
			const auto* sw = this->getSwitch("help");
			return sw && sw->on();
		}

		ArgumentParseResult validateArguments() const;
		ArgumentParseResult validateOptions() const;

		// Check if command isn't ill-formed before parse is called.
		// Eg. optional positional arguments must be at the end of the command line
		ParseResult validateCommand() const;

		static bool isOption(const std::string& arg);
		static bool isOptionAbbr(const std::string& arg);

		static std::string getOptionName(const std::string& arg);
		static std::string getOptionAbbr(const std::string& arg);

		// Extract argument name and value, eg {xxx, yyy} from --xxx=yyy
		static std::pair<std::string, std::string> getNameEqualsValue(const std::string& arg);

		// Get argument representation in '--arg, -a [value]' format
		static std::string getArgRepresentation(const Argument& arg);
		size_t getNameLength(const std::vector<std::reference_wrapper<const Argument>>& args) const;

		void setHelpMaxWidth(size_t w);
		void setHelp(HelpPred pred);
		void setHelp(const std::string& help);
		std::string getHelp() const;

	protected:
		ArgumentParseResult parseArgument(const std::string& arg, size_t& pos);
		ArgumentParseResult parseOption(const std::string& arg, Option** activeOption);	
		ArgumentParseResult parseSwitch(const std::string& arg);

	protected:
		std::string cmdname;
		std::list<Argument> args;
		std::list<Option> options;
		std::list<Switch> switches;

		std::vector<HelpSection> helpSections;

		bool autohelp;
		size_t helpMaxWidth = 250;
		size_t helpMaxArgWidth = 50;

		HelpPred helpPred = {};		
	};
}

#endif
