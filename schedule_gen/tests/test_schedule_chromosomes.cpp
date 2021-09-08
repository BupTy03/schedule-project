#include "ScheduleData.h"
#include "ScheduleUtils.h"
#include "ScheduleChromosomes.h"
#include "GAScheduleGenerator.h"
#include <catch2/catch.hpp>


TEST_CASE("No ovelaps by professors", "[chromosomes][initialization]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        // lesson numbers may NOT intersect
        SubjectRequest{0, 1, 1, {WeekDay::Monday}, {0}, {{0, 1}}},
        SubjectRequest{1, 1, 2, {WeekDay::Monday}, {1}, {{0, 2}}},
        SubjectRequest{2, 1, 3, {WeekDay::Monday}, {2}, {{0, 3}}},
        SubjectRequest{3, 1, 4, {WeekDay::Monday}, {3}, {{0, 4}}},
        SubjectRequest{4, 1, 1, {WeekDay::Monday}, {4}, {{0, 5}}},
        SubjectRequest{5, 1, 2, {WeekDay::Monday}, {5}, {{0, 6}}},
        SubjectRequest{6, 1, 3, {WeekDay::Monday}, {6}, {{0, 7}}},
        SubjectRequest{7, 1, 4, {WeekDay::Monday}, {7}, {{0, 8}}},
        SubjectRequest{8, 1, 1, {WeekDay::Monday}, {8}, {{0, 9}}},
        SubjectRequest{9, 1, 2, {WeekDay::Monday}, {9}, {{0, 10}}},
        SubjectRequest{10, 1, 3, {WeekDay::Monday}, {10}, {{0, 11}}},
        SubjectRequest{11, 1, 4, {WeekDay::Monday}, {11}, {{0, 12}}},
        SubjectRequest{12, 1, 1, {WeekDay::Monday}, {12}, {{0, 13}}},
        SubjectRequest{13, 1, 2, {WeekDay::Monday}, {13}, {{0, 14}}},

        // lesson numbers MAY intersect
        SubjectRequest{14, 2, 3, {WeekDay::Monday}, {14}, {{0, 15}}},
        SubjectRequest{15, 3, 4, {WeekDay::Monday}, {15}, {{0, 16}}}
    };

    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut = InitializeChromosomes(data);
    for(std::size_t r = 1; r < 14; ++r)
        REQUIRE(sut.Lesson(r - 1) != sut.Lesson(r));
}

TEST_CASE("No ovelaps by groups", "[chromosomes][initialization]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        // lesson numbers may NOT intersect
        SubjectRequest{0, 1, 1, {WeekDay::Monday}, {1}, {{0, 1}}},
        SubjectRequest{1, 2, 2, {WeekDay::Monday}, {1, 2}, {{0, 2}}},
        SubjectRequest{2, 3, 3, {WeekDay::Monday}, {1, 3}, {{0, 3}}},
        SubjectRequest{3, 4, 4, {WeekDay::Monday}, {1, 2, 4}, {{0, 4}}},
        SubjectRequest{4, 5, 1, {WeekDay::Monday}, {1, 4, 5}, {{0, 5}}},
        SubjectRequest{5, 6, 2, {WeekDay::Monday}, {1, 2, 3}, {{0, 6}}},
        SubjectRequest{6, 7, 3, {WeekDay::Monday}, {1, 8, 9}, {{0, 7}}},
        SubjectRequest{7, 8, 4, {WeekDay::Monday}, {1, 2, 4, 5}, {{0, 8}}},
        SubjectRequest{8, 9, 1, {WeekDay::Monday}, {1, 3, 6, 7}, {{0, 9}}},
        SubjectRequest{9, 10, 2, {WeekDay::Monday}, {1, 11, 12}, {{0, 10}}},
        SubjectRequest{10, 11, 3, {WeekDay::Monday}, {1, 16, 17}, {{0, 11}}},
        SubjectRequest{11, 12, 4, {WeekDay::Monday}, {1, 2, 4, 5}, {{0, 12}}},
        SubjectRequest{12, 13, 1, {WeekDay::Monday}, {1, 2, 3}, {{0, 13}}},
        SubjectRequest{13, 14, 2, {WeekDay::Monday}, {1, 5, 6, 7, 8}, {{0, 14}}},

        // lesson numbers MAY intersect
        SubjectRequest{14, 15, 3, {WeekDay::Monday}, {200}, {{0, 15}}},
        SubjectRequest{15, 16, 4, {WeekDay::Monday}, {300}, {{0, 16}}}
    };

    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut = InitializeChromosomes(data);

    for(std::size_t r = 1; r < 14; ++r)
        REQUIRE(sut.Lesson(r - 1) != sut.Lesson(r));
}

