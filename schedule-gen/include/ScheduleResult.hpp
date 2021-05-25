#pragma once
#include "ScheduleCommon.hpp"

#include <map>
#include <cstdint>
#include <vector>
#include <optional>
#include <stdexcept>


class ScheduleData;

struct ScheduleItem
{
    ScheduleItem() = default;
    explicit ScheduleItem(std::size_t lessonAddress,
                          std::size_t subjectRequestID,
                          std::size_t classroom);

    std::size_t Address = 0;
    std::size_t SubjectRequestID = 0;
    std::size_t Classroom = 0;
};

struct ScheduleItemLess
{
    [[nodiscard]] bool operator()(const ScheduleItem& lhs, const ScheduleItem& rhs) const
    {
        return lhs.Address < rhs.Address;
    }
    [[nodiscard]] bool operator()(const ScheduleItem& lhs, std::size_t rhsLessonAddress) const
    {
        return lhs.Address < rhsLessonAddress;
    }
    [[nodiscard]] bool operator()(std::size_t lhsLessonAddress, const ScheduleItem& rhs) const
    {
        return lhsLessonAddress < rhs.Address;
    }
};

template<typename Iter>
struct Range
{
    explicit Range(Iter first, Iter last)
        : first_(first)
        , last_(last)
    {}

    auto begin() const { return first_; }
    auto end() const { return last_; }

private:
    Iter first_;
    Iter last_;
};

template<typename Iter>
Range<Iter> make_range(Iter first, Iter last) { return Range<Iter>(first, last); }


class ScheduleResult
{
public:
    ScheduleResult() = default;
    explicit ScheduleResult(std::vector<ScheduleItem> items);

    [[nodiscard]] bool empty() const;
    [[nodiscard]] const std::vector<ScheduleItem>& items() const;

    std::vector<ScheduleItem>::iterator insert(const ScheduleItem& item);
    [[nodiscard]] Range<std::vector<ScheduleItem>::const_iterator> at(std::size_t lessonAddress) const;

private:
    std::vector<ScheduleItem> items_;
};


struct OverlappedClassroom
{
    std::size_t Address = 0;
    std::size_t Classroom = 0;
    std::vector<std::size_t> SubjectRequestsIDs;
};

struct OverlappedProfessor
{
    std::size_t Address = 0;
    std::size_t Professor = 0;
    std::vector<std::size_t> SubjectRequestsIDs;
};

struct OverlappedGroups
{
    std::size_t Address = 0;
    std::vector<std::size_t> Groups;
    std::vector<std::size_t> SubjectRequestsIDs;
};

struct ViolatedWeekdayRequest
{
    std::size_t Address = 0;
    std::size_t SubjectRequestID = 0;
};


struct CheckScheduleResult
{
    std::vector<OverlappedClassroom> OverlappedClassroomsList;
    std::vector<OverlappedProfessor> OverlappedProfessorsList;
    std::vector<OverlappedGroups> OverlappedGroupsList;
    std::vector<ViolatedWeekdayRequest> ViolatedWeekdayRequestsList;
};


[[nodiscard]] std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data,
                                                                        const ScheduleResult& result);

[[nodiscard]] std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data,
                                                                        const ScheduleResult& result);

[[nodiscard]] std::vector<OverlappedGroups> FindOverlappedGroups(const ScheduleData& data,
                                                                 const ScheduleResult& result);

[[nodiscard]] std::vector<ViolatedWeekdayRequest> FindViolatedWeekdayRequests(const ScheduleData& data,
                                                                              const ScheduleResult& result);

CheckScheduleResult CheckSchedule(const ScheduleData& data,
                                  const ScheduleResult& result);
