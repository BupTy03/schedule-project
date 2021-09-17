#include "ScheduleGA.h"
#include "ScheduleDataSerialization.h"

#include <catch2/catch.hpp>


TEST_CASE("Parsing lessons set", "[parsing]")
{
    SECTION("Normal week days json array parsed successfully")
    {
        REQUIRE(ParseLessonsSet("[0, 1, 2, 3, 4, 5]"_json)
                == std::vector<std::size_t>{0, 1, 2, 3, 4, 5});
    }
    SECTION("If duplications found - they are ignored")
    {
        REQUIRE(ParseLessonsSet("[0, 0, 1, 2, 3, 3, 3]"_json)
                == std::vector<std::size_t>{0, 1, 2, 3});
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

        const SubjectRequest expected{0,
                                      1,
                                      2,
                                      {1, 2, 4, 5},
                                      {0, 2, 4},
                                      {ClassroomAddress{.Building = 0, .Classroom = 2},
                                       ClassroomAddress{.Building = 0, .Classroom = 11},
                                       ClassroomAddress{.Building = 1, .Classroom = 3},
                                       ClassroomAddress{.Building = 1, .Classroom = 12}}};

        REQUIRE(request == expected);
    }
    SECTION(
        "Subject request includes fields 'id', 'professor', 'complexity', 'groups', 'lessons' and "
        "'classrooms'")
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

TEST_CASE("Integration test #1", "[integration]")
{
    const auto jsonData = R"(
        {
        "subject_requests": [
            {
                "classrooms": [
                [
                    4,
                    6,
                    8,
                    9,
                    10,
                    12,
                    13,
                    14
                ],
                [
                    1
                ]
                ],
                "complexity": 1,
                "groups": [
                1,
                3,
                4,
                6,
                9
                ],
                "id": 16,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                42,
                43,
                44,
                45,
                46,
                47,
                48
                ],
                "professor": 157
            },
            {
                "classrooms": [
                [
                    9,
                    10,
                    13,
                    20
                ],
                [
                    1
                ]
                ],
                "complexity": 1,
                "groups": [
                1,
                4,
                8,
                9,
                10
                ],
                "id": 25,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                42,
                43,
                44,
                45,
                46,
                47,
                48
                ],
                "professor": 157
            },
            {
                "classrooms": [
                [
                    12,
                    13,
                    17
                ],
                []
                ],
                "complexity": 1,
                "groups": [
                4,
                5
                ],
                "id": 62,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                42,
                43,
                44,
                45,
                46,
                47,
                48
                ],
                "professor": 157
            },
            {
                "classrooms": [
                [
                    19
                ],
                []
                ],
                "complexity": 1,
                "groups": [
                2,
                3,
                6,
                7
                ],
                "id": 176,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                42,
                43,
                44,
                45,
                46,
                47,
                48
                ],
                "professor": 157
            },
            {
                "classrooms": [
                [
                    4,
                    5,
                    7,
                    12,
                    13
                ],
                [
                    2
                ]
                ],
                "complexity": 1,
                "groups": [
                1
                ],
                "id": 181,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                42,
                43,
                44,
                45,
                46,
                47,
                48
                ],
                "professor": 157
            },
            {
                "classrooms": [
                [
                    5,
                    12,
                    16,
                    18
                ],
                [
                    1
                ]
                ],
                "complexity": 4,
                "groups": [
                4,
                9,
                10
                ],
                "id": 98,
                "lessons": [
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                63,
                64,
                65,
                66,
                67,
                68,
                69
                ],
                "professor": 17
            },
            {
                "classrooms": [
                [
                    3,
                    4,
                    5,
                    6,
                    7,
                    12,
                    13,
                    14,
                    15,
                    16,
                    17
                ],
                []
                ],
                "complexity": 4,
                "groups": [
                2,
                6,
                8,
                9
                ],
                "id": 144,
                "lessons": [
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                63,
                64,
                65,
                66,
                67,
                68,
                69
                ],
                "professor": 17
            },
            {
                "classrooms": [
                [
                    9,
                    15,
                    18
                ],
                []
                ],
                "complexity": 4,
                "groups": [
                3,
                9
                ],
                "id": 155,
                "lessons": [
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                63,
                64,
                65,
                66,
                67,
                68,
                69
                ],
                "professor": 17
            },
            {
                "classrooms": [
                [
                    4,
                    5,
                    7,
                    10,
                    12,
                    14,
                    20
                ],
                [
                    2
                ]
                ],
                "complexity": 4,
                "groups": [
                1,
                3,
                5
                ],
                "id": 173,
                "lessons": [
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                63,
                64,
                65,
                66,
                67,
                68,
                69
                ],
                "professor": 17
            },
            {
                "classrooms": [
                [
                    7,
                    11,
                    18,
                    19,
                    20
                ],
                [
                    2
                ]
                ],
                "complexity": 4,
                "groups": [
                5,
                6,
                7,
                10
                ],
                "id": 80,
                "lessons": [
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                63,
                64,
                65,
                66,
                67,
                68,
                69
                ],
                "professor": 1
            },
            {
                "classrooms": [
                [
                    5,
                    6,
                    8,
                    9,
                    11,
                    14,
                    17
                ],
                [
                    1
                ]
                ],
                "complexity": 4,
                "groups": [
                4
                ],
                "id": 93,
                "lessons": [
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                63,
                64,
                65,
                66,
                67,
                68,
                69
                ],
                "professor": 1
            },
            {
                "classrooms": [
                [
                    5,
                    9
                ],
                []
                ],
                "complexity": 1,
                "groups": [
                4,
                5,
                6,
                9
                ],
                "id": 2,
                "lessons": [
                0,
                7,
                21,
                35,
                42,
                49,
                63,
                77
                ],
                "professor": 4
            },
            {
                "classrooms": [
                [
                    3,
                    4,
                    5,
                    6,
                    8,
                    11,
                    13,
                    15,
                    20
                ],
                [
                    2
                ]
                ],
                "complexity": 1,
                "groups": [
                2,
                6,
                8,
                9,
                10
                ],
                "id": 67,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                35,
                36,
                37,
                38,
                42,
                43,
                44,
                45,
                46,
                47,
                48,
                49,
                50,
                51,
                52,
                53,
                54,
                55,
                63,
                64,
                65,
                66,
                67,
                68,
                69,
                77,
                78,
                79,
                80
                ],
                "professor": 4
            },
            {
                "classrooms": [
                [
                    6
                ],
                []
                ],
                "complexity": 1,
                "groups": [
                8
                ],
                "id": 81,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                35,
                36,
                37,
                38,
                42,
                43,
                44,
                45,
                46,
                47,
                48,
                49,
                50,
                51,
                52,
                53,
                54,
                55,
                63,
                64,
                65,
                66,
                67,
                68,
                69,
                77,
                78,
                79,
                80
                ],
                "professor": 4
            },
            {
                "classrooms": [
                [
                    5,
                    7,
                    12,
                    13,
                    14,
                    16
                ],
                []
                ],
                "complexity": 1,
                "groups": [
                2,
                4,
                6,
                7,
                8
                ],
                "id": 83,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                35,
                36,
                37,
                38,
                42,
                43,
                44,
                45,
                46,
                47,
                48,
                49,
                50,
                51,
                52,
                53,
                54,
                55,
                63,
                64,
                65,
                66,
                67,
                68,
                69,
                77,
                78,
                79,
                80
                ],
                "professor": 4
            },
            {
                "classrooms": [
                [
                    13,
                    16
                ],
                [
                    1
                ]
                ],
                "complexity": 1,
                "groups": [
                1,
                4,
                7,
                8,
                10
                ],
                "id": 100,
                "lessons": [
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                21,
                22,
                23,
                24,
                25,
                26,
                27,
                35,
                36,
                37,
                38,
                42,
                43,
                44,
                45,
                46,
                47,
                48,
                49,
                50,
                51,
                52,
                53,
                54,
                55,
                63,
                64,
                65,
                66,
                67,
                68,
                69,
                77,
                78,
                79,
                80
                ],
                "professor": 4
            }
        ]
    }
    )"_json;

    ScheduleGA generator;
    generator.SetParams(ScheduleGAParams{
        .IndividualsCount = 50,
        .IterationsCount = 20,
        .SelectionCount = 16,
        .CrossoverCount = 12,
        .MutationChance = 39
    });

    const ScheduleData scheduleData = jsonData;
    const ScheduleResult scheduleResult = Generate(generator, scheduleData);
    const nlohmann::json jsonResult = scheduleResult;

    const CheckScheduleResult checkResult = CheckSchedule(scheduleData, jsonResult);
    REQUIRE(empty(checkResult));
}

