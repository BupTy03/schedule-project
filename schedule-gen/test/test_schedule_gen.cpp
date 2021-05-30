#include "ScheduleUtils.hpp"
#include "ScheduleCommon.hpp"
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"

#include <catch2/catch.hpp>

#include <array>


TEST_CASE("Test.WeekDays.contains", "[WeekDays]")
{
    WeekDays days;
    REQUIRE(days.contains(WeekDay::Monday));
    REQUIRE(days.contains(WeekDay::Tuesday));
    REQUIRE(days.contains(WeekDay::Wednesday));
    REQUIRE(days.contains(WeekDay::Thursday));
    REQUIRE(days.contains(WeekDay::Friday));
    REQUIRE(days.contains(WeekDay::Saturday));
}

TEST_CASE("Test.WeekDays.erase", "[WeekDays]")
{
    auto checkRemove = [](const WeekDays& weekDays, WeekDay removed){
        for(int i = 0; i < weekDays.size(); ++i)
        {
            const auto wd = static_cast<WeekDay>(i);
            if(removed == wd)
            {
                REQUIRE_FALSE(weekDays.contains(wd));
            }
            else
            {
                REQUIRE(weekDays.contains(wd));
            }
        }
    };

    for(int i = 0; i < 6; ++i)
    {
        const auto wd = static_cast<WeekDay>(i);
        WeekDays days;
        days.erase(wd);
        checkRemove(days, wd);
    }

    WeekDays days;
    for(int i = 0; i < 6; ++i)
        days.erase(static_cast<WeekDay>(i));

    REQUIRE(days == WeekDays::emptyWeek());
}

TEST_CASE("Test.WeekDays.insert", "[WeekDays]")
{
    for(int i = 0; i < 6; ++i)
    {
        WeekDays weekDays = WeekDays::emptyWeek();
        weekDays.insert(static_cast<WeekDay>(i));
        for(int j = 0; j < 6; ++j)
        {
            if(i == j)
                REQUIRE(weekDays.contains(static_cast<WeekDay>(j)));
            else
                REQUIRE_FALSE(weekDays.contains(static_cast<WeekDay>(j)));
        }
    }
}

TEST_CASE("Test.WeekDays.WeekDaysIterator", "[WeekDays]")
{
    for(int i = 0; i < 6; ++i)
    {
        const auto removed = static_cast<WeekDay>(i);

        WeekDays weekDays;
        weekDays.erase(removed);

        auto it = weekDays.begin();
        for(int j = 0; j < 6; ++j)
        {
            if(i == j)
                REQUIRE_FALSE(*it);
            else
                REQUIRE(*it);

            ++it;
        }
        REQUIRE(it == weekDays.end());
    }
}


TEST_CASE("SubjectRequest constructs correctly")
{
    SECTION("Sorting and removing duplicates from WeekDays when constructing")
    {
        const SubjectRequest sut{0, 1, 1,
                           {WeekDay::Saturday, WeekDay::Thursday, WeekDay::Wednesday,
                             WeekDay::Saturday, WeekDay::Thursday, WeekDay::Monday}, {0}, {}};

        REQUIRE(sut.RequestedWeekDays() == WeekDays({WeekDay::Monday, WeekDay::Wednesday, WeekDay::Thursday, WeekDay::Saturday}));
    }
    SECTION("Sorting and removing duplicates from groups list while constructing")
    {
        const SubjectRequest sut{0, 1, 1, {}, {3, 1, 2, 2, 5, 0, 10, 5, 1, 3, 3}, {}};
        REQUIRE(sut.Groups() == std::vector<std::size_t>{0, 1, 2, 3, 5, 10});
    }
    SECTION("Sorting and removing duplicates from classrooms list while constructing")
    {
        const SubjectRequest sut{0, 1, 1, {}, {3},
                                  {{0, 1}, {1, 2},
                                    {1, 0}, {1, 2},
                                    {4, 5}, {0, 1}}};
        REQUIRE(sut.Classrooms() == std::vector<ClassroomAddress>({{0, 1}, {1, 0}, {1, 2}, {4, 5}}));
    }
}