TEST_CASE("No ovelaps by classrooms", "[chromosomes][initialization]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        // lesson numbers may NOT intersect
        SubjectRequest{0, 1, 1, {WeekDay::Monday}, {1}, {{0, 1}}},
        SubjectRequest{1, 2, 2, {WeekDay::Monday}, {2}, {{0, 1}}},
        SubjectRequest{2, 3, 3, {WeekDay::Monday}, {3}, {{0, 1}}},
        SubjectRequest{3, 4, 4, {WeekDay::Monday}, {4}, {{0, 1}}},
        SubjectRequest{4, 5, 1, {WeekDay::Monday}, {5}, {{0, 1}}},
        SubjectRequest{5, 6, 2, {WeekDay::Monday}, {6}, {{0, 1}}},
        SubjectRequest{6, 7, 3, {WeekDay::Monday}, {7}, {{0, 1}}},
        SubjectRequest{7, 8, 4, {WeekDay::Monday}, {8}, {{0, 1}}},
        SubjectRequest{8, 9, 1, {WeekDay::Monday}, {9}, {{0, 1}}},
        SubjectRequest{9, 10, 2, {WeekDay::Monday}, {10}, {{0, 1}}},
        SubjectRequest{10, 11, 3, {WeekDay::Monday}, {11}, {{0, 1}}},
        SubjectRequest{11, 12, 4, {WeekDay::Monday}, {12}, {{0, 1}}},
        SubjectRequest{12, 13, 1, {WeekDay::Monday}, {13}, {{0, 1}}},
        SubjectRequest{13, 14, 2, {WeekDay::Monday}, {14}, {{0, 1}}},

        // lesson numbers MAY intersect
        SubjectRequest{14, 15, 3, {WeekDay::Monday}, {15}, {{0, 1}, {0, 200}}},
        SubjectRequest{15, 16, 4, {WeekDay::Monday}, {16}, {{0, 1}, {0, 300}}}
    };

    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut = InitializeChromosomes(data);

    for(std::size_t r = 1; r < 14; ++r)
        REQUIRE(sut.Lesson(r - 1) != sut.Lesson(r));
}

TEST_CASE("No weekday violations", "[chromosomes][initialization]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 1, 1, {WeekDay::Monday}, {1}, {{0, 1}}},
        SubjectRequest{1, 2, 2, {WeekDay::Tuesday}, {2}, {{0, 2}}},
        SubjectRequest{2, 3, 3, {WeekDay::Wednesday}, {3}, {{0, 3}}},
        SubjectRequest{3, 4, 4, {WeekDay::Thursday}, {4}, {{0, 4}}},
        SubjectRequest{4, 5, 1, {WeekDay::Friday}, {5}, {{0, 5}}},
        SubjectRequest{5, 6, 2, {WeekDay::Saturday}, {6}, {{0, 6}}}
    };

    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut = InitializeChromosomes(data);

    REQUIRE(ToWeekDay(LessonToScheduleDay(sut.Lesson(0))) == WeekDay::Monday);
    REQUIRE(ToWeekDay(LessonToScheduleDay(sut.Lesson(1))) == WeekDay::Tuesday);
    REQUIRE(ToWeekDay(LessonToScheduleDay(sut.Lesson(2))) == WeekDay::Wednesday);
    REQUIRE(ToWeekDay(LessonToScheduleDay(sut.Lesson(3))) == WeekDay::Thursday);
    REQUIRE(ToWeekDay(LessonToScheduleDay(sut.Lesson(4))) == WeekDay::Friday);
    REQUIRE(ToWeekDay(LessonToScheduleDay(sut.Lesson(5))) == WeekDay::Saturday);
}

TEST_CASE("Morning classes are in 0-3 places in Saturday", "[chromosomes][initialization]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}},
        SubjectRequest{1, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}},
        SubjectRequest{2, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}},
        SubjectRequest{3, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}},
        SubjectRequest{4, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}},
        SubjectRequest{5, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}},
        SubjectRequest{6, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}},
        SubjectRequest{7, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}},

        SubjectRequest{8, 2, 1, {WeekDay::Saturday}, {2}, {{0, 2}}},
        SubjectRequest{9, 3, 1, {WeekDay::Saturday}, {3}, {{0, 3}}}
    };

    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut = InitializeChromosomes(data);

    for(std::size_t lesson : sut.Lessons())
    {
        const auto dayLesson = lesson % MAX_LESSONS_PER_DAY;
        REQUIRE(dayLesson < 4);
    }
}

