#pragma once
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"

#include <map>
#include <string>
#include <variant>
#include <iostream>
#include <stdexcept>


using ScheduleGenOption = std::variant<std::uint32_t, bool>;

template<typename T>
T RequireOption(const std::map<std::string, ScheduleGenOption>& options, const std::string& name)
{
    auto it = options.find(name);
    if(it == options.end())
        throw std::invalid_argument("Missing option '" + name + '\'');

    if(!std::holds_alternative<T>(it->second))
        throw std::invalid_argument(std::string("Invalid option type. Expected '") + typeid(T).name() + '\'');

    return std::get<T>(it->second);
}


/**
 * Generates result schedule from schedule data.
 */
class ScheduleGenerator
{
public:
    virtual ~ScheduleGenerator() = default;
    virtual void SetOptions(const std::map<std::string, ScheduleGenOption>& options)
    {
        for(auto&&[key, option] : options)
        {
            std::cout << '[' << key << "] = ";
            std::visit([](auto&& value){ std::cout << value; }, option);
            std::cout << '\n';
        }

        std::cout.flush();
    }
    [[nodiscard]] virtual ScheduleResult Generate(const ScheduleData&) = 0;
};