TEST_CASE("ScheduleData construct correctly")
{
    const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
    SECTION("Sorting by id and removing duplicates while sorting from subject requests list while constructing")
    {
        const std::vector given{
            SubjectRequest(3, 4, 3, {}, {3}, classrooms),
            SubjectRequest(0, 1, 1, {}, {0}, classrooms),
            SubjectRequest(1, 2, 1, {}, {1}, classrooms),
            SubjectRequest(3, 4, 3, {}, {3}, classrooms),
            SubjectRequest(2, 3, 2, {}, {2}, classrooms),
            SubjectRequest(4, 5, 4, {}, {4}, classrooms),
            SubjectRequest(1, 2, 1, {}, {1}, classrooms)
        };

        const std::vector expected{
            SubjectRequest(0, 1, 1, {}, {0}, classrooms),
            SubjectRequest(1, 2, 1, {}, {1}, classrooms),
            SubjectRequest(2, 3, 2, {}, {2}, classrooms),
            SubjectRequest(3, 4, 3, {}, {3}, classrooms),
            SubjectRequest(4, 5, 4, {}, {4}, classrooms)
        };

        const ScheduleData sut{given, {}};
        REQUIRE(sut.SubjectRequests() == expected);
    }
    SECTION("Sorting by id and removing duplicates while sorting from locked lesson list while constructing")
    {
        const std::vector requests{
            SubjectRequest(0, 1, 1, {}, {0}, classrooms),
            SubjectRequest(1, 2, 1, {}, {1}, classrooms),
            SubjectRequest(2, 3, 2, {}, {2}, classrooms),
            SubjectRequest(3, 4, 3, {}, {3}, classrooms),
            SubjectRequest(4, 5, 4, {}, {4}, classrooms)
        };

        const std::vector given{
            SubjectWithAddress(5, 0),
            SubjectWithAddress(0, 0),
            SubjectWithAddress(2, 1),
            SubjectWithAddress(1, 0),
            SubjectWithAddress(0, 0),
            SubjectWithAddress(0, 0),
            SubjectWithAddress(3, 2),
            SubjectWithAddress(4, 1),
            SubjectWithAddress(1, 0)

        };

        const std::vector expected{
            SubjectWithAddress(0, 0),
            SubjectWithAddress(1, 0),
            SubjectWithAddress(2, 1),
            SubjectWithAddress(3, 2),
            SubjectWithAddress(4, 1),
            SubjectWithAddress(5, 0)
        };

        const ScheduleData sut{requests, given};
        REQUIRE(sut.LockedLessons() == expected);
    }
}

TEST_CASE("Check if classrooms overlaps", "[validation]")
{
    const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
    const std::vector subjectRequests{
        SubjectRequest(0, 1, 1, {}, {0}, classrooms),
        SubjectRequest(1, 2, 1, {}, {1}, classrooms),
        SubjectRequest(2, 3, 2, {}, {2}, classrooms),
        SubjectRequest(3, 4, 3, {}, {3}, classrooms),
        SubjectRequest(4, 5, 4, {}, {4}, classrooms)
    };
    const ScheduleData scheduleData{subjectRequests,{}};

    SECTION("No overlaps - empty overlaps list returned")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(1, 1, 1));
        scheduleResult.insert(ScheduleItem(2, 2, 2));
        scheduleResult.insert(ScheduleItem(3, 3, 3));
        scheduleResult.insert(ScheduleItem(4, 4, 3));

        const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
        REQUIRE(result.empty());
    }
    SECTION("If classroom doesn't matter (classroom = 0) - no overlaps here")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(0, 1, 0));
        scheduleResult.insert(ScheduleItem(0, 2, 1));
        scheduleResult.insert(ScheduleItem(0, 3, 2));
        scheduleResult.insert(ScheduleItem(0, 4, 3));

        const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
        REQUIRE(result.empty());
    }
    SECTION("Overlapped classroom found successfully")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(0, 1, 1));
        scheduleResult.insert(ScheduleItem(0, 2, 1));
        scheduleResult.insert(ScheduleItem(0, 3, 2));
        scheduleResult.insert(ScheduleItem(0, 4, 3));

        const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
        const OverlappedClassroom expected{.Address = 0, .Classroom = 1, .SubjectRequestsIDs = {1, 2}};
        REQUIRE(result.size() == 1);
        REQUIRE(result.front() == expected);
    }
}

TEST_CASE("Check if professors overlaps", "[validation]")
{
    const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
    const std::vector subjectRequests{
            SubjectRequest(0, 1, 1, {}, {1}, classrooms),
            SubjectRequest(1, 2, 1, {}, {2}, classrooms),
            SubjectRequest(2, 2, 2, {}, {3}, classrooms),
            SubjectRequest(3, 3, 3, {}, {4}, classrooms),
            SubjectRequest(4, 4, 4, {}, {5}, classrooms)
    };
    const ScheduleData scheduleData{subjectRequests, {}};

    SECTION("No overlaps - empty overlaps list returned")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(0, 1, 1));
        scheduleResult.insert(ScheduleItem(1, 2, 2));
        scheduleResult.insert(ScheduleItem(0, 3, 3));
        scheduleResult.insert(ScheduleItem(0, 4, 4));

        const auto result = FindOverlappedProfessors(scheduleData, scheduleResult);
        REQUIRE(result.empty());
    }
    SECTION("Overlapped professor found successfully")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(0, 1, 1));
        scheduleResult.insert(ScheduleItem(0, 2, 2));
        scheduleResult.insert(ScheduleItem(0, 3, 3));
        scheduleResult.insert(ScheduleItem(0, 4, 4));

        const auto result = FindOverlappedProfessors(scheduleData, scheduleResult);
        const OverlappedProfessor expected{.Address = 0, .Professor = 2, .SubjectRequestsIDs = {1, 2}};
        REQUIRE(result.size() == 1);
        REQUIRE(result.front() == expected);
    }
}