TEST_CASE("Evening classes are in 5-6 places (except Saturday)", "[chromosomes][initialization]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 1, 1, {WeekDay::Monday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{1, 1, 1, {WeekDay::Tuesday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{2, 1, 1, {WeekDay::Wednesday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{3, 1, 1, {WeekDay::Thursday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{4, 1, 1, {WeekDay::Friday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{6, 1, 1, {WeekDay::Monday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{7, 1, 1, {WeekDay::Tuesday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{8, 1, 1, {WeekDay::Wednesday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{9, 1, 1, {WeekDay::Thursday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{10, 1, 1, {WeekDay::Friday}, {1}, {{0, 1}}, ClassesType::Evening},

        SubjectRequest{11, 1, 1, {WeekDay::Monday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{12, 1, 1, {WeekDay::Tuesday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{13, 1, 1, {WeekDay::Wednesday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{14, 1, 1, {WeekDay::Thursday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{15, 1, 1, {WeekDay::Friday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{16, 1, 1, {WeekDay::Monday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{17, 1, 1, {WeekDay::Tuesday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{18, 1, 1, {WeekDay::Wednesday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{19, 1, 1, {WeekDay::Thursday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{20, 1, 1, {WeekDay::Friday}, {1}, {{0, 1}}, ClassesType::Evening},

        SubjectRequest{21, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}, ClassesType::Evening},
        SubjectRequest{22, 1, 1, {WeekDay::Saturday}, {1}, {{0, 1}}, ClassesType::Evening}
    };

    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut = InitializeChromosomes(data);

    for(std::size_t r = 0; r < 20; ++r)
    {
        const auto dayLesson = sut.Lesson(r) % MAX_LESSONS_PER_DAY;
        REQUIRE(dayLesson > 4);
    }
}

TEST_CASE("Check for groups intersection performs", "[chromosomes][checks]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 1, 1, {}, {0, 1, 2}, {{0, 1}}},
        SubjectRequest{1, 2, 1, {}, {2, 3, 5}, {{0, 2}}},
        SubjectRequest{2, 3, 1, {}, {6, 7, 8}, {{0, 3}}},
        SubjectRequest{3, 4, 1, {}, {9, 10, 11}, {{0, 4}}},
        SubjectRequest{4, 5, 1, {}, {12, 13, 14}, {{0, 5}}}
    };
    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    REQUIRE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
    REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));

    REQUIRE_FALSE(sut.GroupsOrProfessorsIntersects(data, 1, 2));
    REQUIRE_FALSE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 2));
}

TEST_CASE("Check for professors intersection performs", "[chromosomes][checks]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 2, 1, {}, {1}, {{0, 1}}},
        SubjectRequest{1, 2, 1, {}, {2}, {{0, 2}}},
        SubjectRequest{2, 3, 1, {}, {3}, {{0, 3}}},
        SubjectRequest{3, 4, 1, {}, {4}, {{0, 4}}},
        SubjectRequest{4, 5, 1, {}, {5}, {{0, 5}}}
    };
    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    REQUIRE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
    REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));

    REQUIRE_FALSE(sut.GroupsOrProfessorsIntersects(data, 1, 2));
    REQUIRE_FALSE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 2));
}

TEST_CASE("Check for classrooms intersection performs", "[chromosomes][checks]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 1, 1, {}, {1}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, {}, {2}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 3, 1, {}, {3}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, {}, {4}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, {}, {5}, {{0, 1}, {0, 2}, {0, 3}}}
    };
    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    REQUIRE_FALSE(sut.GroupsOrProfessorsIntersects(data, 3, 0));
    REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 3, 0));

    REQUIRE_FALSE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
    REQUIRE_FALSE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));
}


TEST_CASE("Check if chromosomes ready for crossover", "[chromosomes][checks]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 1, 1, {}, {0, 1, 2}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, {}, {1, 2, 3}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 1, 1, {}, {4, 5, 6}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, {}, {7, 8, 9}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, {}, {10},      {{0, 1}, {0, 2}, {0, 3}}}
    };
    const ScheduleData data{requests, {}};

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
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 1, 1, {}, {0, 1, 2}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, {}, {1, 2, 3}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 1, 1, {}, {4, 5, 6}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, {}, {7, 8, 9}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, {}, {10},      {{0, 1}, {0, 2}, {0, 3}}}
    };
    const ScheduleData data{requests, {}};

    ScheduleChromosomes first{{0, 1, 2, 3, 4}, {{0,3}, {0,2}, {0,1}, {0,3}, {0,2}}};
    ScheduleChromosomes second{{4, 3, 2, 1, 0}, {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    Crossover(first, second, 0);
    REQUIRE(first.Lesson(0) == 4);
    REQUIRE(first.Classroom(0) == ClassroomAddress{.Building = 0, .Classroom = 1});

    REQUIRE(second.Lesson(0) == 0);
    REQUIRE(second.Classroom(0) == ClassroomAddress{.Building = 0, .Classroom = 3});
}

TEST_CASE("MakeScheduleResult works correctly", "[chromosomes][conversion]")
{
    // [id, professor, complexity, weekDays, groups, classrooms]
    const std::vector<SubjectRequest> requests {
        SubjectRequest{0, 1, 1, {}, {0, 1, 2}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, {}, {1, 2, 3}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 1, 1, {}, {4, 5, 6}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, {}, {7, 8, 9}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, {}, {10},      {{0, 1}, {0, 2}, {0, 3}}}
    };
    const ScheduleData data{requests, {}};

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
