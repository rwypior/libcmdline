#include "libcmdline/cmdline.h"

#include <cstdarg>
#include <cstdio>
#include <functional>
#include <sstream>
#include <iomanip>
#include <cassert>

namespace cmdline
{
	// Dependency preds
	
	ArgumentEnablePred enableAlways()
	{
		return [](){
			return true; 
		};
	}

	ArgumentEnablePred enableWhenSwitchIsSet(const Switch& s)
	{
		return [&s](){
			return s.on();
		};
	}

	HelpPred staticHelp(const std::string& help)
	{
		return [help]() {
			return help;
		};
	}

	// Parse result
	ParseResult::ParseResult(const std::vector<std::string>& errors)
		: errors(errors)
	{ }
	ParseResult::~ParseResult() = default;

	ParseResult& ParseResult::operator=(const ParseResult& b)
	{
		this->errors = b.errors;
		return *this;
	}

	ParseResult::operator bool() const
	{
		return this->errors.empty();
	}

	bool ParseResult::merge(const ParseResult& res)
	{
		this->errors.insert(this->errors.end(), res.errors.begin(), res.errors.end());
		return res;
	}

	std::string ParseResult::errorStr() const
	{
		std::string result;
		for (const std::string& err : this->errors)
		{
			result += err + "\n";
		}
		return result;
	}

	ArgumentParseResult::ArgumentParseResult(bool accepted, const std::string& error)
		: ParseResult(error.empty() ? std::vector<std::string>{} : std::vector<std::string>{ error })
		, accepted(accepted)
	{ }
	ArgumentParseResult::~ArgumentParseResult() = default;

	ArgumentParseResult& ArgumentParseResult::operator=(const ArgumentParseResult& b)
	{
		ParseResult::operator=(b);
		this->accepted = b.accepted;
		return *this;
	}

	ArgumentParseResult::operator bool() const
	{
		return this->accepted;
	}

	// Parser

	Parser::Parser(bool autohelp)
		: autohelp(autohelp)
	{
		if (autohelp)
			this->addStandardHelpSwitch();
	}

	ParseResult Parser::parse(int argc, char** argv)
	{
		return this->parse(std::vector<std::string>(argv, argv + argc));
	}

	ParseResult Parser::parse(const std::vector<std::string>& args)
	{
		assert(this->validateCommand() && "Command is ill-formed");

		ParseResult result;

		// Used to fill option's value in the "--option value syntax"
		Option* activeOption = nullptr;

		if (!args.empty())
			this->cmdname = args.front();

		size_t pos = 0;
		for (auto it = args.begin() + 1; it != args.end(); it++)
		{
			const std::string& arg = *it;

			if (activeOption)
			{
				activeOption->value = arg;
				activeOption = nullptr;
				continue;
			}

			ArgumentParseResult argres { false };
			if (argres.merge(this->parseArgument(arg, pos)))
				continue;
			if (argres.merge(this->parseOption(arg, &activeOption)))
				continue;
			if (argres.merge(this->parseSwitch(arg)))
				continue;

			result.merge(argres);
		}

		result.merge(this->validateArguments());
		result.merge(this->validateOptions());

		return result;
	}

	ArgumentParseResult Parser::parseArgument(const std::string& arg, size_t& pos)
	{
		if (isOption(arg) || isOptionAbbr(arg))
			return false;

		Argument* argument = this->getArgument(pos);
		if (!argument || !argument->enabled())
			//return false;
			return {false, std::string("This command does not accept ") + std::to_string(pos + 1) + " positional arguments"};

		if (!argument->enabled())
			return false;

		argument->value = arg;
		pos++;

		return true;
	}

	ArgumentParseResult Parser::parseOption(const std::string& arg, Option** activeOption)
	{
		Option* option = nullptr;
		bool abbr = false;
		if (isOption(arg))
			option = this->getOption(getOptionName(arg));
		else if (isOptionAbbr(arg))
		{
			option = this->getOption(getOptionAbbr(arg).front());
			abbr = true;
		}
		else
			return false;

		if (!option || !option->enabled())
			return {false, std::string("This command does not accept \"") + arg + "\" option"};

		auto nameVal = getNameEqualsValue(arg);
		if (abbr)
		{
			if (!nameVal.second.empty())
				option->value = nameVal.second;
			else if (arg.length() > 2) // For cases like -x42
				option->value = arg.substr(2);
			else // For cases like -x 42
				*activeOption = option;
			return true;
		}

		if (!nameVal.first.empty())
		{
			// For cases like --xyz=42
			option->value = nameVal.second;
			return true;
		}

		// For cases like --xyz 42
		*activeOption = option;
		return true;
	}

