#include "ScheduleUtils.h"
#include "ScheduleCommon.h"
#include "ScheduleData.h"
#include "ScheduleResult.h"

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


TEST_CASE("SubjectRequest constructs correctly", "[SubjectRequest]")
{
    SECTION("Sorting and removing duplicates from groups list while constructing")
    {
        // set_intersects algorithm REQUIRES sorted groups
        const SubjectRequest sut{0, 1, 1, {}, {3, 1, 2, 2, 5, 0, 10, 5, 1, 3, 3}, {}};
        REQUIRE(sut.Groups() == std::vector<std::size_t>{0, 1, 2, 3, 5, 10});
    }
}

TEST_CASE("Week day requested check performs correctly", "[SubjectRequest]")
{
    const SubjectRequest sut{0, 1, 1, {WeekDay::Monday, WeekDay::Wednesday, WeekDay::Thursday},
                              {1, 2, 3}, {{0,0}}};

    // Monday numerator/denominator is requested
    REQUIRE(sut.RequestedWeekDay(0));
    REQUIRE(sut.RequestedWeekDay(6));

    // Wednesday numerator/denominator is requested
    REQUIRE(sut.RequestedWeekDay(2));
    REQUIRE(sut.RequestedWeekDay(8));

    // Thursday numerator/denominator is requested
    REQUIRE(sut.RequestedWeekDay(3));
    REQUIRE(sut.RequestedWeekDay(9));

    // Other days are not requested
    for(int wd : {1, 4, 5, 7, 10, 11})
        REQUIRE_FALSE(sut.RequestedWeekDay(wd));
}

TEST_CASE("Search by subject id performs correctly", "[ScheduleData]")
{
    const std::vector subjectRequests{
        SubjectRequest{0, 1, 1, {}, {0}, {}},
        SubjectRequest{1, 2, 1, {}, {1}, {}},
        SubjectRequest{2, 3, 2, {}, {2}, {}},
        SubjectRequest{3, 4, 3, {}, {3}, {}},
        SubjectRequest{4, 5, 4, {}, {4}, {}}
    };
    const ScheduleData sut{subjectRequests,{}};

    SECTION("Searching subject request by id")
    {
        for(auto&& request : sut.SubjectRequests())
            REQUIRE(sut.SubjectRequestAtID(request.ID()) == request);

        REQUIRE_THROWS(sut.SubjectRequestAtID(5));
        REQUIRE_THROWS(sut.SubjectRequestAtID(101));
    }
    SECTION("Searching index in requests list by subject id works")
    {
        for(std::size_t i = 0; i < sut.SubjectRequests().size(); ++i)
        {
            auto&& request = sut.SubjectRequests().at(i);
            REQUIRE(sut.IndexOfSubjectRequestWithID(request.ID()) == i);
        }

        REQUIRE_THROWS(sut.IndexOfSubjectRequestWithID(5));
        REQUIRE_THROWS(sut.IndexOfSubjectRequestWithID(100));
    }
}

TEST_CASE("Check if subject request has locked lesson works", "[ScheduleData]")
{
    const std::vector subjectRequests{
        SubjectRequest{0, 1, 1, {}, {0}, {}},
        SubjectRequest{1, 2, 1, {}, {1}, {}},
        SubjectRequest{2, 3, 2, {}, {2}, {}},
        SubjectRequest{3, 4, 3, {}, {3}, {}},
        SubjectRequest{4, 5, 4, {}, {4}, {}}
    };
    const std::vector lockedLessons {
        SubjectWithAddress{0, 0},
        SubjectWithAddress{3, 2},
        SubjectWithAddress{4, 1}
    };
    const ScheduleData sut{subjectRequests, lockedLessons};


    REQUIRE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(0)));
    REQUIRE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(3)));
    REQUIRE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(4)));

    REQUIRE_FALSE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(1)));
    REQUIRE_FALSE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(2)));
}

