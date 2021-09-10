#include "ScheduleData.h"
#include "ScheduleUtils.h"
#include "ScheduleChromosomes.h"
#include "GAScheduleGenerator.h"
#include <catch2/catch.hpp>


TEST_CASE("No ovelaps by professors", "[chromosomes][initialization]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        // lesson numbers may NOT intersect
        SubjectRequest{0, 1, 1, {0}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{1, 1, 2, {1}, WeekdaysToLessons({WeekDay::Monday}), {{0, 2}}},
        SubjectRequest{2, 1, 3, {2}, WeekdaysToLessons({WeekDay::Monday}), {{0, 3}}},
        SubjectRequest{3, 1, 4, {3}, WeekdaysToLessons({WeekDay::Monday}), {{0, 4}}},
        SubjectRequest{4, 1, 1, {4}, WeekdaysToLessons({WeekDay::Monday}), {{0, 5}}},
        SubjectRequest{5, 1, 2, {5}, WeekdaysToLessons({WeekDay::Monday}), {{0, 6}}},
        SubjectRequest{6, 1, 3, {6}, WeekdaysToLessons({WeekDay::Monday}), {{0, 7}}},
        SubjectRequest{7, 1, 4, {7}, WeekdaysToLessons({WeekDay::Monday}), {{0, 8}}},
        SubjectRequest{8, 1, 1, {8}, WeekdaysToLessons({WeekDay::Monday}), {{0, 9}}},
        SubjectRequest{9, 1, 2, {9}, WeekdaysToLessons({WeekDay::Monday}), {{0, 10}}},
        SubjectRequest{10, 1, 3, {10}, WeekdaysToLessons({WeekDay::Monday}), {{0, 11}}},
        SubjectRequest{11, 1, 4, {11}, WeekdaysToLessons({WeekDay::Monday}), {{0, 12}}},
        SubjectRequest{12, 1, 1, {12}, WeekdaysToLessons({WeekDay::Monday}), {{0, 13}}},
        SubjectRequest{13, 1, 2, {13}, WeekdaysToLessons({WeekDay::Monday}), {{0, 14}}},

        // lesson numbers MAY intersect
        SubjectRequest{14, 2, 3, {14}, WeekdaysToLessons({WeekDay::Monday}), {{0, 15}}},
        SubjectRequest{15, 3, 4, {15}, WeekdaysToLessons({WeekDay::Monday}), {{0, 16}}}
    }};
    const ScheduleChromosomes sut = InitializeChromosomes(data);
    for(std::size_t r = 1; r < 14; ++r)
        REQUIRE(sut.Lesson(r - 1) != sut.Lesson(r));
}

TEST_CASE("No ovelaps by groups", "[chromosomes][initialization]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        // lesson numbers may NOT intersect
        SubjectRequest{0, 1, 1, {1}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{1, 2, 2, {1, 2}, WeekdaysToLessons({WeekDay::Monday}), {{0, 2}}},
        SubjectRequest{2, 3, 3, {1, 3}, WeekdaysToLessons({WeekDay::Monday}), {{0, 3}}},
        SubjectRequest{3, 4, 4, {1, 2, 4}, WeekdaysToLessons({WeekDay::Monday}), {{0, 4}}},
        SubjectRequest{4, 5, 1, {1, 4, 5}, WeekdaysToLessons({WeekDay::Monday}), {{0, 5}}},
        SubjectRequest{5, 6, 2, {1, 2, 3}, WeekdaysToLessons({WeekDay::Monday}), {{0, 6}}},
        SubjectRequest{6, 7, 3, {1, 8, 9}, WeekdaysToLessons({WeekDay::Monday}), {{0, 7}}},
        SubjectRequest{7, 8, 4, {1, 2, 4, 5}, WeekdaysToLessons({WeekDay::Monday}), {{0, 8}}},
        SubjectRequest{8, 9, 1, {1, 3, 6, 7}, WeekdaysToLessons({WeekDay::Monday}), {{0, 9}}},
        SubjectRequest{9, 10, 2, {1, 11, 12}, WeekdaysToLessons({WeekDay::Monday}), {{0, 10}}},
        SubjectRequest{10, 11, 3, {1, 16, 17}, WeekdaysToLessons({WeekDay::Monday}), {{0, 11}}},
        SubjectRequest{11, 12, 4, {1, 2, 4, 5}, WeekdaysToLessons({WeekDay::Monday}), {{0, 12}}},
        SubjectRequest{12, 13, 1, {1, 2, 3}, WeekdaysToLessons({WeekDay::Monday}), {{0, 13}}},
        SubjectRequest{13, 14, 2, {1, 5, 6, 7, 8}, WeekdaysToLessons({WeekDay::Monday}), {{0, 14}}},

        // lesson numbers MAY intersect
        SubjectRequest{14, 15, 3, {200}, WeekdaysToLessons({WeekDay::Monday}), {{0, 15}}},
        SubjectRequest{15, 16, 4, {300}, WeekdaysToLessons({WeekDay::Monday}), {{0, 16}}}
    }};
    const ScheduleChromosomes sut = InitializeChromosomes(data);

    for(std::size_t r = 1; r < 14; ++r)
        REQUIRE(sut.Lesson(r - 1) != sut.Lesson(r));
}

