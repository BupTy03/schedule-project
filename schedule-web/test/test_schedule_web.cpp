#include "ScheduleDataSerialization.h"
#include <catch2/catch.hpp>


TEST_CASE("Parsing lesson address", "[parsing]")
{
    SECTION("Normal lesson address json parsed successfully")
    {
        LessonAddress lessonAddress = nlohmann::json::parse(R"({"group": 1, "lesson": 5})");
        REQUIRE(lessonAddress == LessonAddress(1, 5));
    }
    SECTION("Lesson address includes fields 'group' and 'lesson'")
    {
        LessonAddress lessonAddress;
        REQUIRE_THROWS(lessonAddress = nlohmann::json::object());
        REQUIRE_THROWS(lessonAddress = nlohmann::json::object({{"group", 1}}));
        REQUIRE_THROWS(lessonAddress = nlohmann::json::object({{"lesson", 5}}));
    }
}

TEST_CASE("Parsing locked lessons", "[parsing]")
{
    SECTION("Normal locked lesson json parsed successfully")
    {
        auto address = nlohmann::json::object({{"group", 5}, {"lesson", 3}});
        SubjectWithAddress lockedLesson = nlohmann::json::object({{"address", address}, {"subject_request", 3}});
        REQUIRE(lockedLesson == SubjectWithAddress(3, LessonAddress(5, 3)));
    }
    SECTION("Locked lesson includes fields 'address' and 'lesson'")
    {
        auto address = nlohmann::json::object({{"group", 5}, {"lesson", 3}});

        SubjectWithAddress lockedLesson;
        REQUIRE_THROWS(lockedLesson = nlohmann::json::object());
        REQUIRE_THROWS(lockedLesson = nlohmann::json::object({{"address", address}}));
        REQUIRE_THROWS(lockedLesson = nlohmann::json::object({{"subject_request", 5}}));
    }
}

TEST_CASE("Parsing week days set", "[parsing]")
{
    SECTION("Normal week days json array parsed successfully")
    {
        const WeekDays weekDays = nlohmann::json::array({0, 1, 2, 3, 4, 5});
        REQUIRE(weekDays == WeekDays::fullWeek());
    }
    SECTION("If duplications found - they are ignored")
    {
        const WeekDays weekDays = nlohmann::json::array({0, 0, 1, 2, 3, 3, 3});
        REQUIRE(weekDays == WeekDays({WeekDay::Monday, WeekDay::Tuesday, WeekDay::Wednesday, WeekDay::Thursday}));
    }
    SECTION("Negative value is incorrect")
    {
        WeekDays weekDays;
        REQUIRE_THROWS_AS(weekDays = nlohmann::json::array({2, -3, 1, -4}), std::invalid_argument);
    }
    SECTION("Too big value is incorrect")
    {
        WeekDays weekDays;
        REQUIRE_THROWS_AS(weekDays = nlohmann::json::array({2, 6, 1, 0}), std::invalid_argument);
    }
}

TEST_CASE("Parsing IDs set", "[parsing]")
{
    SECTION("Normal IDs json array parsed successfully")
    {
        const std::vector<std::size_t> ids = ParseIDsSet(nlohmann::json::array({0, 1, 2, 3, 4, 5}));
        REQUIRE(ids == SortedSet<std::size_t>({0, 1, 2, 3, 4, 5}));
    }
    SECTION("If array is empty - IDs set is empty too")
    {
        const std::vector<std::size_t> ids = ParseIDsSet(nlohmann::json::array());
        REQUIRE(ids == std::vector<std::size_t>());
    }
    SECTION("If duplications found - they are ignored")
    {
        const std::vector<std::size_t> ids = ParseIDsSet(nlohmann::json::array({1, 2, 2, 0, 1, 0, 5, 5, 5}));
        REQUIRE(ids == std::vector<std::size_t>({0, 1, 2, 5}));
    }
    SECTION("Negative value is incorrect")
    {
        std::vector<std::size_t> ids;
        REQUIRE_THROWS_AS(ids = ParseIDsSet(nlohmann::json::array({2, -3, 1, -4})), std::invalid_argument);
    }
}