	ArgumentParseResult Parser::parseSwitch(const std::string& arg)
	{
		if (!isOption(arg) && !isOptionAbbr(arg))
			return false;

		bool abbr = isOptionAbbr(arg);

		if (abbr)
		{
			// For cases like -x or -xyz
			for (char c : getOptionAbbr(arg))
			{
				Switch* sw = this->getSwitch(c);
				if (!sw || !sw->enabled())
			return {false, std::string("This command does not accept \"") + arg + "\" switch"};
				sw->setValue(true);
			}

			return true;
		}
		
		// For cases like --xyz
		Switch *sw = this->getSwitch(getOptionName(arg));
		if (!sw || !sw->enabled())
			return false;
		sw->setValue(true);

		return true;
	}

	Argument& Parser::addArgument(
			const std::string& name, 
			const std::string& value, 
			Req required,
			const std::string& description,
			ArgumentEnablePred enablePred)
	{
		return this->addArgument(Argument(name, value, required, description, enablePred));
	}

	Argument& Parser::addArgument(const Argument& arg)
	{
		this->args.push_back(arg);
		return this->args.back();
	}

	Option& Parser::addOption(
			const std::string& name, 
			char abbr, 
			const std::string& value, 
			Req required,
			const std::string& description,
			ArgumentEnablePred enablePred)
	{
		return this->addOption(Option(name, abbr, value, required, description, enablePred));
	}

	Option& Parser::addOption(const Option& option)
	{
		this->options.push_back(option);
		return this->options.back();
	}

	Switch& Parser::addSwitch(
			const std::string& name, 
			char abbr,
			const std::string& description,
			ArgumentEnablePred enablePred)
	{
		return this->addSwitch(Switch(name, abbr, description, enablePred));
	}

	Switch& Parser::addSwitch(const Switch& sw)
	{
		this->switches.push_back(sw);
		return this->switches.back();
	}

	void Parser::addStandardHelpSwitch()
	{
		this->addSwitch("help", '?', "Show help message");
	}

	void Parser::addHelpSection(const HelpSection& hs)
	{
		this->helpSections.push_back(hs);
	}

	void Parser::addHelpSection(const std::string& name, const std::string& description)
	{
		this->addHelpSection(HelpSection(name, description));
	}

	Argument* Parser::getArgument(const std::string& name)
	{
		for (Argument& arg : this->args)
		{
			if (arg.name == name)
				return &arg;
		}
		return nullptr;
	}

	Argument* Parser::getArgument(size_t pos)
	{
		if (this->args.size() <= pos)
			return nullptr;

		auto it = this->args.begin();
		std::advance(it, pos);
		return &*it;
	}
	
	const Argument* Parser::getArgument(const std::string& name) const
	{
		for (const Argument& arg : this->args)
		{
			if (arg.name == name)
				return &arg;
		}
		return nullptr;
	}

	Option* Parser::getOption(const std::string& name)
	{
		for (Option& opt : this->options)
		{
			if (opt.name == name)
				return &opt;
		}
		return nullptr;
	}	
		
	Option* Parser::getOption(const char abbr)
	{
		for (Option& opt : this->options)
		{
			if (opt.abbr == abbr)
				return &opt;
		}
		return nullptr;
	}

	const Option* Parser::getOption(const std::string& name) const
	{
		for (const Option& opt : this->options)
		{
			if (opt.name == name)
				return &opt;
		}
		return nullptr;
	}

	Switch* Parser::getSwitch(const std::string& name)
	{
		for (Switch& sw : this->switches)
		{
			if (sw.name == name)
				return &sw;
		}
		return nullptr;
	}

	Switch* Parser::getSwitch(const char abbr)
	{
		for (Switch& sw : this->switches)
		{
			if (sw.abbr == abbr)
				return &sw;
		}
		return nullptr;
	}	
	
	const Switch* Parser::getSwitch(const std::string& name) const
	{
		for (const Switch& sw : this->switches)
		{
			if (sw.name == name)
				return &sw;
		}
		return nullptr;
	}

	std::vector<std::reference_wrapper<const Argument>> Parser::getArguments() const
	{
		std::vector<std::reference_wrapper<const Argument>> result;
		for (const Argument& arg : this->args)
		{
			if (arg.enabled())
				result.push_back(arg);
		}
		return result;
	}
	
	std::vector<std::reference_wrapper<const Option>> Parser::getOptions() const
	{
		std::vector<std::reference_wrapper<const Option>> result;
		for (const Option& opt : this->options)
		{
			if (opt.enabled())
				result.push_back(opt);
		}
		return result;
	}
	
	std::vector<std::reference_wrapper<const Switch>> Parser::getSwitches() const
	{
		std::vector<std::reference_wrapper<const Switch>> result;
		for (const Switch& sw : this->switches)
		{
			if (sw.enabled())
				result.push_back(sw);
		}
		return result;
	}

