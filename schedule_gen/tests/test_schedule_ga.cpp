#include "GAScheduleGenerator.h"
#include "ScheduleData.h"
#include "ScheduleIndividual.h"
#include "ScheduleChromosomes.h"
#include <catch2/catch.hpp>


TEST_CASE("Initialization of chromosomes is valid", "[ScheduleChromosomes]")
{
    const auto fullWeek = WeekDays::fullWeek();
    const WeekDays weekDays{WeekDay::Monday, WeekDay::Wednesday, WeekDay::Thursday};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest{0, 1, 1, fullWeek, {0, 1, 2}, {{0, 1}, {0, 2}, {0, 3}}, true},
        SubjectRequest{1, 2, 1, weekDays, {1, 2, 3}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 1, 1, fullWeek, {4, 5, 6}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, fullWeek, {7, 8, 9}, {{0, 1}, {0, 2}, {0, 3}}, true},
        SubjectRequest{4, 5, 1, fullWeek, {10},      {{0, 1}, {0, 2}, {0, 3}}}
    };

    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut{data};

    SECTION("No ovelaps by professors")
    {
        REQUIRE(sut.Lesson(0) != sut.Lesson(2));
    }
    SECTION("No overlaps by groups")
    {
        REQUIRE(sut.Lesson(0) != sut.Lesson(1));
    }
    SECTION("No overlaps by classrooms")
    {
        for(std::size_t i = 0; i < requests.size(); ++i)
        {
            for(std::size_t j = 0; j < requests.size(); ++j)
            {
                if(i == j)
                    continue;

                REQUIRE((sut.Lesson(i) != sut.Lesson(j) || sut.Classroom(i) != sut.Classroom(j)));
            }
        }
    }
    SECTION("No weekday request violations")
    {
        const auto day = LessonToScheduleDay(sut.Lesson(1));
        const auto weekDay = ToWeekDay(day);
        REQUIRE((weekDay == WeekDay::Monday ||
                 weekDay == WeekDay::Wednesday ||
                 weekDay == WeekDay::Thursday));
    }
    SECTION("Evening classes are in 5-6 places (except Saturday)")
    {
        for(std::size_t i = 0; i < requests.size(); ++i)
        {
            if(requests.at(i).IsEveningClass())
            {
                const auto day = LessonToScheduleDay(sut.Lesson(i));
                const auto weekDay = ToWeekDay(day);

                const auto dayLesson = sut.Lesson(i) % MAX_LESSONS_PER_DAY;
                REQUIRE((weekDay == WeekDay::Saturday || dayLesson == 5 || dayLesson == 6));
            }
        }
    }
}

TEST_CASE("Check if groups or professors or classrooms intersects", "[ScheduleChromosomes]")
{
    const std::vector<ClassroomAddress> classrooms = {{0, 1}, {0, 2}, {0, 3}};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, {}, {0, 1, 2}, classrooms),
        SubjectRequest(1, 2, 1, {}, {1, 2, 3}, classrooms),
        SubjectRequest(2, 1, 1, {}, {4, 5, 6}, classrooms),
        SubjectRequest(3, 4, 1, {}, {7, 8, 9}, classrooms),
        SubjectRequest(4, 5, 1, {}, {10},      classrooms)
    };
    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    SECTION("Check if groups intersects")
    {
        REQUIRE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));
    }
    SECTION("Check if professors intersects")
    {
        REQUIRE(sut.GroupsOrProfessorsIntersects(data, 2, 2));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 2, 2));
    }
    SECTION("Check if classrooms intersects")
    {
        REQUIRE(sut.ClassroomsIntersects(0, {0, 1}));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 3, 0));
    }
}

TEST_CASE("Check if chromosomes ready for crossover", "[ScheduleChromosomes]")
{
    const std::vector<ClassroomAddress> classrooms = {{0, 1}, {0, 2}, {0, 3}};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, {}, {0, 1, 2}, classrooms),
        SubjectRequest(1, 2, 1, {}, {1, 2, 3}, classrooms),
        SubjectRequest(2, 1, 1, {}, {4, 5, 6}, classrooms),
        SubjectRequest(3, 4, 1, {}, {7, 8, 9}, classrooms),
        SubjectRequest(4, 5, 1, {}, {10},      classrooms)
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