TEST_CASE("Parsing subject request", "[parsing]")
{
    SECTION("Normal subject request json parsed successfully")
    {
        const auto subjectRequest = nlohmann::json::object({
            {"id", 0},
            {"professor", 1},
            {"complexity", 2},
            {"days", nlohmann::json::array({0, 2, 4})},
            {"groups", nlohmann::json::array({1, 2, 4, 5})},
            {"classrooms", nlohmann::json::array({nlohmann::json::array({2, 11}), nlohmann::json::array({3, 12})})}
        });

        SubjectRequest request = subjectRequest;
        REQUIRE(request == SubjectRequest(0, 1, 2, {WeekDay::Monday, WeekDay::Wednesday, WeekDay::Friday}, {1, 2, 4, 5},
                                          {ClassroomAddress(0, 2),
                                            ClassroomAddress(0, 11),
                                            ClassroomAddress(1, 3),
                                            ClassroomAddress(1, 12)}));
    }
    SECTION("Subject request includes fields 'id', 'professor', 'complexity', 'days', 'groups' and 'classrooms'")
    {
        auto days = nlohmann::json::array({0, 2, 4});
        auto groups = nlohmann::json::array({0, 2, 4});
        auto classrooms = nlohmann::json::array({2, 11});

        SubjectRequest request;
        REQUIRE_THROWS(request = nlohmann::json::object());

        REQUIRE_THROWS(request = nlohmann::json::object({ { "professor", 1 },
                                                             { "complexity", 1 },
                                                             { "days", nlohmann::json::array({ 0, 2, 4 }) },
                                                             { "groups", nlohmann::json::array({ 1, 2, 4, 5 }) },
                                                             { "classrooms", nlohmann::json::array({ nlohmann::json::array({ 2, 11 }), nlohmann::json::array({ 3, 12 }) }) } }));

        REQUIRE_THROWS(request = nlohmann::json::object({ { "id", 0 },
                                                             { "complexity", 1 },
                                                             { "days", nlohmann::json::array({ 0, 2, 4 }) },
                                                             { "groups", nlohmann::json::array({ 1, 2, 4, 5 }) },
                                                             { "classrooms", nlohmann::json::array({ nlohmann::json::array({ 2, 11 }), nlohmann::json::array({ 3, 12 }) }) } }));

        REQUIRE_THROWS(request = nlohmann::json::object({ { "id", 0 },
                                                             { "professor", 1 },
                                                             { "days", nlohmann::json::array({ 0, 2, 4 }) },
                                                             { "groups", nlohmann::json::array({ 1, 2, 4, 5 }) },
                                                             { "classrooms", nlohmann::json::array({ nlohmann::json::array({ 2, 11 }), nlohmann::json::array({ 3, 12 }) }) } }));

        REQUIRE_THROWS(request = nlohmann::json::object({ { "id", 0 },
                                                             { "professor", 1 },
                                                             { "complexity", 1 },
                                                             { "groups", nlohmann::json::array({ 1, 2, 4, 5 }) },
                                                             { "classrooms", nlohmann::json::array({ nlohmann::json::array({ 2, 11 }), nlohmann::json::array({ 3, 12 }) }) } }));

        REQUIRE_THROWS(request = nlohmann::json::object({ { "id", 0 },
                                                             { "professor", 1 },
                                                             { "complexity", 1 },
                                                             { "days", nlohmann::json::array({ 0, 2, 4 }) },
                                                             { "classrooms", nlohmann::json::array({ nlohmann::json::array({ 2, 11 }), nlohmann::json::array({ 3, 12 }) }) } }));

        REQUIRE_THROWS(request = nlohmann::json::object({
                              { "id", 0 },
                              { "professor", 1 },
                              { "complexity", 1 },
                              { "days", nlohmann::json::array({ 0, 2, 4 }) },
                              { "groups", nlohmann::json::array({ 1, 2, 4, 5 }) },
                          }));
    }
}

TEST_CASE("Parsing schedule data", "[parsing]")
{
    SECTION("Schedule data includes fields 'subject_requests' ('locked_lessons' is optional)")
    {
        ScheduleData data;
        REQUIRE_THROWS(data = nlohmann::json::object({{"locked_lessons", nlohmann::json::array()}}));
    }
    SECTION("'subject_requests' array must not be empty")
    {
        ScheduleData data;
        REQUIRE_THROWS(data = nlohmann::json::object({{"subject_requests", nlohmann::json::array()}}));
    }
}

TEST_CASE("Schedule item serialized successfully", "[serializing]")
{
    ScheduleItem scheduleItem = nlohmann::json::object({{"address", 7},
                                                        {"subject_request_id", 1},
                                                        {"classroom", 4}});

    REQUIRE(scheduleItem.Address == 7);
    REQUIRE(scheduleItem.SubjectRequestID == 1);
    REQUIRE(scheduleItem.Classroom == 4);
}
