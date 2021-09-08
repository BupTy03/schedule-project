#include "ScheduleDataSerialization.h"
#include <catch2/catch.hpp>


TEST_CASE("Parsing locked lessons", "[parsing]")
{
    SECTION("Normal locked lesson json parsed successfully")
    {
        const SubjectWithAddress lockedLesson = R"({"address": 5, "subject_request_id": 3})"_json;
        REQUIRE(lockedLesson == SubjectWithAddress{.SubjectRequestID = 3, .Address = 5});
    }
    SECTION("Locked lesson includes fields 'address' and 'lesson'")
    {
        SubjectWithAddress lockedLesson;
        REQUIRE_THROWS(lockedLesson = "{}"_json);
        REQUIRE_THROWS(lockedLesson = R"({"address": 5})"_json);
        REQUIRE_THROWS(lockedLesson = R"({"subject_request_id": 3})"_json);
    }
}

TEST_CASE("Parsing week days set", "[parsing]")
{
    SECTION("Normal week days json array parsed successfully")
    {
        const WeekDays weekDays = "[0, 1, 2, 3, 4, 5]"_json;
        REQUIRE(weekDays == WeekDays::fullWeek());
    }
    SECTION("If duplications found - they are ignored")
    {
        const WeekDays weekDays = "[0, 0, 1, 2, 3, 3, 3]"_json;
        REQUIRE(weekDays == WeekDays({WeekDay::Monday, WeekDay::Tuesday, WeekDay::Wednesday, WeekDay::Thursday}));
    }
    SECTION("Negative value is incorrect")
    {
        WeekDays weekDays;
        REQUIRE_THROWS_AS(weekDays = "[2, -3, 1, -4]"_json, std::invalid_argument);
    }
    SECTION("Too big value is incorrect")
    {
        WeekDays weekDays;
        REQUIRE_THROWS_AS(weekDays = "[2, 6, 1, 0]"_json, std::invalid_argument);
    }
}

TEST_CASE("Parsing IDs set", "[parsing]")
{
    SECTION("Normal IDs json array parsed successfully")
    {
        const auto ids = ParseIDsSet("[0, 1, 2, 3, 4, 5]"_json);
        REQUIRE(ids == std::vector<std::size_t>({0, 1, 2, 3, 4, 5}));
    }
    SECTION("If array is empty - IDs set is empty too")
    {
        const auto ids = ParseIDsSet("[]"_json);
        REQUIRE(ids.empty());
    }
    SECTION("If duplications found - they are ignored")
    {
        const auto ids = ParseIDsSet("[1, 2, 2, 0, 1, 0, 5, 5, 5]"_json);
        REQUIRE(ids == std::vector<std::size_t>({0, 1, 2, 5}));
    }
    SECTION("Negative value is incorrect")
    {
        std::vector<std::size_t> ids;
        REQUIRE_THROWS_AS(ids = ParseIDsSet("[2, -3, 1, -4]"_json), std::invalid_argument);
    }
}

TEST_CASE("Parsing subject request", "[parsing]")
{
    SECTION("Normal subject request json parsed successfully")
    {
        const SubjectRequest request = R"({
            "id": 0,
            "professor": 1,
            "complexity": 2,
            "days": [0, 2, 4],
            "groups": [1, 2, 4, 5],
            "classrooms": [[2, 11], [3, 12]]
        })"_json;


        REQUIRE(request == SubjectRequest(0, 1, 2, {WeekDay::Monday, WeekDay::Wednesday, WeekDay::Friday}, {1, 2, 4, 5},
                                          {ClassroomAddress{.Building = 0, .Classroom = 2},
                                            ClassroomAddress{.Building = 0, .Classroom = 11},
                                            ClassroomAddress{.Building = 1, .Classroom = 3},
                                            ClassroomAddress{.Building = 1, .Classroom = 12}}));
    }
    SECTION("Subject request includes fields 'id', 'professor', 'complexity', 'days', 'groups' and 'classrooms'")
    {
        SubjectRequest request;
        REQUIRE_THROWS(request = "{}"_json);

        REQUIRE_THROWS(request = R"({
            "professor": 1,
            "complexity": 1,
            "days": [0, 2, 4],
            "groups": [1, 2, 4, 5],
            "classrooms": [[2, 11], [3, 12]]
        })"_json);

        REQUIRE_THROWS(request = R"({
            "id": 0,
            "complexity": 1,
            "days": [0, 2, 4],
            "groups": [1, 2, 4, 5],
            "classrooms": [[2, 11], [3, 12]]
        })"_json);

        REQUIRE_THROWS(request = R"({
            "id": 0,
            "professor": 1,
            "days": [0, 2, 4],
            "groups": [1, 2, 4, 5],
            "classrooms": [[2, 11], [3, 12]]
        })"_json);

        REQUIRE_THROWS(request = R"({
            "id": 0,
            "professor": 1,
            "complexity": 1,
            "groups": [1, 2, 4, 5],
            "classrooms": [[2, 11], [3, 12]]
        })"_json);

        REQUIRE_THROWS(request = R"({
            "id": 0,
            "professor": 1,
            "complexity": 1,
            "days": [0, 2, 4],
            "classrooms": [[2, 11], [3, 12]]
        })"_json);

        REQUIRE_THROWS(request = R"({
            "id": 0,
            "professor": 1,
            "complexity": 1,
            "days": [0, 2, 4],
            "groups": [1, 2, 4, 5],
        })"_json);
    }
}

TEST_CASE("Parsing schedule data", "[parsing]")
{
    SECTION("Schedule data includes fields 'subject_requests' ('locked_lessons' is optional)")
    {
        ScheduleData data;
        REQUIRE_THROWS(data = R"({"locked_lessons": []})"_json);
    }
    SECTION("'subject_requests' array must not be empty")
    {
        ScheduleData data;
        REQUIRE_THROWS(data = R"({"subject_requests": []})"_json);
    }
}

TEST_CASE("Parsing schedule item", "[parsing]")
{
    const ScheduleItem scheduleItem = R"({
        "address": 7,
        "subject_request_id": 1,
        "classroom": 4
    })"_json;

    REQUIRE(scheduleItem == ScheduleItem{.Address = 7, .SubjectRequestID = 1, .Classroom = 4});
}