TEST_CASE("Crossover works", "[ScheduleChromosomes]")
{
    const std::vector<ClassroomAddress> classrooms = {{0, 1}, {0, 2}, {0, 3}};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, {}, {0, 1, 2}, classrooms),
        SubjectRequest(1, 2, 1, {}, {1, 2, 3}, classrooms),
        SubjectRequest(2, 1, 1, {}, {4, 5, 6}, classrooms),
        SubjectRequest(3, 4, 1, {}, {7, 8, 9}, classrooms),
        SubjectRequest(4, 5, 1, {}, {10},      classrooms)
    };
    const ScheduleData data{requests, {}};

    ScheduleChromosomes first{{0, 1, 2, 3, 4}, {{0,3}, {0,2}, {0,1}, {0,3}, {0,2}}};
    ScheduleChromosomes second{{4, 3, 2, 1, 0}, {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    Crossover(first, second, 0);
    REQUIRE(first.Lesson(0) == 4);
    REQUIRE(first.Classroom(0) == ClassroomAddress{0,1});

    REQUIRE(second.Lesson(0) == 0);
    REQUIRE(second.Classroom(0) == ClassroomAddress{0,3});
}

TEST_CASE("ToScheduleResult works correctly", "[GAScheduleGenerator]")
{
    const std::vector<ClassroomAddress> classrooms = {{0, 1}, {0, 2}, {0, 3}};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest{0, 1, 1, {}, {0, 1, 2}, classrooms},
        SubjectRequest{1, 2, 1, {}, {1, 2, 3}, classrooms},
        SubjectRequest{2, 1, 1, {}, {4, 5, 6}, classrooms},
        SubjectRequest{3, 4, 1, {}, {7, 8, 9}, classrooms},
        SubjectRequest{4, 5, 1, {}, {10},      classrooms}
    };
    const ScheduleData data{requests, {}};

    const ScheduleChromosomes chromosomes{
        {0, 4, 3, 2, 1},
        {{0,1}, {0,1}, {0,3}, {0, 2}, {0,3}}
    };

    const ScheduleResult scheduleResult = ToScheduleResult(chromosomes, data);
    REQUIRE(scheduleResult.items().at(0) == ScheduleItem{0, 0, 1});
    REQUIRE(scheduleResult.items().at(1) == ScheduleItem{1, 4, 3});
    REQUIRE(scheduleResult.items().at(2) == ScheduleItem{2, 3, 2});
    REQUIRE(scheduleResult.items().at(3) == ScheduleItem{3, 2, 3});
    REQUIRE(scheduleResult.items().at(4) == ScheduleItem{4, 1, 1});
}


TEST_CASE("ScheduleIndividual initialized correctly")
{
    const std::vector<ClassroomAddress> classrooms = {{0, 1}, {0, 2}, {0, 3}};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest{0, 1, 1, {}, {0, 1, 2}, classrooms},
        SubjectRequest{1, 2, 1, {}, {1, 2, 3}, classrooms},
        SubjectRequest{2, 1, 1, {}, {4, 5, 6}, classrooms},
        SubjectRequest{3, 4, 1, {}, {7, 8, 9}, classrooms},
        SubjectRequest{4, 5, 1, {}, {10},      classrooms}
    };
    const ScheduleData data{requests, {}};

    std::random_device rd;
    const ScheduleIndividual sut{rd, &data};
    const ScheduleResult result = ToScheduleResult(sut.Chromosomes(), data);
    const auto checkResult = CheckSchedule(data, result);

    REQUIRE(checkResult.OverlappedGroupsList.empty());
    REQUIRE(checkResult.OverlappedProfessorsList.empty());
    REQUIRE(checkResult.OverlappedClassroomsList.empty());
    REQUIRE(checkResult.ViolatedWeekdayRequestsList.empty());
}