TEST_CASE("Integration test with blocks", "[integration]")
{
    const auto jsonData = R"(
        {
            "subject_requests": [
                {
                    "id": 1,
                    "complexity": 1,
                    "professor": 1,
                    "classrooms": [
                        [3, 5, 6, 7, 8, 12],
                        [1]
                    ],
                    "groups": [1, 2],
                    "lessons": [
                        0, 1, 2, 3, 4, 5, 6,
                        7, 8, 9, 10,
                        14, 15, 16, 17, 18, 19, 20
                    ]
                },
                {
                    "id": 2,
                    "complexity": 1,
                    "professor": 2,
                    "classrooms": [
                        [3, 5, 6, 7, 8, 12],
                        [1]
                    ],
                    "groups": [1, 2],
                    "lessons": [
                        0, 1, 2, 3, 4, 5, 6,
                        7, 8, 9, 10, 11,
                        14, 15, 16, 17, 18, 19, 20
                    ]
                },
                {
                    "id": 3,
                    "complexity": 1,
                    "professor": 3,
                    "classrooms": [
                        [3, 5, 6, 7, 8, 12],
                        [1, 2]
                    ],
                    "groups": [3, 4, 5],
                    "lessons": [
                        0, 1, 2, 3, 4, 5, 6,
                        14, 15, 16, 17, 18, 19, 20
                    ]
                },
                {
                    "id": 4,
                    "complexity": 1,
                    "professor": 4,
                    "classrooms": [
                        [8, 12],
                        [2]
                    ],
                    "groups": [5],
                    "lessons": [
                        0, 1, 2, 3, 4, 5, 6,
                        21, 22, 23, 24, 25, 26, 27
                    ]
                }
            ],
            "blocks": [[1, 2]]
        }
    )"_json;

    ScheduleGA generator;
    generator.SetParams(ScheduleGAParams{
        .IndividualsCount = 50,
        .IterationsCount = 20,
        .SelectionCount = 16,
        .CrossoverCount = 12,
        .MutationChance = 39
    });

    const ScheduleData scheduleData = jsonData;
    const ScheduleResult scheduleResult = Generate(generator, scheduleData);

    const auto& items = scheduleResult.items();
    const auto firstIt =
        std::find_if(items.begin(),
                     items.end(),
                     [](const ScheduleItem& item) { return item.SubjectRequestID == 1; });
    const auto secondIt =
        std::find_if(items.begin(),
                     items.end(),
                     [](const ScheduleItem& item) { return item.SubjectRequestID == 2; });

    REQUIRE(firstIt != items.end());
    REQUIRE(secondIt != items.end());
    REQUIRE((secondIt->Address - firstIt->Address) == 1);

    const nlohmann::json jsonResult = scheduleResult;
    const CheckScheduleResult checkResult = CheckSchedule(scheduleData, jsonResult);
    REQUIRE(empty(checkResult));
}
