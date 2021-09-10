#include "ScheduleUtils.h"
#include "ScheduleCommon.h"
#include "ScheduleData.h"
#include "ScheduleResult.h"
#include <catch2/catch.hpp>
#include <array>


TEST_CASE("SubjectRequest constructs correctly", "[subject_request]")
{
    SECTION("Sorting and removing duplicates from groups list while constructing")
    {
        // set_intersects algorithm REQUIRES sorted groups
        const SubjectRequest sut{0, 1, 1, {}, {3, 1, 2, 2, 5, 0, 10, 5, 1, 3, 3}, {}};
        REQUIRE(sut.Groups() == std::vector<std::size_t>{0, 1, 2, 3, 5, 10});
    }
}

TEST_CASE("Week day requested check performs correctly", "[subject_request]")
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

TEST_CASE("Search by subject id performs correctly", "[schedule_data]")
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

TEST_CASE("Check if subject request has locked lesson works", "[schedule_data]")
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = 1, .SubjectRequestID = 1, .Classroom = 1},
                ScheduleItem{.Address = 2, .SubjectRequestID = 2, .Classroom = 2},
                ScheduleItem{.Address = 3, .SubjectRequestID = 3, .Classroom = 3},
                ScheduleItem{.Address = 4, .SubjectRequestID = 4, .Classroom = 3}
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 0},
                ScheduleItem{.Address = 0, .SubjectRequestID = 2, .Classroom = 1},
                ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 2},
                ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 3}
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                ScheduleItem{.Address = 0, .SubjectRequestID = 2, .Classroom = 1},
                ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 2},
                ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 3}
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                ScheduleItem{.Address = 1, .SubjectRequestID = 2, .Classroom = 2},
                ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 3},
                ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 4}
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                ScheduleItem{.Address = 0, .SubjectRequestID = 2, .Classroom = 2},
                ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 3},
                ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 4}
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                ScheduleItem{.Address = 1, .SubjectRequestID = 2, .Classroom = 2},
                ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 3},
                ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 4}
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                ScheduleItem{.Address = 0, .SubjectRequestID = 2, .Classroom = 2},
                ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 3},
                ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 4}
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = MAX_LESSONS_PER_DAY, .SubjectRequestID = 1, .Classroom = 1},
                ScheduleItem{.Address = 2 * MAX_LESSONS_PER_DAY, .SubjectRequestID = 2, .Classroom = 2},
                ScheduleItem{.Address = 3 * MAX_LESSONS_PER_DAY + 2, .SubjectRequestID = 3, .Classroom = 3},
                ScheduleItem{.Address = 4 * MAX_LESSONS_PER_DAY + 1, .SubjectRequestID = 4, .Classroom = 4}
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
                ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                ScheduleItem{.Address = MAX_LESSONS_PER_DAY, .SubjectRequestID = 1, .Classroom = 1},
                ScheduleItem{.Address = 2 * MAX_LESSONS_PER_DAY, .SubjectRequestID = 2, .Classroom = 2},
                ScheduleItem{.Address = 3 * MAX_LESSONS_PER_DAY + 2, .SubjectRequestID = 3, .Classroom = 3},
                ScheduleItem{.Address = 2 * MAX_LESSONS_PER_DAY + 1, .SubjectRequestID = 4, .Classroom = 4}
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
