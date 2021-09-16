#include "ScheduleValidation.h"

#include <catch2/catch.hpp>


SCENARIO("Check if classrooms overlaps", "[validation]")
{
    GIVEN("ScheduleData with requests with some classrooms")
    {
        const std::vector<ClassroomAddress> classrooms{{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        // [id, professor, complexity, groups, lessons, classrooms]
        const ScheduleData scheduleData{{SubjectRequest{0, 1, 1, {0}, {}, classrooms},
                                         SubjectRequest{1, 2, 1, {1}, {}, classrooms},
                                         SubjectRequest{2, 3, 2, {2}, {}, classrooms},
                                         SubjectRequest{3, 4, 3, {3}, {}, classrooms},
                                         SubjectRequest{4, 5, 4, {4}, {}, classrooms}}};

        WHEN("no overlaps")
        {
            const ScheduleResult scheduleResult{
                {ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                 ScheduleItem{.Address = 1, .SubjectRequestID = 1, .Classroom = 1},
                 ScheduleItem{.Address = 2, .SubjectRequestID = 2, .Classroom = 2},
                 ScheduleItem{.Address = 3, .SubjectRequestID = 3, .Classroom = 3},
                 ScheduleItem{.Address = 4, .SubjectRequestID = 4, .Classroom = 3}}};

            THEN("empty overlaps list returned")
            {
                const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
                REQUIRE(std::empty(result));
            }
        }
        WHEN("classroom doesn't matter (classroom = 0)")
        {
            const ScheduleResult scheduleResult{
                {ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 0},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 2, .Classroom = 1},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 2},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 3}}};

            THEN("empty overlaps list returned")
            {
                const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
                REQUIRE(std::empty(result));
            }
        }
        WHEN("some classrooms overlaps")
        {
            const ScheduleResult scheduleResult{
                {ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 2, .Classroom = 1},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 2},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 3}}};

            THEN("overlapped classroom found successfully")
            {
                const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
                const OverlappedClassroom expected{
                    .Address = 0, .Classroom = 1, .SubjectRequestsIDs = {1, 2}};
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
        const std::vector<ClassroomAddress> classrooms{{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        // [id, professor, complexity, groups, lessons, classrooms]
        const ScheduleData scheduleData{{SubjectRequest{0, 1, 1, {1}, {}, classrooms},
                                         SubjectRequest{1, 2, 1, {2}, {}, classrooms},
                                         SubjectRequest{2, 2, 2, {3}, {}, classrooms},
                                         SubjectRequest{3, 3, 3, {4}, {}, classrooms},
                                         SubjectRequest{4, 4, 4, {5}, {}, classrooms}}};

        WHEN("no overlaps")
        {
            const ScheduleResult scheduleResult{
                {ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                 ScheduleItem{.Address = 1, .SubjectRequestID = 2, .Classroom = 2},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 3},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 4}}};

            THEN("empty overlaps list returned")
            {
                const auto result = FindOverlappedProfessors(scheduleData, scheduleResult);
                REQUIRE(result.empty());
            }
        }
        SECTION("some professors overlapped")
        {
            const ScheduleResult scheduleResult{
                {ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 2, .Classroom = 2},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 3},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 4}}};

            THEN("overlapped professor found successfully")
            {
                const auto result = FindOverlappedProfessors(scheduleData, scheduleResult);
                const OverlappedProfessor expected{
                    .Address = 0, .Professor = 2, .SubjectRequestsIDs = {1, 2}};
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
        const std::vector<ClassroomAddress> classrooms{{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        // [id, professor, complexity, groups, lessons, classrooms]
        const ScheduleData scheduleData{{SubjectRequest{0, 1, 1, {1, 3, 5}, {}, classrooms},
                                         SubjectRequest{1, 2, 1, {4, 21}, {}, classrooms},
                                         SubjectRequest{2, 3, 2, {0, 2, 3, 5}, {}, classrooms},
                                         SubjectRequest{3, 4, 3, {9, 10}, {}, classrooms},
                                         SubjectRequest{4, 5, 4, {11, 12}, {}, classrooms}}};

        WHEN("no overlaps")
        {
            const ScheduleResult scheduleResult{
                {ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                 ScheduleItem{.Address = 1, .SubjectRequestID = 2, .Classroom = 2},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 3},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 4}}};

            THEN("empty overlaps list returned")
            {
                const auto result = FindOverlappedGroups(scheduleData, scheduleResult);
                REQUIRE(result.empty());
            }
        }
        WHEN("groups overlaps")
        {
            const ScheduleResult scheduleResult{
                {ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 0},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 1, .Classroom = 1},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 2, .Classroom = 2},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 3, .Classroom = 3},
                 ScheduleItem{.Address = 0, .SubjectRequestID = 4, .Classroom = 4}}};

            THEN("overlapped groups found successfully")
            {
                const auto result = FindOverlappedGroups(scheduleData, scheduleResult);
                const OverlappedGroups expected{
                    .Address = 0, .Groups = {3, 5}, .SubjectRequestsIDs = {0, 2}};
                REQUIRE(result.size() == 1);
                REQUIRE(result.front() == expected);
            }
        }
    }
}
