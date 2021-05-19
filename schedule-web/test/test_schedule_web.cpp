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

TEST_CASE("Parsing week days set", "[parsing]")
{
    SECTION("Normal week days json array parsed successfully")
    {
        const auto sat = ParseWeekDays(nlohmann::json::array({0, 1, 2, 3, 4, 5}));
        REQUIRE(sat == WeekDays::fullWeek());
    }
    SECTION("If duplications found - they are ignored")
    {
        const auto sat = ParseWeekDays(nlohmann::json::array({0, 0, 1, 2, 3, 3, 3}));
        REQUIRE(sat == WeekDays({WeekDay::Monday, WeekDay::Tuesday, WeekDay::Wednesday, WeekDay::Thursday}));
    }
    SECTION("Negative value is incorrect")
    {
        WeekDays sat;
        REQUIRE_THROWS_AS(sat = ParseWeekDays(nlohmann::json::array({2, -3, 1, -4})), std::invalid_argument);
    }
    SECTION("Too big value is incorrect")
    {
        WeekDays sat;
        REQUIRE_THROWS_AS(sat = ParseWeekDays(nlohmann::json::array({2, 6, 1, 0})), std::invalid_argument);
    }
}

TEST_CASE("Parsing IDs set", "[parsing]")
{
    SECTION("Normal IDs json array parsed successfully")
    {
        const auto sat = ParseIDsSet(nlohmann::json::array({0, 1, 2, 3, 4, 5}));
        REQUIRE(sat == SortedSet<std::size_t>({0, 1, 2, 3, 4, 5}));
    }
    SECTION("If array is empty - IDs set is empty too")
    {
        const auto sat = ParseIDsSet(nlohmann::json::array());
        REQUIRE(sat == SortedSet<std::size_t>());
    }
    SECTION("If duplications found - they are ignored")
    {
        const auto sat = ParseIDsSet(nlohmann::json::array({1, 2, 2, 0, 1, 0, 5, 5, 5}));
        REQUIRE(sat == SortedSet<std::size_t>({0, 1, 2, 5}));
    }
    SECTION("Negative value is incorrect")
    {
        SortedSet<std::size_t> sat;
        REQUIRE_THROWS_AS(sat = ParseIDsSet(nlohmann::json::array({2, -3, 1, -4})), std::invalid_argument);
    }
}

TEST_CASE("Parsing subject request", "[parsing]")
{
    SECTION("Normal subject request json parsed successfully")
    {
        const auto subjectRequest = nlohmann::json::object({
            {"professor", 1},
            {"hours_count", 3},
            {"complexity", 1},
            {"days", nlohmann::json::array({0, 2, 4})},
            {"groups", nlohmann::json::array({1, 2, 4, 5})},
            {"classrooms", nlohmann::json::array({2, 11})}
        });

        SubjectRequest request = ParseSubjectRequest(subjectRequest);
        REQUIRE(request == SubjectRequest(1, 3, 1, {WeekDay::Monday, WeekDay::Wednesday, WeekDay::Friday}, {1, 2, 4, 5}, {2, 11}));
    }
    SECTION("Subject request includes fields 'professor', 'hours_count', 'complexity', 'days', 'groups' and 'classrooms'")
    {
        auto days = nlohmann::json::array({0, 2, 4});
        auto groups = nlohmann::json::array({0, 2, 4});
        auto classrooms = nlohmann::json::array({2, 11});

        SubjectRequest request;
        REQUIRE_THROWS_AS(request = ParseSubjectRequest(nlohmann::json::object()), std::invalid_argument);
        REQUIRE_THROWS_AS(request = ParseSubjectRequest(nlohmann::json::object({{"hours_count", 3},
                                                                                   {"complexity", 1},
                                                                                   {"days", nlohmann::json::array({0, 2, 4})},
                                                                                   {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                                                   {"classrooms", nlohmann::json::array({2, 11})}
                                                                               })), std::invalid_argument);

        REQUIRE_THROWS_AS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                                                   {"complexity", 1},
                                                                                   {"days", nlohmann::json::array({0, 2, 4})},
                                                                                   {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                                                   {"classrooms", nlohmann::json::array({2, 11})}
                                                                               })), std::invalid_argument);

        REQUIRE_THROWS_AS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                                                   {"hours_count", 3},
                                                                                   {"days", nlohmann::json::array({0, 2, 4})},
                                                                                   {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                                                   {"classrooms", nlohmann::json::array({2, 11})}
                                                                               })), std::invalid_argument);

        REQUIRE_THROWS_AS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                                                   {"hours_count", 3},
                                                                                   {"complexity", 1},
                                                                                   {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                                                   {"classrooms", nlohmann::json::array({2, 11})}
                                                                               })), std::invalid_argument);

        REQUIRE_THROWS_AS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                                                   {"hours_count", 3},
                                                                                   {"complexity", 1},
                                                                                   {"days", nlohmann::json::array({0, 2, 4})},
                                                                                   {"classrooms", nlohmann::json::array({2, 11})}
                                                                               })), std::invalid_argument);

        REQUIRE_THROWS_AS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                                                   {"hours_count", 3},
                                                                                   {"complexity", 1},
                                                                                   {"days", nlohmann::json::array({0, 2, 4})},
                                                                                   {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                                               })), std::invalid_argument);
    }
    SECTION("Throws if fields have invalid values")
    {
        SubjectRequest request;

        // negative week day
        REQUIRE_THROWS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                                {"hours_count", 3},
                                                                {"complexity", 1},
                                                                {"days", nlohmann::json::array({0, -2, 4})},
                                                                {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                                {"classrooms", nlohmann::json::array({2, 11})}
                                                            })));

        // too big week day
        REQUIRE_THROWS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                                             {"hours_count", 3},
                                                                             {"complexity", 1},
                                                                             {"days", nlohmann::json::array({0, 2, 1231231123})},
                                                                             {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                                             {"classrooms", nlohmann::json::array({2, 11})}
                                                                            })));

        // negative group IDs
        REQUIRE_THROWS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                              {"hours_count", 3},
                                                              {"complexity", 1},
                                                              {"days", nlohmann::json::array({0, 2, 4})},
                                                              {"groups", nlohmann::json::array({-1, 2, -4, 5})},
                                                              {"classrooms", nlohmann::json::array({2, 11})}
                                                             })));

        // negative classroom IDs
        REQUIRE_THROWS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                              {"hours_count", 3},
                                                              {"complexity", 1},
                                                              {"days", nlohmann::json::array({0, 2, 4})},
                                                              {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                              {"classrooms", nlohmann::json::array({2, -11})}
                                                             })));
    }
    SECTION("Throws when fields have invalid type")
    {
        SubjectRequest request;

        REQUIRE_THROWS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", "1"},
                                                                             {"hours_count", 3},
                                                                             {"complexity", 1},
                                                                             {"days", nlohmann::json::array({0, 2, 4})},
                                                                             {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                                             {"classrooms", nlohmann::json::array({2, 11})}
                                                                            })));

        REQUIRE_THROWS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                              {"hours_count", 3},
                                                              {"complexity", 1},
                                                              {"days", nlohmann::json::object()},
                                                              {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                              {"classrooms", nlohmann::json::array({2, 11})}
                                                             })));

        REQUIRE_THROWS(request = ParseSubjectRequest(nlohmann::json::object({{"professor", 1},
                                                              {"hours_count", 3},
                                                              {"complexity", 1},
                                                              {"days", nlohmann::json::object()},
                                                              {"groups", nlohmann::json::array({1, 2, 4, 5})},
                                                              {"classrooms", 2}
                                                             })));
    }
}