TEST_CASE("No ovelaps by classrooms", "[chromosomes][initialization]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        // lesson numbers may NOT intersect
        SubjectRequest{0, 1, 1, {1}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{1, 2, 2, {2}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{2, 3, 3, {3}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{3, 4, 4, {4}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{4, 5, 1, {5}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{5, 6, 2, {6}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{6, 7, 3, {7}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{7, 8, 4, {8}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{8, 9, 1, {9}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{9, 10, 2, {10}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{10, 11, 3, {11}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{11, 12, 4, {12}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{12, 13, 1, {13}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},
        SubjectRequest{13, 14, 2, {14}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}}},

        // lesson numbers MAY intersect
        SubjectRequest{14, 15, 3, {15}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}, {0, 200}}},
        SubjectRequest{15, 16, 4, {16}, WeekdaysToLessons({WeekDay::Monday}), {{0, 1}, {0, 300}}}
    }};
    const ScheduleChromosomes sut = InitializeChromosomes(data);

    for(std::size_t r = 1; r < 14; ++r)
        REQUIRE(sut.Lesson(r - 1) != sut.Lesson(r));
}

TEST_CASE("Check for groups intersection performs", "[chromosomes][checks][intersections]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        SubjectRequest{0, 1, 1, {0, 1, 2}, {}, {{0, 1}}},
        SubjectRequest{1, 2, 1, {2, 3, 5}, {}, {{0, 2}}},
        SubjectRequest{2, 3, 1, {6, 7, 8}, {}, {{0, 3}}},
        SubjectRequest{3, 4, 1, {9, 10, 11}, {}, {{0, 4}}},
        SubjectRequest{4, 5, 1, {12, 13, 14}, {}, {{0, 5}}}
    }};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    REQUIRE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
    REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));

    REQUIRE_FALSE(sut.GroupsOrProfessorsIntersects(data, 1, 2));
    REQUIRE_FALSE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 2));
}

TEST_CASE("Check for professors intersection performs", "[chromosomes][checks][intersections]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        SubjectRequest{0, 2, 1, {1}, {}, {{0, 1}}},
        SubjectRequest{1, 2, 1, {2}, {}, {{0, 2}}},
        SubjectRequest{2, 3, 1, {3}, {}, {{0, 3}}},
        SubjectRequest{3, 4, 1, {4}, {}, {{0, 4}}},
        SubjectRequest{4, 5, 1, {5}, {}, {{0, 5}}}
    }};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    REQUIRE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
    REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));

    REQUIRE_FALSE(sut.GroupsOrProfessorsIntersects(data, 1, 2));
    REQUIRE_FALSE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 2));
}

TEST_CASE("Check for classrooms intersection performs", "[chromosomes][checks][intersections]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        SubjectRequest{0, 1, 1, {1}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, {2}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 3, 1, {3}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, {4}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, {5}, {}, {{0, 1}, {0, 2}, {0, 3}}}
    }};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    REQUIRE_FALSE(sut.GroupsOrProfessorsIntersects(data, 3, 0));
    REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 3, 0));

    REQUIRE_FALSE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
    REQUIRE_FALSE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));
}

TEST_CASE("Check if chromosomes ready for crossover", "[chromosomes][checks][crossover]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        SubjectRequest{0, 1, 1, {0, 1, 2}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, {1, 2, 3}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 1, 1, {4, 5, 6}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, {7, 8, 9}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, {10},      {}, {{0, 1}, {0, 2}, {0, 3}}}
    }};

    ScheduleChromosomes sut1{{0, 1, 2, 3, 4}, {{0,3}, {0,2}, {0,1}, {0,3}, {0,2}}};
    ScheduleChromosomes sut2{{4, 3, 2, 1, 0}, {{0,1}, {0,2}, {0,3}, {0,2}, {0,2}}};

    REQUIRE(ReadyToCrossover(sut1, sut2, data, 0));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 1));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 2));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 3));
    REQUIRE(ReadyToCrossover(sut1, sut2, data, 4));
}

TEST_CASE("Crossover works", "[chromosomes][crossover]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        SubjectRequest{0, 1, 1, {0, 1, 2}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, {1, 2, 3}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 1, 1, {4, 5, 6}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, {7, 8, 9}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, {10},      {}, {{0, 1}, {0, 2}, {0, 3}}}
    }};

    ScheduleChromosomes first{{0, 1, 2, 3, 4}, {{0,3}, {0,2}, {0,1}, {0,3}, {0,2}}};
    ScheduleChromosomes second{{4, 3, 2, 1, 0}, {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    Crossover(first, second, 0);
    REQUIRE(first.Lesson(0) == 4);
    REQUIRE(first.Classroom(0) == ClassroomAddress{.Building = 0, .Classroom = 1});

    REQUIRE(second.Lesson(0) == 0);
    REQUIRE(second.Classroom(0) == ClassroomAddress{.Building = 0, .Classroom = 3});
}

TEST_CASE("MakeScheduleResult works correctly", "[chromosomes][conversions]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData data{{
        SubjectRequest{0, 1, 1, {0, 1, 2}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, {1, 2, 3}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 1, 1, {4, 5, 6}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, {7, 8, 9}, {}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, {10},      {}, {{0, 1}, {0, 2}, {0, 3}}}
    }};

    const ScheduleChromosomes chromosomes{
        {0, 4, 3, 2, 1},
        {{0,1}, {0,1}, {0,3}, {0, 2}, {0,3}}
    };

    const ScheduleResult scheduleResult = MakeScheduleResult(chromosomes, data);
    REQUIRE(contains(scheduleResult.items(), ScheduleItem{.Address = 0, .SubjectRequestID = 0, .Classroom = 1}));
    REQUIRE(contains(scheduleResult.items(), ScheduleItem{.Address = 1, .SubjectRequestID = 4, .Classroom = 3}));
    REQUIRE(contains(scheduleResult.items(), ScheduleItem{.Address = 2, .SubjectRequestID = 3, .Classroom = 2}));
    REQUIRE(contains(scheduleResult.items(), ScheduleItem{.Address = 3, .SubjectRequestID = 2, .Classroom = 3}));
    REQUIRE(contains(scheduleResult.items(), ScheduleItem{.Address = 4, .SubjectRequestID = 1, .Classroom = 1}));
}
