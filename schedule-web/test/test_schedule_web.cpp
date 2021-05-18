#include "ScheduleRequestHandler.h"
#include <catch2/catch.hpp>


TEST_CASE("Parsing invalid json", "[parsing]")
{
    SECTION("Parsing empty string")
    {
        REQUIRE_THROWS(nlohmann::json::parse(""));
    }
    SECTION("Parsing invalid json string")
    {
        REQUIRE_THROWS(nlohmann::json::parse("{sdfsdf]"));
        REQUIRE_THROWS(nlohmann::json::parse("<xml><tag></tag></xml>"));
    }

}

TEST_CASE("Parsing json field", "[parsing]")
{
    SECTION("Field exists - ok")
    {
        nlohmann::json field = RequireField(nlohmann::json::parse(R"({"group": 1, "lesson": 5})"), "group");
        REQUIRE(field.get<int>() == 1);
    }
    SECTION("Throw if field has invalid type")
    {
        nlohmann::json field = RequireField(nlohmann::json::parse(R"({"a": "ABC"})"), "a");
        REQUIRE_THROWS(field.get<int>());
    }
    SECTION("Throw std::invalid_argument if field is not exists")
    {
        nlohmann::json field;
        REQUIRE_THROWS_AS(field = RequireField(nlohmann::json::parse(R"({"a": 1, "b": 5})"), "c"), std::invalid_argument);
    }
}

TEST_CASE("Parsing lesson address", "[parsing]")
{
    SECTION("Normal lesson address json parsed successfully")
    {
        auto lessonAddress = ParseLessonAddress(nlohmann::json::parse(R"({"group": 1, "lesson": 5})"));
        REQUIRE(lessonAddress == LessonAddress(1, 5));
    }
    SECTION("Lesson address includes fields 'group' and 'lesson'")
    {
        LessonAddress lessonAddress;
        REQUIRE_THROWS_AS(lessonAddress = ParseLessonAddress(nlohmann::json::object()), std::invalid_argument);
        REQUIRE_THROWS_AS(lessonAddress = ParseLessonAddress(nlohmann::json::object({{"group", 1}})), std::invalid_argument);
        REQUIRE_THROWS_AS(lessonAddress = ParseLessonAddress(nlohmann::json::object({{"lesson", 5}})), std::invalid_argument);
    }
}

TEST_CASE("Parsing locked lessons", "[parsing]")
{
    SECTION("Normal locked lesson json parsed successfully")
    {
        auto address = nlohmann::json::object({{"group", 5}, {"lesson", 3}});
        SubjectWithAddress lockedLesson = ParseLockedLesson(nlohmann::json::object({{"address", address}, {"subject_request", 3}}));
        REQUIRE(lockedLesson == SubjectWithAddress(3, LessonAddress(5, 3)));
    }
    SECTION("Locked lesson includes fields 'address' and 'lesson'")
    {
        auto address = nlohmann::json::object({{"group", 5}, {"lesson", 3}});

        SubjectWithAddress lockedLesson;
        REQUIRE_THROWS_AS(lockedLesson = ParseLockedLesson(nlohmann::json::object()), std::invalid_argument);
        REQUIRE_THROWS_AS(lockedLesson = ParseLockedLesson(nlohmann::json::object({{"address", address}})), std::invalid_argument);
        REQUIRE_THROWS_AS(lockedLesson = ParseLockedLesson(nlohmann::json::object({{"subject_request", 5}})), std::invalid_argument);
    }
}
