#include "ScheduleDataSerialization.h"
#include "GAScheduleGenerator.h"
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


        // REQUIRE(request == SubjectRequest(0, 1, 2, {WeekDay::Monday, WeekDay::Wednesday, WeekDay::Friday}, {1, 2, 4, 5},
        //                                   {ClassroomAddress{.Building = 0, .Classroom = 2},
        //                                     ClassroomAddress{.Building = 0, .Classroom = 11},
        //                                     ClassroomAddress{.Building = 1, .Classroom = 3},
        //                                     ClassroomAddress{.Building = 1, .Classroom = 12}}));
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

TEST_CASE("Integration test #1", "[integration]")
{
    const auto jsonData = R"(
        {
            "subject_requests": [
                {
                    "id": 16,
                    "professor": 157,
                    "complexity": 1,
                    "days": [
                        0
                    ],
                    "groups": [
                        1,
                        3,
                        4,
                        6,
                        9
                    ],
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
                    ]
                },
                {
                    "id": 25,
                    "professor": 157,
                    "complexity": 1,
                    "days": [
                        0
                    ],
                    "groups": [
                        1,
                        4,
                        8,
                        9,
                        10
                    ],
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
                    ]
                },
                {
                    "id": 62,
                    "professor": 157,
                    "complexity": 1,
                    "days": [
                        0
                    ],
                    "groups": [
                        4,
                        5
                    ],
                    "classrooms": [
                        [
                            12,
                            13,
                            17
                        ],
                        []
                    ]
                },
                {
                    "id": 176,
                    "professor": 157,
                    "complexity": 1,
                    "days": [
                        0
                    ],
                    "groups": [
                        2,
                        3,
                        6,
                        7
                    ],
                    "classrooms": [
                        [
                            19
                        ],
                        []
                    ]
                },
                {
                    "id": 181,
                    "professor": 157,
                    "complexity": 1,
                    "days": [
                        0
                    ],
                    "groups": [
                        1
                    ],
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
                    ]
                },
                {
                    "id": 98,
                    "professor": 17,
                    "complexity": 4,
                    "days": [
                        3
                    ],
                    "groups": [
                        4,
                        9,
                        10
                    ],
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
                    ]
                },
                {
                    "id": 144,
                    "professor": 17,
                    "complexity": 4,
                    "days": [
                        3
                    ],
                    "groups": [
                        2,
                        6,
                        8,
                        9
                    ],
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
                    ]
                },
                {
                    "id": 155,
                    "professor": 17,
                    "complexity": 4,
                    "days": [
                        3
                    ],
                    "groups": [
                        3,
                        9
                    ],
                    "classrooms": [
                        [
                            9,
                            15,
                            18
                        ],
                        []
                    ]
                },
                {
                    "id": 173,
                    "professor": 17,
                    "complexity": 4,
                    "days": [
                        3
                    ],
                    "groups": [
                        1,
                        3,
                        5
                    ],
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
                    ]
                },
                {
                    "id": 80,
                    "professor": 1,
                    "complexity": 4,
                    "days": [
                        3
                    ],
                    "groups": [
                        5,
                        6,
                        7,
                        10
                    ],
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
                    ]
                },
                {
                    "id": 93,
                    "professor": 1,
                    "complexity": 4,
                    "days": [
                        3
                    ],
                    "groups": [
                        4
                    ],
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
                    ]
                },
                {
                    "id": 2,
                    "professor": 4,
                    "complexity": 1,
                    "days": [
                        0,
                        5,
                        1,
                        3
                    ],
                    "groups": [
                        4,
                        5,
                        6,
                        9
                    ],
                    "classrooms": [
                        [
                            5,
                            9
                        ],
                        []
                    ]
                },
                {
                    "id": 67,
                    "professor": 4,
                    "complexity": 1,
                    "days": [
                        0,
                        5,
                        1,
                        3
                    ],
                    "groups": [
                        2,
                        6,
                        8,
                        9,
                        10
                    ],
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
                    ]
                },
                {
                    "id": 81,
                    "professor": 4,
                    "complexity": 1,
                    "days": [
                        0,
                        5,
                        1,
                        3
                    ],
                    "groups": [
                        8
                    ],
                    "classrooms": [
                        [
                            6
                        ],
                        []
                    ]
                },
                {
                    "id": 83,
                    "professor": 4,
                    "complexity": 1,
                    "days": [
                        0,
                        5,
                        1,
                        3
                    ],
                    "groups": [
                        2,
                        4,
                        6,
                        7,
                        8
                    ],
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
                    ]
                },
                {
                    "id": 100,
                    "professor": 4,
                    "complexity": 1,
                    "days": [
                        0,
                        5,
                        1,
                        3
                    ],
                    "groups": [
                        1,
                        4,
                        7,
                        8,
                        10
                    ],
                    "classrooms": [
                        [
                            13,
                            16
                        ],
                        [
                            1
                        ]
                    ]
                },
                {
                    "id": 143,
                    "professor": 4,
                    "complexity": 1,
                    "days": [
                        0,
                        5,
                        1,
                        3
                    ],
                    "groups": [
                        1
                    ],
                    "classrooms": [
                        [
                            3,
                            4,
                            5,
                            6,
                            7,
                            11,
                            12,
                            14,
                            16,
                            18,
                            19
                        ],
                        []
                    ]
                },
                {
                    "id": 147,
                    "professor": 4,
                    "complexity": 1,
                    "days": [
                        0,
                        5,
                        1,
                        3
                    ],
                    "groups": [
                        3,
                        4,
                        7,
                        8
                    ],
                    "classrooms": [
                        [
                            3,
                            10,
                            12,
                            17,
                            19
                        ],
                        []
                    ]
                },
                {
                    "id": 193,
                    "professor": 4,
                    "complexity": 1,
                    "days": [
                        0,
                        5,
                        1,
                        3
                    ],
                    "groups": [
                        1,
                        2,
                        4,
                        6,
                        9
                    ],
                    "classrooms": [
                        [
                            3,
                            5,
                            6,
                            7,
                            8,
                            12,
                            13
                        ],
                        [
                            1
                        ]
                    ]
                },
                {
                    "id": 79,
                    "professor": 88,
                    "complexity": 3,
                    "days": [
                        4,
                        1,
                        3,
                        2,
                        0
                    ],
                    "groups": [
                        6
                    ],
                    "classrooms": [
                        [
                            11,
                            15,
                            19
                        ],
                        []
                    ]
                }
            ],
            "locked_lessons": [
                {
                    "subject_request_id": 2,
                    "address": 0
                }
            ]
        }
    )"_json;

    GAScheduleGenerator generator;
    generator.SetOptions(ScheduleGenOptions{
        {"individuals_count", 50},
        {"iterations_count", 20},
        {"selection_count", 16},
        {"crossover_count", 12},
        {"mutation_chance", 39}
    });

    const ScheduleData scheduleData = jsonData;
    const ScheduleResult scheduleResult = generator.Generate(scheduleData);
    const nlohmann::json jsonResult = scheduleResult;

    const CheckScheduleResult checkResult = CheckSchedule(scheduleData, jsonResult);
    REQUIRE(std::empty(checkResult.OverlappedClassroomsList));
    REQUIRE(std::empty(checkResult.OverlappedProfessorsList));
    REQUIRE(std::empty(checkResult.OverlappedGroupsList));
    REQUIRE(std::empty(checkResult.ViolatedWeekdayRequestsList));
}