TEST_CASE("Check if groups overlaps", "[validation]")
{
    const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
    const std::vector subjectRequests{
        SubjectRequest(0, 1, 1, {}, {1, 3, 5}, classrooms),
        SubjectRequest(1, 2, 1, {}, {21, 4}, classrooms),
        SubjectRequest(2, 3, 2, {}, {0, 3, 2, 5}, classrooms),
        SubjectRequest(3, 4, 3, {}, {9, 10}, classrooms),
        SubjectRequest(4, 5, 4, {}, {11, 12}, classrooms)
    };
    const ScheduleData scheduleData{subjectRequests, {}};

    SECTION("No overlaps - empty overlaps list returned")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(0, 1, 1));
        scheduleResult.insert(ScheduleItem(1, 2, 2));
        scheduleResult.insert(ScheduleItem(0, 3, 3));
        scheduleResult.insert(ScheduleItem(0, 4, 4));

        const auto result = FindOverlappedGroups(scheduleData, scheduleResult);
        REQUIRE(result.empty());
    }
    SECTION("Overlapped groups found successfully")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(0, 1, 1));
        scheduleResult.insert(ScheduleItem(0, 2, 2));
        scheduleResult.insert(ScheduleItem(0, 3, 3));
        scheduleResult.insert(ScheduleItem(0, 4, 4));

        const auto result = FindOverlappedGroups(scheduleData, scheduleResult);
        const OverlappedGroups expected{.Address = 0, .Groups = {3, 5}, .SubjectRequestsIDs = {0, 2}};
        REQUIRE(result.size() == 1);
        REQUIRE(result.front() == expected);
    }
}

TEST_CASE("Check if weekday requests violated", "[validation]")
{
    const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
    const std::vector subjectRequests{
        SubjectRequest(0, 1, 1, {WeekDay::Monday}, {1}, classrooms),
        SubjectRequest(1, 2, 1, {WeekDay::Tuesday}, {2}, classrooms),
        SubjectRequest(2, 3, 2, {WeekDay::Wednesday}, {3}, classrooms),
        SubjectRequest(3, 4, 3, {WeekDay::Thursday}, {4}, classrooms),
        SubjectRequest(4, 5, 4, {WeekDay::Friday, WeekDay::Saturday}, {5}, classrooms)
    };
    const ScheduleData scheduleData{subjectRequests, {}};

    SECTION("No violations - empty list returned")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(MAX_LESSONS_PER_DAY, 1, 1));
        scheduleResult.insert(ScheduleItem(2 * MAX_LESSONS_PER_DAY, 2, 2));
        scheduleResult.insert(ScheduleItem(3 * MAX_LESSONS_PER_DAY + 2, 3, 3));
        scheduleResult.insert(ScheduleItem(4 * MAX_LESSONS_PER_DAY + 1, 4, 4));

        const auto result = FindViolatedWeekdayRequests(scheduleData, scheduleResult);
        REQUIRE(result.empty());
    }
    SECTION("Violated weekday found successfully")
    {
        ScheduleResult scheduleResult;
        scheduleResult.insert(ScheduleItem(0, 0, 0));
        scheduleResult.insert(ScheduleItem(MAX_LESSONS_PER_DAY, 1, 1));
        scheduleResult.insert(ScheduleItem(2 * MAX_LESSONS_PER_DAY, 2, 2));
        scheduleResult.insert(ScheduleItem(3 * MAX_LESSONS_PER_DAY + 2, 3, 3));
        scheduleResult.insert(ScheduleItem(2 * MAX_LESSONS_PER_DAY + 1, 4, 4));

        const auto result = FindViolatedWeekdayRequests(scheduleData, scheduleResult);
        const ViolatedWeekdayRequest expected{.Address = 2 * MAX_LESSONS_PER_DAY + 1,
                                               .SubjectRequestID = 4};
        REQUIRE(result.size() == 1);
        REQUIRE(result.front() == expected);
    }
}