	ArgumentParseResult Parser::validateArguments() const
	{
		for (const Argument& arg : this->args)
		{
			if (!arg.enabled())
				continue;

			if (
				arg.required == Req::required &&
				arg.value.empty()
			)
				return { false, std::string("Positional argument ") + arg.name + " is required" };
		}

		return true;
	}

	ArgumentParseResult Parser::validateOptions() const
	{
		for (const Option& opt : this->options)
		{
			if (!opt.enabled())
				continue;

			if (
				opt.required == Req::required &&
				opt.value.empty()
			)
				return { false, std::string("Option ") + opt.name + " is required" };
		}

		return true;
	}

	ParseResult Parser::validateCommand() const
	{
		ParseResult res;

		bool hasOptional = false;
		size_t i = 0;
		for (const Argument& arg : this->args)
		{
			if (arg.required == Req::optional)
				hasOptional = true;
			else if (hasOptional)
				res.merge({{"Positional argument \"" + arg.name + "\" cannot be optional"}});
		}

		return res;
	}

	bool Parser::isOption(const std::string& arg)
	{
		return arg.size() > 2 && arg[0] == '-' && arg[1] == '-';
	}
	
	bool Parser::isOptionAbbr(const std::string& arg)
	{
		return arg.size() > 1 && !isOption(arg) && arg.front() == '-';
	}

	std::string Parser::getOptionName(const std::string& arg)
	{
		size_t equals = arg.find_first_of('=');
		return arg.substr(2, equals - 2);
	}

	std::string Parser::getOptionAbbr(const std::string& arg)
	{
		return arg.substr(1);
	}

	std::pair<std::string, std::string> Parser::getNameEqualsValue(const std::string& arg)
	{
		size_t eqpos = arg.find_first_of('=');
		if (eqpos == std::string::npos)
			return {};
		std::string name = getOptionName(arg.substr(0, eqpos));
		std::string val = arg.substr(eqpos + 1);
		return { std::move(name), std::move(val) };
	}

	std::string Parser::getArgRepresentation(const Argument& arg)
	{
		if (const Option* opt = dynamic_cast<const Option*>(&arg))
		{
			std::string res = "--" + opt->name;
			if (opt->abbr)
			{
				res += ", -";
				res.push_back(opt->abbr);
			}

			if (opt->expectsValue())
				res += " [value]";

			return res;
		}
			
		return arg.name;
	}
	
	size_t Parser::getNameLength(const std::vector<std::reference_wrapper<const Argument>>& args) const
	{
		size_t result = 0;

		for (const Argument& arg : args)
		{
			size_t width = getArgRepresentation(arg).length();
			if (width <= this->helpMaxArgWidth && width > result)
					result = width;
		}

		return result;
	}

	void Parser::setHelpMaxWidth(size_t w)
	{
		this->helpMaxWidth = w;
	}

	void Parser::setHelp(HelpPred pred)
	{
		this->helpPred = pred;
	}

	void Parser::setHelp(const std::string& help)
	{
		this->helpPred = staticHelp(help);
	}

	std::string Parser::getHelp() const
	{
		std::string result;

		if (this->helpPred)
			result += this->helpPred() + "\n\n";

		result += "Usage:\n\n";

		auto args = this->getArguments();
		auto opts = this->getOptions();
		auto switches = this->getSwitches();

		result += "  " + this->cmdname + " [Options]";
		for (const auto& arg : args)
			result += " <" + arg.get().name + ">";
		result += "\n";

		auto section = [this](const std::vector<std::reference_wrapper<const Argument>>& args, const std::string& name = ""){
			size_t widest = getNameLength(args);

			std::stringstream str;

			if (!name.empty())
				str << name << ":\n";

			str << std::setiosflags(std::ios::left);
			for (const Argument& arg : args)
			{
				auto name = getArgRepresentation(arg);

				size_t width = name.length();
				std::string indent(2, ' ');
				if (width > widest)
				{
					str << indent << name << "\n";
					str << indent << std::setw(widest) << " ";
				}
				else
				{
					str << indent << std::setw(widest) << name;
				}

				if (!arg.description.empty())
				{
					str << " = ";
					str << arg.description << "\n";
				}
				else
					str << "\n";
			}

			return str.str();
		};

		if (!args.empty())
			result += "\n" + section(args, "Arguments");

		std::vector<std::reference_wrapper<const Argument>> allOptions;
		allOptions.insert(allOptions.end(), opts.begin(), opts.end());
		allOptions.insert(allOptions.end(), switches.begin(), switches.end());

		if (!allOptions.empty())
			result += "\n" + section(allOptions, "Options");

		return result;
	}
}