SCENARIO("Check if classrooms overlaps", "[validation]")
{
    GIVEN("ScheduleData with requests with some classrooms")
    {
        const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        const std::vector subjectRequests{
            SubjectRequest{0, 1, 1, {}, {0}, classrooms},
            SubjectRequest{1, 2, 1, {}, {1}, classrooms},
            SubjectRequest{2, 3, 2, {}, {2}, classrooms},
            SubjectRequest{3, 4, 3, {}, {3}, classrooms},
            SubjectRequest{4, 5, 4, {}, {4}, classrooms}
        };
        const ScheduleData scheduleData{subjectRequests,{}};

        WHEN("no overlaps")
        {
            const ScheduleResult scheduleResult{ {
                ScheduleItem{0, 0, 0},
                ScheduleItem{1, 1, 1},
                ScheduleItem{2, 2, 2},
                ScheduleItem{3, 3, 3},
                ScheduleItem{4, 4, 3}
            }};

            THEN("empty overlaps list returned")
            {
                const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
                REQUIRE(std::empty(result));
            }
        }
        WHEN("classroom doesn't matter (classroom = 0)")
        {
            const ScheduleResult scheduleResult {{
                ScheduleItem{0, 0, 0},
                ScheduleItem{0, 1, 0},
                ScheduleItem{0, 2, 1},
                ScheduleItem{0, 3, 2},
                ScheduleItem{0, 4, 3}
            }};

            THEN("empty overlaps list returned")
            {
                const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
                REQUIRE(std::empty(result));
            }
        }
        WHEN("some classrooms overlaps")
        {
            const ScheduleResult scheduleResult{{
                ScheduleItem{0, 0, 0},
                ScheduleItem{0, 1, 1},
                ScheduleItem{0, 2, 1},
                ScheduleItem{0, 3, 2},
                ScheduleItem{0, 4, 3}
            }};

            THEN("overlapped classroom found successfully")
            {
                const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
                const OverlappedClassroom expected{
                    .Address = 0,
                    .Classroom = 1,
                    .SubjectRequestsIDs = {1, 2}
                };
                REQUIRE(result.size() == 1);
                REQUIRE(result.front() == expected);
            }
        }
    }
}

SCENARIO("Check if professors overlaps", "[validation]")
{
    GIVEN("ScheduleData with requests with some professors")
    {
        const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        const std::vector subjectRequests{
            SubjectRequest{0, 1, 1, {}, {1}, classrooms},
            SubjectRequest{1, 2, 1, {}, {2}, classrooms},
            SubjectRequest{2, 2, 2, {}, {3}, classrooms},
            SubjectRequest{3, 3, 3, {}, {4}, classrooms},
            SubjectRequest{4, 4, 4, {}, {5}, classrooms}
        };
        const ScheduleData scheduleData{subjectRequests, {}};

        WHEN("no overlaps")
        {
            const ScheduleResult scheduleResult{{
                ScheduleItem{0, 0, 0},
                ScheduleItem{0, 1, 1},
                ScheduleItem{1, 2, 2},
                ScheduleItem{0, 3, 3},
                ScheduleItem{0, 4, 4}
            }};

            THEN("empty overlaps list returned")
            {
                const auto result = FindOverlappedProfessors(scheduleData, scheduleResult);
                REQUIRE(result.empty());
            }
        }
        SECTION("some professors overlapped")
        {
            const ScheduleResult scheduleResult{{
                ScheduleItem{0, 0, 0},
                ScheduleItem{0, 1, 1},
                ScheduleItem{0, 2, 2},
                ScheduleItem{0, 3, 3},
                ScheduleItem{0, 4, 4}
            }};

            THEN("overlapped professor found successfully")
            {
                const auto result = FindOverlappedProfessors(scheduleData, scheduleResult);
                const OverlappedProfessor expected{
                    .Address = 0,
                    .Professor = 2,
                    .SubjectRequestsIDs = {1, 2}
                };
                REQUIRE(result.size() == 1);
                REQUIRE(result.front() == expected);
            }
        }
    }
}