TEST_CASE("Parsing schedule data", "[parsing]")
{
    SECTION("Schedule data includes fields 'subject_requests' ('locked_lessons' is optional)")
    {
        ScheduleData data;
        REQUIRE_THROWS_AS(data = ParseScheduleData(nlohmann::json::object({{"subject_requests", nlohmann::json::array()}})), std::invalid_argument);
    }
}

TEST_CASE("We can merge two ranges", "[algorithms]")
{
    SECTION("Merging to ranges")
    {
        const std::vector<std::size_t> first = {0, 2, 6, 8};
        const std::vector<std::size_t> second = {1, 3, 4, 5, 7};
        REQUIRE(Merge(first, second) == std::vector<std::size_t>({0, 1, 2, 3, 4, 5, 6, 7, 8}));
    }
    SECTION("If first range is empty - returns second range")
    {
        const std::vector<std::size_t> first;
        const std::vector<std::size_t> second = {1, 3, 4, 5, 7};
        REQUIRE(Merge(first, second) == second);
    }
    SECTION("If second range is empty - returns first range")
    {
        const std::vector<std::size_t> first = {0, 2, 6, 8};
        const std::vector<std::size_t> second;
        REQUIRE(Merge(first, second) == first);
    }
}

TEST_CASE("We can insert ordered and unique values", "[algorithms]")
{
    SECTION("Inserting elements in empty range")
    {
        std::vector<std::size_t> elems;

        InsertUniqueOrdered(elems, 2);
        REQUIRE(elems == std::vector<std::size_t>({2}));

        InsertUniqueOrdered(elems, 1);
        REQUIRE(elems == std::vector<std::size_t>({1, 2}));

        InsertUniqueOrdered(elems, 5);
        REQUIRE(elems == std::vector<std::size_t>({1, 2, 5}));

        InsertUniqueOrdered(elems, 0);
        REQUIRE(elems == std::vector<std::size_t>({0, 1, 2, 5}));

        InsertUniqueOrdered(elems, 3);
        REQUIRE(elems == std::vector<std::size_t>({0, 1, 2, 3, 5}));
    }
    SECTION("Inserting element that already exists - has no effect")
    {
        const std::vector<std::size_t> initial = {1, 2, 3, 4, 5};
        std::vector<std::size_t> vec = initial;

        InsertUniqueOrdered(vec, 3);
        REQUIRE(vec == initial);

        InsertUniqueOrdered(vec, 5);
        REQUIRE(vec == initial);

        InsertUniqueOrdered(vec, 2);
        REQUIRE(vec == initial);
    }
}

TEST_CASE("Lesson address serialized successfully", "[serializing]")
{
    REQUIRE(ToJson(LessonAddress(1, 5)) == nlohmann::json::object({{"group", 1},
                                                                    {"lesson", 5}}));
}

TEST_CASE("Schedule item serialized successfully", "[serializing]")
{
    ScheduleItem scheduleItem(LessonAddress(3, 7), 1, 2, 4);

    REQUIRE(ToJson(scheduleItem) == nlohmann::json::object({{"address", nlohmann::json::object({{"group", 3}, {"lesson", 7}})},
                                                             {"subject", 1},
                                                             {"professor", 2},
                                                             {"classroom", 4}}));
}
