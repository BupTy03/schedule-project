#pragma once
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <iostream>
#include <stdexcept>


using ScheduleGenOption = std::variant<int, bool>;
using ScheduleGenOptions = std::map<std::string, ScheduleGenOption>;


/**
 * Generates result schedule from schedule data.
 */
class ScheduleGenerator
{
public:
    virtual ~ScheduleGenerator() = default;
    virtual ScheduleResult Generate(const ScheduleData&) = 0;

    virtual void SetOptions(const ScheduleGenOptions&) = 0;
    virtual ScheduleGenOptions DefaultOptions() const;
    virtual std::unique_ptr<ScheduleGenerator> Clone() const = 0;
};


std::ostream& operator<<(std::ostream& os, const ScheduleGenOptions& options);

template<typename T>
T RequireOption(const ScheduleGenOptions& options, const std::string& name)
{
    auto it = options.find(name);
    if(it == options.end())
        throw std::invalid_argument("Missing option '" + name + '\'');

    if(!std::holds_alternative<T>(it->second))
        throw std::invalid_argument(std::string("Invalid option type. Expected '") + typeid(T).name() + '\'');

    return std::get<T>(it->second);
}