SCENARIO("Check if groups overlaps", "[validation]")
{
    GIVEN("ScheduleData with requests with some groups")
    {
        const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        const std::vector subjectRequests{
            SubjectRequest{0, 1, 1, {}, {1, 3, 5}, classrooms},
            SubjectRequest{1, 2, 1, {}, {21, 4}, classrooms},
            SubjectRequest{2, 3, 2, {}, {0, 3, 2, 5}, classrooms},
            SubjectRequest{3, 4, 3, {}, {9, 10}, classrooms},
            SubjectRequest{4, 5, 4, {}, {11, 12}, classrooms}
        };
        const ScheduleData scheduleData{subjectRequests, {}};

        WHEN("no overlaps")
        {
            const ScheduleResult scheduleResult{{
                ScheduleItem{0, 0, 0},
                ScheduleItem{0, 1, 1},
                ScheduleItem{1, 2, 2},
                ScheduleItem{0, 3, 3},
                ScheduleItem{0, 4, 4}
            }};

            THEN("empty overlaps list returned")
            {
                const auto result = FindOverlappedGroups(scheduleData, scheduleResult);
                REQUIRE(result.empty());
            }
        }
        WHEN("groups overlaps")
        {
            const ScheduleResult scheduleResult{{
                ScheduleItem{0, 0, 0},
                ScheduleItem{0, 1, 1},
                ScheduleItem{0, 2, 2},
                ScheduleItem{0, 3, 3},
                ScheduleItem{0, 4, 4}
            }};

            THEN("overlapped groups found successfully")
            {
                const auto result = FindOverlappedGroups(scheduleData, scheduleResult);
                const OverlappedGroups expected{
                    .Address = 0,
                    .Groups = {3, 5},
                    .SubjectRequestsIDs = {0, 2}
                };
                REQUIRE(result.size() == 1);
                REQUIRE(result.front() == expected);
            }
        }
    }
}

SCENARIO("Check if weekday requests violated", "[validation]")
{
    GIVEN("ScheduleData with subject requests with weekday requests")
    {
        const std::vector<ClassroomAddress> classrooms {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        const std::vector subjectRequests{
            SubjectRequest{0, 1, 1, {WeekDay::Monday}, {1}, classrooms},
            SubjectRequest{1, 2, 1, {WeekDay::Tuesday}, {2}, classrooms},
            SubjectRequest{2, 3, 2, {WeekDay::Wednesday}, {3}, classrooms},
            SubjectRequest{3, 4, 3, {WeekDay::Thursday}, {4}, classrooms},
            SubjectRequest{4, 5, 4, {WeekDay::Friday, WeekDay::Saturday}, {5}, classrooms}
        };
        const ScheduleData scheduleData{subjectRequests, {}};

        WHEN("no violations")
        {
            const ScheduleResult scheduleResult{{
                ScheduleItem{0, 0, 0},
                ScheduleItem{MAX_LESSONS_PER_DAY, 1, 1},
                ScheduleItem{2 * MAX_LESSONS_PER_DAY, 2, 2},
                ScheduleItem{3 * MAX_LESSONS_PER_DAY + 2, 3, 3},
                ScheduleItem{4 * MAX_LESSONS_PER_DAY + 1, 4, 4}
            }};

            THEN("empty list returned")
            {
                const auto result = FindViolatedWeekdayRequests(scheduleData, scheduleResult);
                REQUIRE(result.empty());
            }
        }
        WHEN("some violated weekdays")
        {
            const ScheduleResult scheduleResult{{
                ScheduleItem{0, 0, 0},
                ScheduleItem{MAX_LESSONS_PER_DAY, 1, 1},
                ScheduleItem{2 * MAX_LESSONS_PER_DAY, 2, 2},
                ScheduleItem{3 * MAX_LESSONS_PER_DAY + 2, 3, 3},
                ScheduleItem{2 * MAX_LESSONS_PER_DAY + 1, 4, 4}
            }};

            THEN("violated weekday found successfully")
            {
                const auto result = FindViolatedWeekdayRequests(scheduleData, scheduleResult);
                const ViolatedWeekdayRequest expected{
                    .Address = 2 * MAX_LESSONS_PER_DAY + 1,
                    .SubjectRequestID = 4
                };
                REQUIRE(result.size() == 1);
                REQUIRE(result.front() == expected);
            }
        }
    }
}
