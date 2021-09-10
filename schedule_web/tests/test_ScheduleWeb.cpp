#include "ScheduleDataSerialization.h"
#include "GAScheduleGenerator.h"
#include <catch2/catch.hpp>


TEST_CASE("Parsing lessons set", "[parsing]")
{
    SECTION("Normal week days json array parsed successfully")
    {
        REQUIRE(ParseLessonsSet("[0, 1, 2, 3, 4, 5]"_json) == std::vector<std::size_t>{0, 1, 2, 3, 4, 5});
    }
    SECTION("If duplications found - they are ignored")
    {
        REQUIRE(ParseLessonsSet("[0, 0, 1, 2, 3, 3, 3]"_json) == std::vector<std::size_t>{0, 1, 2, 3});
    }
    SECTION("Negative value is incorrect")
    {
        std::vector<std::size_t> lessons;
        REQUIRE_THROWS_AS(lessons = ParseLessonsSet("[2, -3, 1, -4]"_json), std::out_of_range);
    }
    SECTION("Too big value is incorrect")
    {
        std::vector<std::size_t> lessons;
        REQUIRE_THROWS_AS(lessons = ParseLessonsSet("[84, 1, 3, 4, 5]"_json), std::out_of_range);
        REQUIRE_THROWS_AS(lessons = ParseLessonsSet("[0, 1, 3, 240, 5]"_json), std::out_of_range);
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
            "groups": [1, 2, 4, 5],
            "lessons": [0, 2, 4],
            "classrooms": [[2, 11], [3, 12]]
        })"_json;

        const SubjectRequest expected{
            0, 1, 2, {1, 2, 4, 5}, {0, 2, 4},
            {ClassroomAddress{.Building = 0, .Classroom = 2},
            ClassroomAddress{.Building = 0, .Classroom = 11},
            ClassroomAddress{.Building = 1, .Classroom = 3},
            ClassroomAddress{.Building = 1, .Classroom = 12}}
        };

        REQUIRE(request == expected);
    }
    SECTION("Subject request includes fields 'id', 'professor', 'complexity', 'groups', 'lessons' and 'classrooms'")
    {
        SubjectRequest request;
        REQUIRE_THROWS(request = "{}"_json);

        REQUIRE_THROWS(request = R"({
            "professor": 1,
            "complexity": 1,
            "groups": [1, 2, 4, 5],
            "lessons": [0, 2, 4],
            "classrooms": [[2, 11], [3, 12]]
        })"_json);

        REQUIRE_THROWS(request = R"({
            "id": 0,
            "complexity": 1,
            "groups": [1, 2, 4, 5],
            "lessons": [0, 2, 4],
            "classrooms": [[2, 11], [3, 12]]
        })"_json);

        REQUIRE_THROWS(request = R"({
            "id": 0,
            "professor": 1,
            "groups": [1, 2, 4, 5],
            "lessons": [0, 2, 4],
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
            "lessons": [0, 2, 4],
            "classrooms": [[2, 11], [3, 12]]
        })"_json);

        REQUIRE_THROWS(request = R"({
            "id": 0,
            "professor": 1,
            "complexity": 1,
            "groups": [1, 2, 4, 5],
            "lessons": [0, 2, 4],
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
